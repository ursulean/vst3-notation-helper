//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#include "notation_view.h"
#include "controller.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cpoint.h"
#include "vstgui/lib/crect.h"
#include <algorithm>
#include <cmath>

namespace Ursulean {

// Key signature lookup tables
// Index: KeySignature enum value
// Value: array of 7 bools for note classes C, D, E, F, G, A, B (0-6 mod 7)
const bool NotationView::keySignatureAccidentals[15][7] = {
    // C Major: no accidentals
    {false, false, false, false, false, false, false},
    // G Major: F#
    {false, false, false, true, false, false, false},
    // D Major: F#, C#
    {true, false, false, true, false, false, false},
    // A Major: F#, C#, G#
    {true, false, false, true, true, false, false},
    // E Major: F#, C#, G#, D#
    {true, true, false, true, true, false, false},
    // B Major: F#, C#, G#, D#, A#
    {true, true, false, true, true, true, false},
    // F# Major: F#, C#, G#, D#, A#, E#
    {true, true, true, true, true, true, false},
    // C# Major: F#, C#, G#, D#, A#, E#, B#
    {true, true, true, true, true, true, true},
    // F Major: Bb
    {false, false, false, false, false, false, true},
    // Bb Major: Bb, Eb
    {false, false, true, false, false, false, true},
    // Eb Major: Bb, Eb, Ab
    {false, false, true, false, false, true, true},
    // Ab Major: Bb, Eb, Ab, Db
    {false, true, true, false, false, true, true},
    // Db Major: Bb, Eb, Ab, Db, Gb
    {false, true, true, false, true, true, true},
    // Gb Major: Bb, Eb, Ab, Db, Gb, Cb
    {true, true, true, false, true, true, true},
    // Cb Major: Bb, Eb, Ab, Db, Gb, Cb, Fb
    {true, true, true, true, true, true, true}};

const bool NotationView::keySignatureIsSharp[15][7] = {
    // C Major: no accidentals
    {false, false, false, false, false, false, false},
    // Sharp keys: all accidentals are sharp
    {false, false, false, true, false, false, false}, // G Major
    {true, false, false, true, false, false, false},  // D Major
    {true, false, false, true, true, false, false},   // A Major
    {true, true, false, true, true, false, false},    // E Major
    {true, true, false, true, true, true, false},     // B Major
    {true, true, true, true, true, true, false},      // F# Major
    {true, true, true, true, true, true, true},       // C# Major
    // Flat keys: all accidentals are flat
    {false, false, false, false, false, false, false}, // F Major
    {false, false, false, false, false, false, false}, // Bb Major
    {false, false, false, false, false, false, false}, // Eb Major
    {false, false, false, false, false, false, false}, // Ab Major
    {false, false, false, false, false, false, false}, // Db Major
    {false, false, false, false, false, false, false}, // Gb Major
    {false, false, false, false, false, false, false}  // Cb Major
};

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
void NotationView::setKeySignature(KeySignature keySignature) {
  currentKeySignature = keySignature;
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

  // Draw the key signature
  drawKeySignature(context, rect);

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

  // Draw clefs - position them properly centered on their respective staves
  // Treble clef should be centered on G4 line (2nd line from bottom) = -16 from
  // middle C
  double trebleClefY = middleCPosition - 16; // G4 line center
  // Bass clef should be centered on F3 line (4th line from top) = +24 from
  // middle C
  double bassClefY = middleCPosition + 24; // F3 line center
  drawTrebleClef(context, LEFT_MARGIN - 30, trebleClefY - 20, 1.0);
  drawBassClef(context, LEFT_MARGIN - 30, bassClefY - 12, 1.0);
}

//------------------------------------------------------------------------
void NotationView::drawTrebleClef(VSTGUI::CDrawContext *context, double x,
                                  double y, double scale) {
  // Draw treble clef using Unicode musical symbol with larger font
  auto font = VSTGUI::makeOwned<VSTGUI::CFontDesc>("Arial", 24);
  context->setFont(font);
  context->setFontColor(VSTGUI::CColor(0, 0, 0, 255));

  // Draw Unicode treble clef symbol
  VSTGUI::CRect textRect(x, y, x + 30, y + 40);
  context->drawString("𝄞", textRect); // Unicode treble clef
}

//------------------------------------------------------------------------
void NotationView::drawBassClef(VSTGUI::CDrawContext *context, double x,
                                double y, double scale) {
  // Draw bass clef using Unicode musical symbol with larger font
  auto font = VSTGUI::makeOwned<VSTGUI::CFontDesc>("Arial", 24);
  context->setFont(font);
  context->setFontColor(VSTGUI::CColor(0, 0, 0, 255));

  // Draw Unicode bass clef symbol
  VSTGUI::CRect textRect(x, y, x + 30, y + 40);
  context->drawString("𝄢", textRect); // Unicode bass clef
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
  std::vector<bool> isNatural;

  for (int note : sortedNotes) {
    bool treble, accidental, sharp, natural;
    double position =
        getStaffPosition(note, treble, accidental, sharp, natural);
    staffPositions.push_back(position);
    isOnTrebleStaff.push_back(treble);
    needsAccidental.push_back(accidental);
    isSharp.push_back(sharp);
    isNatural.push_back(natural);
  }

  // Group notes by their positioning requirements
  std::vector<std::vector<int>> noteGroups =
      groupNotesByPosition(sortedNotes, staffPositions, needsAccidental);

  // Draw each group of notes - position after key signature
  int numAccidentalsInKey = 0;
  for (int i = 0; i < 7; i++) {
    if (keySignatureAccidentals[static_cast<int>(currentKeySignature)][i]) {
      numAccidentalsInKey++;
    }
  }
  double keySigWidth =
      numAccidentalsInKey * 12 + (numAccidentalsInKey > 0 ? 20 : 0);
  double baseX = LEFT_MARGIN + CLEF_WIDTH + keySigWidth + 20;
  double groupOffsetX = 0;

  for (const auto &group : noteGroups) {
    if (group.size() == 1) {
      // Single note - draw at center position
      int noteIndex = group[0];
      double noteX = baseX + groupOffsetX;
      double noteY = staffPositions[noteIndex];

      // Draw ledger lines if needed
      if (needsLedgerLine(sortedNotes[noteIndex], isOnTrebleStaff[noteIndex])) {
        drawLedgerLinesForNote(context, noteX, noteY, sortedNotes[noteIndex]);
      }

      // Draw accidental if needed
      if (needsAccidental[noteIndex]) {
        if (isNatural[noteIndex]) {
          drawNatural(context, noteX - 25, noteY);
        } else {
          drawAccidental(context, noteX - 25, noteY, isSharp[noteIndex]);
        }
      }

      // Draw the note
      drawNote(context, noteX, noteY, true);
    } else {
      // Multiple notes - position in maximum 2 columns (like real musical
      // notation)
      double groupCenterX = baseX + groupOffsetX;

      if (needsSideBySidePositioning(group, staffPositions, needsAccidental)) {
        // Side-by-side positioning: maximum 2 columns like real notation
        // Left column (where stem would be on left) and right column (where
        // stem would be on right)

        for (size_t i = 0; i < group.size(); i++) {
          int noteIndex = group[i];
          double noteY = staffPositions[noteIndex];

          // Determine which column: alternate between left and right
          // Left column = -NOTE_WIDTH*0.4, Right column = +NOTE_WIDTH*0.4
          double noteX;
          if (i % 2 == 0) {
            noteX = groupCenterX - NOTE_WIDTH * 0.4; // Left column
          } else {
            noteX = groupCenterX + NOTE_WIDTH * 0.4; // Right column
          }

          // Draw ledger lines if needed
          if (needsLedgerLine(sortedNotes[noteIndex],
                              isOnTrebleStaff[noteIndex])) {
            drawLedgerLinesForNote(context, noteX, noteY,
                                   sortedNotes[noteIndex]);
          }

          // Draw accidental if needed with better positioning to avoid
          // collisions
          if (needsAccidental[noteIndex]) {
            if (isNatural[noteIndex]) {
              drawNatural(context, noteX - 25, noteY);
            } else {
              drawAccidental(context, noteX - 25, noteY, isSharp[noteIndex]);
            }
          }

          // Draw the note
          drawNote(context, noteX, noteY, true);
        }
      } else {
        // Stacked positioning: all notes at same X position
        for (size_t i = 0; i < group.size(); i++) {
          int noteIndex = group[i];
          double noteY = staffPositions[noteIndex];
          double noteX = groupCenterX;

          // Draw ledger lines if needed
          if (needsLedgerLine(sortedNotes[noteIndex],
                              isOnTrebleStaff[noteIndex])) {
            drawLedgerLinesForNote(context, noteX, noteY,
                                   sortedNotes[noteIndex]);
          }

          // Draw accidental if needed
          if (needsAccidental[noteIndex]) {
            if (isNatural[noteIndex]) {
              drawNatural(context, noteX - 25, noteY);
            } else {
              drawAccidental(context, noteX - 25, noteY, isSharp[noteIndex]);
            }
          }

          // Draw the note
          drawNote(context, noteX, noteY, true);
        }
      }
    }

    // Move to next group position
    groupOffsetX += 40; // Space between chord groups
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
void NotationView::drawNatural(VSTGUI::CDrawContext *context, double x,
                               double y) {
  context->setLineWidth(1.5);
  context->setFrameColor(VSTGUI::CColor(0, 0, 0, 255));

  // Draw natural symbol (♮)
  // Two vertical lines
  context->drawLine(VSTGUI::CPoint(x + 1, y - 8), VSTGUI::CPoint(x + 1, y + 4));
  context->drawLine(VSTGUI::CPoint(x + 5, y - 4), VSTGUI::CPoint(x + 5, y + 8));

  // Two horizontal connecting lines (slightly slanted)
  context->drawLine(VSTGUI::CPoint(x + 1, y - 2), VSTGUI::CPoint(x + 5, y - 4));
  context->drawLine(VSTGUI::CPoint(x + 1, y + 2), VSTGUI::CPoint(x + 5, y));
}

//------------------------------------------------------------------------
void NotationView::drawLedgerLine(VSTGUI::CDrawContext *context, double x,
                                  double y, double width) {
  context->setLineWidth(1.0);
  context->setFrameColor(VSTGUI::CColor(0, 0, 0, 255));
  // Draw ledger line centered on the specified position
  context->drawLine(VSTGUI::CPoint(x - width / 2, y),
                    VSTGUI::CPoint(x + width / 2, y));
}

//------------------------------------------------------------------------
void NotationView::drawLedgerLinesForNote(VSTGUI::CDrawContext *context,
                                          double x, double noteY,
                                          int midiNote) {
  VSTGUI::CRect rect = getViewSize();
  double centerY = rect.getHeight() / 2.0;

  // Calculate which ledger lines are needed based on note position
  // Staff lines are at centerY ± 8, ± 16, ± 24, ± 32, ± 40
  // Ledger lines are at centerY ± 48, ± 56, ± 64, etc. and centerY (middle C)

  std::vector<double> ledgerLinePositions;

  // Middle C area (between staves)
  if (midiNote >= 59 && midiNote <= 63) {
    ledgerLinePositions.push_back(centerY); // Middle C ledger line
  }

  // Above treble staff (notes above G5 = MIDI 79)
  if (midiNote > 79) {
    // Calculate how many ledger lines we need above the treble staff
    // Treble staff top line is at centerY - 40
    // First ledger line above is at centerY - 48
    double staffTopLine = centerY - 40;
    double notePosition = noteY;

    // Add ledger lines from first ledger line (centerY - 48) down to just above
    // the staff
    for (double ledgerY = centerY - 48; ledgerY >= notePosition - 4;
         ledgerY -= 8) {
      ledgerLinePositions.push_back(ledgerY);
    }
  }

  // Below bass staff (notes below F2 = MIDI 41)
  if (midiNote < 41) {
    // Calculate how many ledger lines we need below the bass staff
    // Bass staff bottom line is at centerY + 40
    // First ledger line below is at centerY + 48
    double staffBottomLine = centerY + 40;
    double notePosition = noteY;

    // Add ledger lines from first ledger line (centerY + 48) up to just below
    // the staff
    for (double ledgerY = centerY + 48; ledgerY <= notePosition + 4;
         ledgerY += 8) {
      ledgerLinePositions.push_back(ledgerY);
    }
  }

  // Draw all calculated ledger lines
  for (double ledgerY : ledgerLinePositions) {
    drawLedgerLine(context, x, ledgerY, 20);
  }
}

//------------------------------------------------------------------------
void NotationView::drawKeySignature(VSTGUI::CDrawContext *context,
                                    const VSTGUI::CRect &rect) {
  // MIDI notes for key signature accidentals in treble clef
  // Sharp order: F, C, G, D, A, E, B
  static const int trebleSharpNotes[7] = {
      77, // F5
      72, // C5
      79, // G5
      74, // D5
      69, // A4
      76, // E5
      71  // B4
  };

  // MIDI notes for key signature accidentals in treble clef
  // Flat order: B, E, A, D, G, C, F
  static const int trebleFlatNotes[7] = {
      71, // B4
      76, // E5
      69, // A4
      74, // D5
      67, // G4
      72, // C5
      65  // F4
  };

  // MIDI notes for key signature accidentals in bass clef
  // Sharp order: F, C, G, D, A, E, B
  static const int bassSharpNotes[7] = {
      53, // F3
      48, // C3
      55, // G3
      50, // D3
      45, // A2
      52, // E3
      47  // B2
  };

  // MIDI notes for key signature accidentals in bass clef
  // Flat order: B, E, A, D, G, C, F
  static const int bassFlatNotes[7] = {
      47, // B2
      52, // E3
      45, // A2
      50, // D3
      43, // G2
      48, // C3
      41  // F2
  };

  double baseX = LEFT_MARGIN + 20; // Start after clefs

  // Count accidentals to draw
  int numAccidentals = 0;
  for (int i = 0; i < 7; i++) {
    if (keySignatureAccidentals[static_cast<int>(currentKeySignature)][i]) {
      numAccidentals++;
    }
  }

  if (numAccidentals == 0)
    return; // C Major - no accidentals

  // Determine if we're using sharps or flats
  bool usingSharps = (static_cast<int>(currentKeySignature) >= 1 &&
                      static_cast<int>(currentKeySignature) <= 7);

  // Draw accidentals in the correct order
  if (usingSharps) {
    // Sharp order: F, C, G, D, A, E, B
    static const int sharpOrder[7] = {
        3, 0, 4, 1, 5, 2, 6}; // F, C, G, D, A, E, B (note class indices)
    int accidentalIndex = 0;
    for (int i = 0; i < 7 && accidentalIndex < numAccidentals; i++) {
      int noteClass = sharpOrder[i];
      if (keySignatureAccidentals[static_cast<int>(currentKeySignature)]
                                 [noteClass]) {
        double x = baseX + accidentalIndex * 12;

        // Use getStaffPosition to get correct Y positions
        bool trebleStaff, needsAcc, isSharp, isNatural;
        double trebleY = getStaffPosition(trebleSharpNotes[i], trebleStaff,
                                          needsAcc, isSharp, isNatural);
        double bassY = getStaffPosition(bassSharpNotes[i], trebleStaff,
                                        needsAcc, isSharp, isNatural);

        // Draw sharp on both staves
        drawAccidental(context, x, trebleY, true);
        drawAccidental(context, x, bassY, true);
        accidentalIndex++;
      }
    }
  } else {
    // Flat order: B, E, A, D, G, C, F
    static const int flatOrder[7] = {
        6, 2, 5, 1, 4, 0, 3}; // B, E, A, D, G, C, F (note class indices)
    int accidentalIndex = 0;
    for (int i = 0; i < 7 && accidentalIndex < numAccidentals; i++) {
      int noteClass = flatOrder[i];
      if (keySignatureAccidentals[static_cast<int>(currentKeySignature)]
                                 [noteClass]) {
        double x = baseX + accidentalIndex * 12;

        // Use getStaffPosition to get correct Y positions
        bool trebleStaff, needsAcc, isSharp, isNatural;
        double trebleY = getStaffPosition(trebleFlatNotes[i], trebleStaff,
                                          needsAcc, isSharp, isNatural);
        double bassY = getStaffPosition(bassFlatNotes[i], trebleStaff, needsAcc,
                                        isSharp, isNatural);

        // Draw flat on both staves
        drawAccidental(context, x, trebleY, false);
        drawAccidental(context, x, bassY, false);
        accidentalIndex++;
      }
    }
  }
}

//------------------------------------------------------------------------
double NotationView::getStaffPosition(int midiNote, bool &isOnTrebleStaff,
                                      bool &needsAccidental, bool &isSharp,
                                      bool &isNatural) {
  // Unified grand staff positioning - all notes relative to middle C
  VSTGUI::CRect rect = getViewSize();
  double centerY = rect.getHeight() / 2.0;

  // Middle C position (the invisible line between treble and bass staves)
  double middleCPosition = centerY;

  // Determine if note needs accidental based on key signature
  int noteClass = midiNote % 12;
  static const bool isBlackKey[12] = {false, true,  false, true,  false, false,
                                      true,  false, true,  false, true,  false};

  // Convert MIDI note class to white key class (C=0, D=1, E=2, F=3, G=4, A=5,
  // B=6)
  static const int whiteKeyClass[12] = {
      0, 0, 1, 1, 2, 3,
      3, 4, 4, 5, 5, 6}; // C, C#, D, D#, E, F, F#, G, G#, A, A#, B
  int whiteNote = whiteKeyClass[noteClass];

  // Initialize flags
  needsAccidental = false;
  isSharp = false;
  isNatural = false;

  if (isBlackKey[noteClass]) {
    // This is a black key - check if it's in the key signature
    if (isNoteInKeySignature(whiteNote)) {
      // This accidental is in the key signature, so don't draw it
      needsAccidental = false;
    } else {
      // This accidental is not in the key signature, so draw it
      needsAccidental = true;
      isSharp = true; // Default to sharp when not in key signature
    }
  } else {
    // This is a white key - check if it needs a natural due to key signature
    if (isNoteInKeySignature(whiteNote)) {
      // This white key is altered by the key signature, so we need a natural
      // sign
      needsAccidental = true;
      isNatural = true;
    } else {
      // This white key is natural and not affected by key signature
      needsAccidental = false;
    }
  }

  // For ledger line logic, we still need to know which staff area we're in
  isOnTrebleStaff = (midiNote >= 60); // Middle C and above go to treble

  // Calculate staff position relative to middle C
  // Each white key step = half staff line height (4 pixels)

  // Calculate white key steps directly from note's octave and white key class
  // MIDI note 60 = C4 (middle C), so octave = (midiNote / 12) - 1
  // But we need to adjust for the fact that C4 = middle C
  int octave = (midiNote / 12) - 1; // This gives us the musical octave
  int middleCOctave = 4;            // Middle C is in octave 4
  int middleCWhiteKeyClass = 0;     // C is white key class 0

  // Calculate white key steps from middle C
  // Each octave has 7 white keys, so octave difference * 7 + white key class
  // difference
  int whiteKeyStepsFromMiddleC =
      (octave - middleCOctave) * 7 + (whiteNote - middleCWhiteKeyClass);

  // Calculate final position: middle C + (white key steps * half staff line
  // height) Negative steps go up (treble), positive steps go down (bass)
  double staffPosition =
      middleCPosition - (whiteKeyStepsFromMiddleC * (STAFF_LINE_HEIGHT / 2.0));

  return staffPosition;
}

//------------------------------------------------------------------------
bool NotationView::needsLedgerLine(int midiNote, bool isOnTrebleStaff) {
  // Ledger lines are needed for:
  // - anything above G5 (MIDI 79)
  // - C4 (MIDI 60) - middle C between staves
  // - anything below F2 (MIDI 41)

  if (midiNote > 79) {
    return true; // Above G5
  }
  if (midiNote < 41) {
    return true; // Below F2
  }
  if (midiNote >= 59 && midiNote <= 63) {
    return true; // Middle C4
  }

  return false; // Note is on a staff line or space, no ledger line needed
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
std::vector<std::vector<int>>
NotationView::groupNotesByPosition(const std::vector<int> &sortedNotes,
                                   const std::vector<double> &staffPositions,
                                   const std::vector<bool> &needsAccidental) {
  std::vector<std::vector<int>> groups;

  if (sortedNotes.empty())
    return groups;

  // For now, treat all simultaneously played notes as one group
  // This creates a chord where notes are stacked or positioned appropriately
  std::vector<int> allNotes;
  for (size_t i = 0; i < sortedNotes.size(); i++) {
    allNotes.push_back(i); // Store indices into the sorted arrays
  }

  groups.push_back(allNotes);
  return groups;
}

//------------------------------------------------------------------------
bool NotationView::needsSideBySidePositioning(
    const std::vector<int> &group, const std::vector<double> &staffPositions,
    const std::vector<bool> &needsAccidental) {
  if (group.size() <= 1)
    return false;

  // Check if any notes are on the same staff position (same Y coordinate)
  // or if notes are on adjacent staff positions
  for (size_t i = 0; i < group.size(); i++) {
    for (size_t j = i + 1; j < group.size(); j++) {
      int idx1 = group[i];
      int idx2 = group[j];

      double pos1 = staffPositions[idx1];
      double pos2 = staffPositions[idx2];
      double posDiff = std::abs(pos1 - pos2);

      // Same position (like C and C#) - need side-by-side
      if (posDiff < 2.0) {
        return true;
      }

      // Adjacent staff positions (line and space) - need side-by-side
      if (posDiff >= 3.5 &&
          posDiff <= 4.5) { // Half staff line height = 4 pixels
        return true;
      }
    }
  }

  // Otherwise, stack the notes
  return false;
}

//------------------------------------------------------------------------
bool NotationView::isNoteInKeySignature(int noteClass) const {
  if (noteClass < 0 || noteClass >= 7)
    return false;
  return keySignatureAccidentals[static_cast<int>(currentKeySignature)]
                                [noteClass];
}

//------------------------------------------------------------------------
bool NotationView::keySignatureUsesSharp(int noteClass) const {
  if (noteClass < 0 || noteClass >= 7)
    return false;
  return keySignatureIsSharp[static_cast<int>(currentKeySignature)][noteClass];
}

//------------------------------------------------------------------------
} // namespace Ursulean