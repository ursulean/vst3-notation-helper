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
  double trebleStaffTop = centerY - GRAND_STAFF_GAP / 2.0 - STAFF_SPACING;
  double bassStaffTop = centerY + GRAND_STAFF_GAP / 2.0;

  // Draw treble staff lines (5 lines)
  for (int i = 0; i < 5; i++) {
    double y = trebleStaffTop + i * STAFF_LINE_HEIGHT;
    context->drawLine(VSTGUI::CPoint(LEFT_MARGIN, y),
                      VSTGUI::CPoint(rect.getWidth() - RIGHT_MARGIN, y));
  }

  // Draw bass staff lines (5 lines)
  for (int i = 0; i < 5; i++) {
    double y = bassStaffTop + i * STAFF_LINE_HEIGHT;
    context->drawLine(VSTGUI::CPoint(LEFT_MARGIN, y),
                      VSTGUI::CPoint(rect.getWidth() - RIGHT_MARGIN, y));
  }

  // Draw clefs
  drawTrebleClef(context, LEFT_MARGIN - 25, trebleStaffTop, 1.0);
  drawBassClef(context, LEFT_MARGIN - 25, bassStaffTop, 1.0);
}

//------------------------------------------------------------------------
void NotationView::drawTrebleClef(VSTGUI::CDrawContext *context, double x,
                                  double y, double scale) {
  // Simple text-based treble clef representation
  context->setLineWidth(2.0 * scale);
  context->setFrameColor(VSTGUI::CColor(0, 0, 0, 255));
  context->setFillColor(VSTGUI::CColor(0, 0, 0, 255));

  // Draw "𝄞" or simple "G" to represent treble clef
  context->setFont(VSTGUI::kSystemFont);
  context->setFontColor(VSTGUI::CColor(0, 0, 0, 255));

  VSTGUI::CRect textRect(x, y + 10 * scale, 25 * scale, 20 * scale);
  context->drawString("G", textRect, VSTGUI::kCenterText);
}

//------------------------------------------------------------------------
void NotationView::drawBassClef(VSTGUI::CDrawContext *context, double x,
                                double y, double scale) {
  // Simple text-based bass clef representation
  context->setLineWidth(2.0 * scale);
  context->setFrameColor(VSTGUI::CColor(0, 0, 0, 255));
  context->setFillColor(VSTGUI::CColor(0, 0, 0, 255));

  // Draw "𝄢" or simple "F" to represent bass clef
  context->setFont(VSTGUI::kSystemFont);
  context->setFontColor(VSTGUI::CColor(0, 0, 0, 255));

  VSTGUI::CRect textRect(x, y + 10 * scale, 25 * scale, 20 * scale);
  context->drawString("F", textRect, VSTGUI::kCenterText);
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
      drawLedgerLine(context, noteX - 6, noteY, 24);
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

  // Draw note head as an ellipse
  VSTGUI::CRect noteRect(x - NOTE_WIDTH / 2, y - NOTE_HEIGHT / 2, NOTE_WIDTH,
                         NOTE_HEIGHT);

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
    VSTGUI::CRect flatCurve(x + 2, y - 2, 6, 6);
    context->drawEllipse(flatCurve, VSTGUI::kDrawStroked);
  }
}

//------------------------------------------------------------------------
void NotationView::drawLedgerLine(VSTGUI::CDrawContext *context, double x,
                                  double y, double width) {
  context->setLineWidth(1.0);
  context->setFrameColor(VSTGUI::CColor(0, 0, 0, 255));
  context->drawLine(VSTGUI::CPoint(x - width / 2, y),
                    VSTGUI::CPoint(x + width / 2, y));
}

//------------------------------------------------------------------------
double NotationView::getStaffPosition(int midiNote, bool &isOnTrebleStaff,
                                      bool &needsAccidental, bool &isSharp) {
  // Middle C is MIDI note 60
  // Treble staff: Middle C is one ledger line below the staff
  // Bass staff: Middle C is one ledger line above the staff

  VSTGUI::CRect rect = getViewSize();
  double centerY = rect.getHeight() / 2.0;
  double trebleStaffTop = centerY - GRAND_STAFF_GAP / 2.0 - STAFF_SPACING;
  double bassStaffTop = centerY + GRAND_STAFF_GAP / 2.0;

  // Determine which staff to use (split around middle C)
  isOnTrebleStaff = (midiNote >= 60); // Middle C and above go to treble

  // Calculate position relative to staff
  int noteClass = midiNote % 12;
  int octave = midiNote / 12;

  // White key positions (C=0, D=1, E=2, F=3, G=4, A=5, B=6)
  static const int whiteKeyMap[12] = {
      0, 0, 1, 1, 2, 3,
      3, 4, 4, 5, 5, 6}; // C, C#, D, D#, E, F, F#, G, G#, A, A#, B
  static const bool isBlackKey[12] = {false, true,  false, true,  false, false,
                                      true,  false, true,  false, true,  false};

  int whiteKeyPosition = whiteKeyMap[noteClass];
  needsAccidental = isBlackKey[noteClass];
  isSharp = needsAccidental; // For simplicity, always use sharps

  if (isOnTrebleStaff) {
    // Treble staff: Middle C (C4, MIDI 60) is one ledger line below (position
    // 5) Staff lines are at positions: 0, 1, 2, 3, 4 (bottom to top) Middle C
    // (C4) is at position 5 (below staff) C5 would be at position -2 (in staff)

    double staffPosition;
    if (midiNote == 60) { // Middle C
      staffPosition =
          trebleStaffTop + 5 * STAFF_LINE_HEIGHT; // One line below staff
    } else {
      // Calculate relative to Middle C
      int semitonesFromMiddleC = midiNote - 60;
      int whiteKeysFromMiddleC = 0;

      // Count white keys from middle C
      for (int i = 60; i != midiNote; i += (midiNote > 60 ? 1 : -1)) {
        if (!isBlackKey[i % 12]) {
          whiteKeysFromMiddleC += (midiNote > 60 ? 1 : -1);
        }
      }

      staffPosition = trebleStaffTop +
                      (5 - whiteKeysFromMiddleC) * (STAFF_LINE_HEIGHT / 2.0);
    }

    return staffPosition;
  } else {
    // Bass staff: Middle C is one ledger line above the staff
    double staffPosition;
    if (midiNote == 60) {                               // Middle C
      staffPosition = bassStaffTop - STAFF_LINE_HEIGHT; // One line above staff
    } else {
      // Calculate relative to Middle C
      int whiteKeysFromMiddleC = 0;

      // Count white keys from middle C
      for (int i = 60; i != midiNote; i += (midiNote > 60 ? 1 : -1)) {
        if (!isBlackKey[i % 12]) {
          whiteKeysFromMiddleC += (midiNote > 60 ? 1 : -1);
        }
      }

      staffPosition = bassStaffTop - STAFF_LINE_HEIGHT -
                      whiteKeysFromMiddleC * (STAFF_LINE_HEIGHT / 2.0);
    }

    return staffPosition;
  }
}

//------------------------------------------------------------------------
bool NotationView::needsLedgerLine(double staffPosition, bool isOnTrebleStaff) {
  VSTGUI::CRect rect = getViewSize();
  double centerY = rect.getHeight() / 2.0;
  double trebleStaffTop = centerY - GRAND_STAFF_GAP / 2.0 - STAFF_SPACING;
  double bassStaffTop = centerY + GRAND_STAFF_GAP / 2.0;

  if (isOnTrebleStaff) {
    double staffBottom = trebleStaffTop + 4 * STAFF_LINE_HEIGHT;
    return (staffPosition < trebleStaffTop || staffPosition > staffBottom);
  } else {
    double staffTop = bassStaffTop;
    double staffBottom = bassStaffTop + 4 * STAFF_LINE_HEIGHT;
    return (staffPosition < staffTop || staffPosition > staffBottom);
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