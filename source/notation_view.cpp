//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#include "notation_view.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cpoint.h"
#include "vstgui/lib/crect.h"
#include <algorithm>
#include <cmath>

namespace Ursulean {

//------------------------------------------------------------------------
NotationView::NotationView(const VSTGUI::CRect &size) : CView(size) {
  initializeNoteMappings();
}

//------------------------------------------------------------------------
void NotationView::setActiveNotes(const std::vector<int> &notes) {
  activeNotes = notes;
  invalid(); // Trigger redraw
}

//------------------------------------------------------------------------
void NotationView::draw(VSTGUI::CDrawContext *context) {
  CView::draw(context);

  VSTGUI::CRect rect = getViewSize();

  // Clear background
  context->setFillColor(
      VSTGUI::CColor(250, 250, 250, 255)); // Light gray background
  context->drawRect(rect, VSTGUI::kDrawFilled);

  // Draw the staff
  drawStaff(context, rect);

  // Draw the notes
  drawNotes(context, rect);
}

//------------------------------------------------------------------------
void NotationView::drawStaff(VSTGUI::CDrawContext *context,
                             const VSTGUI::CRect &rect) {
  context->setLineWidth(1.0);
  context->setDrawMode(VSTGUI::kAntiAliasing);
  context->setLineStyle(VSTGUI::kLineSolid);
  context->setFrameColor(VSTGUI::CColor(0, 0, 0, 255)); // Black lines

  double centerY = rect.getHeight() / 2.0;
  double middleCPosition = centerY;

  // Treble staff lines (bottom to top): E4, G4, B4, D5, F5
  // Positioned at: -8, -16, -24, -32, -40 pixels from middle C
  for (int i = 0; i < 5; i++) {
    double y =
        middleCPosition - (8 + i * 8); // E4 line, then up by 8 pixels each
    context->drawLine(VSTGUI::CPoint(LEFT_MARGIN, y),
                      VSTGUI::CPoint(rect.getWidth() - RIGHT_MARGIN, y));
  }

  // Bass staff lines (top to bottom): A3, F3, D3, B2, G2
  // Positioned at: +8, +16, +24, +32, +40 pixels from middle C
  for (int i = 0; i < 5; i++) {
    double y =
        middleCPosition + (8 + i * 8); // A3 line, then down by 8 pixels each
    context->drawLine(VSTGUI::CPoint(LEFT_MARGIN, y),
                      VSTGUI::CPoint(rect.getWidth() - RIGHT_MARGIN, y));
  }

  // Draw clefs - position them relative to their respective staves
  double trebleClefY = middleCPosition - 24; // Around B4 line
  double bassClefY = middleCPosition + 16;   // Around F3 line
  drawTrebleClef(context, LEFT_MARGIN - 25, trebleClefY, 1.0);
  drawBassClef(context, LEFT_MARGIN - 25, bassClefY, 1.0);
}

//------------------------------------------------------------------------
void NotationView::drawTrebleClef(VSTGUI::CDrawContext *context, double x,
                                  double y, double scale) {
  // Very simple treble clef - just draw "♪" or "G"
  context->setFont(VSTGUI::kSystemFont);
  context->setFontColor(VSTGUI::CColor(0, 0, 0, 255));

  // CRect takes (left, top, right, bottom)
  VSTGUI::CRect textRect(x, y + 15, x + 20, y + 30);
  context->drawString("G", textRect);
}

//------------------------------------------------------------------------
void NotationView::drawBassClef(VSTGUI::CDrawContext *context, double x,
                                double y, double scale) {
  // Very simple bass clef - just draw "F"
  context->setFont(VSTGUI::kSystemFont);
  context->setFontColor(VSTGUI::CColor(0, 0, 0, 255));

  // CRect takes (left, top, right, bottom)
  VSTGUI::CRect textRect(x, y + 15, x + 20, y + 30);
  context->drawString("F", textRect);
}

//------------------------------------------------------------------------
void NotationView::drawNotes(VSTGUI::CDrawContext *context,
                             const VSTGUI::CRect &rect) {
  if (activeNotes.empty())
    return;

  // Sort notes for consistent positioning
  std::vector<int> sortedNotes = activeNotes;
  std::sort(sortedNotes.begin(), sortedNotes.end());

  // Calculate staff positions for all notes
  std::vector<double> staffPositions;
  std::vector<bool> isOnTrebleStaff;
  std::vector<bool> needsAccidental;
  std::vector<bool> isSharp;

  for (int note : sortedNotes) {
    bool treble, accidental, sharp;
    double position = getStaffPosition(note, treble, accidental, sharp);
    staffPositions.push_back(position);
    isOnTrebleStaff.push_back(treble);
    needsAccidental.push_back(accidental);
    isSharp.push_back(sharp);
  }

  // Draw each note
  double baseX = LEFT_MARGIN + CLEF_WIDTH + 20;

  for (size_t i = 0; i < sortedNotes.size(); i++) {
    double xOffset = getHorizontalOffset(i, staffPositions);
    double noteX = baseX + xOffset;
    double noteY = staffPositions[i];

    // Draw ledger lines if needed
    if (needsLedgerLine(staffPositions[i], isOnTrebleStaff[i])) {
      drawLedgerLine(context, noteX, noteY,
                     20); // Center on note, slightly shorter
    }

    // Draw accidental if needed
    if (needsAccidental[i]) {
      drawAccidental(context, noteX - 20, noteY, isSharp[i]);
    }

    // Draw the note
    drawNote(context, noteX, noteY, true);
  }
}

//------------------------------------------------------------------------
void NotationView::drawNote(VSTGUI::CDrawContext *context, double x, double y,
                            bool filled) {
  context->setLineWidth(1.5);
  context->setFrameColor(VSTGUI::CColor(0, 0, 0, 255));

  // Draw note head as an ellipse - CRect takes (left, top, right, bottom)
  double left = x - NOTE_WIDTH / 2;
  double top = y - NOTE_HEIGHT / 2;
  double right = x + NOTE_WIDTH / 2;
  double bottom = y + NOTE_HEIGHT / 2;

  VSTGUI::CRect noteRect(left, top, right, bottom);

  if (filled) {
    context->setFillColor(VSTGUI::CColor(0, 0, 0, 255));
    context->drawEllipse(noteRect, VSTGUI::kDrawFilled);
  } else {
    context->drawEllipse(noteRect, VSTGUI::kDrawStroked);
  }
}

//------------------------------------------------------------------------
void NotationView::drawAccidental(VSTGUI::CDrawContext *context, double x,
                                  double y, bool isSharp) {
  context->setLineWidth(1.5);
  context->setFrameColor(VSTGUI::CColor(0, 0, 0, 255));

  if (isSharp) {
    // Draw sharp symbol (#)
    // Vertical lines
    context->drawLine(VSTGUI::CPoint(x + 2, y - 6),
                      VSTGUI::CPoint(x + 2, y + 6));
    context->drawLine(VSTGUI::CPoint(x + 6, y - 6),
                      VSTGUI::CPoint(x + 6, y + 6));
    // Horizontal lines
    context->drawLine(VSTGUI::CPoint(x, y - 2), VSTGUI::CPoint(x + 8, y - 4));
    context->drawLine(VSTGUI::CPoint(x, y + 2), VSTGUI::CPoint(x + 8, y));
  } else {
    // Draw flat symbol (b)
    context->drawLine(VSTGUI::CPoint(x + 2, y - 8),
                      VSTGUI::CPoint(x + 2, y + 4));
    // CRect takes (left, top, right, bottom)
    VSTGUI::CRect flatCurve(x + 2, y - 2, x + 8, y + 4);
    context->drawEllipse(flatCurve, VSTGUI::kDrawStroked);
  }
}

//------------------------------------------------------------------------
void NotationView::drawLedgerLine(VSTGUI::CDrawContext *context, double x,
                                  double y, double width) {
  context->setLineWidth(1.0);
  context->setFrameColor(VSTGUI::CColor(0, 0, 0, 255));
  // Draw ledger line centered on the note position
  context->drawLine(VSTGUI::CPoint(x - width / 2, y),
                    VSTGUI::CPoint(x + width / 2, y));
}

//------------------------------------------------------------------------
double NotationView::getStaffPosition(int midiNote, bool &isOnTrebleStaff,
                                      bool &needsAccidental, bool &isSharp) {
  // Unified grand staff positioning - all notes relative to middle C
  VSTGUI::CRect rect = getViewSize();
  double centerY = rect.getHeight() / 2.0;

  // Middle C position (the invisible line between treble and bass staves)
  double middleCPosition = centerY;

  // Determine if note needs accidental (black key)
  int noteClass = midiNote % 12;
  static const bool isBlackKey[12] = {false, true,  false, true,  false, false,
                                      true,  false, true,  false, true,  false};
  needsAccidental = isBlackKey[noteClass];
  isSharp = needsAccidental; // For simplicity, always use sharps

  // For ledger line logic, we still need to know which staff area we're in
  isOnTrebleStaff = (midiNote >= 60); // Middle C and above go to treble

  // Calculate staff position relative to middle C
  // Each white key step = half staff line height (4 pixels)
  int semitonesFromMiddleC = midiNote - 60; // Middle C = MIDI 60
  int whiteKeyStepsFromMiddleC = 0;

  // Count white key steps from middle C
  static const bool isWhiteKey[12] = {true,  false, true,  false, true,  true,
                                      false, true,  false, true,  false, true};

  if (semitonesFromMiddleC > 0) {
    // Going up from middle C
    for (int i = 1; i <= semitonesFromMiddleC; i++) {
      int noteIndex = (60 + i) % 12;
      if (isWhiteKey[noteIndex]) {
        whiteKeyStepsFromMiddleC++;
      }
    }
  } else if (semitonesFromMiddleC < 0) {
    // Going down from middle C
    for (int i = -1; i >= semitonesFromMiddleC; i--) {
      int noteIndex = (60 + i + 120) % 12; // +120 to handle negative modulo
      if (isWhiteKey[noteIndex]) {
        whiteKeyStepsFromMiddleC--;
      }
    }
  }
  // If semitonesFromMiddleC == 0, it's middle C, so whiteKeyStepsFromMiddleC
  // stays 0

  // Calculate final position: middle C + (white key steps * half staff line
  // height) Negative steps go up (treble), positive steps go down (bass)
  double staffPosition =
      middleCPosition - (whiteKeyStepsFromMiddleC * (STAFF_LINE_HEIGHT / 2.0));

  return staffPosition;
}

//------------------------------------------------------------------------
bool NotationView::needsLedgerLine(double staffPosition, bool isOnTrebleStaff) {
  VSTGUI::CRect rect = getViewSize();
  double centerY = rect.getHeight() / 2.0;
  double middleCPosition = centerY;

  // In unified grand staff system:
  // Treble staff lines: E4 to F5 (positions centerY-32 to centerY-8)
  // Bass staff lines: A3 to G2 (positions centerY+8 to centerY+40)
  // Middle C is at centerY (between the staves)

  double trebleStaffTop =
      middleCPosition - (STAFF_SPACING + GRAND_STAFF_GAP / 2);        // F5 line
  double trebleStaffBottom = middleCPosition - (GRAND_STAFF_GAP / 2); // E4 line
  double bassStaffTop = middleCPosition + (GRAND_STAFF_GAP / 2);      // A3 line
  double bassStaffBottom =
      middleCPosition + (STAFF_SPACING + GRAND_STAFF_GAP / 2); // G2 line

  if (isOnTrebleStaff) {
    // Note is in treble clef area - needs ledger line if outside treble staff
    return (staffPosition < trebleStaffTop ||
            staffPosition > trebleStaffBottom);
  } else {
    // Note is in bass clef area - needs ledger line if outside bass staff
    return (staffPosition < bassStaffTop || staffPosition > bassStaffBottom);
  }
}

//------------------------------------------------------------------------
double
NotationView::getHorizontalOffset(int noteIndex,
                                  const std::vector<double> &staffPositions) {
  // Simple horizontal spacing - just spread notes out evenly
  // More sophisticated logic could handle overlapping notes
  return noteIndex * 30.0; // 30 pixels apart
}

//------------------------------------------------------------------------
void NotationView::initializeNoteMappings() {
  // This method could be used to pre-calculate note mappings
  // For now, we calculate positions dynamically in getStaffPosition
}

//------------------------------------------------------------------------
} // namespace Ursulean