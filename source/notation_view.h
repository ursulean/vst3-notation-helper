//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#pragma once

#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cview.h"
#include <map>
#include <vector>

namespace Ursulean {

// Forward declaration to avoid circular include
enum KeySignature;

//------------------------------------------------------------------------
// NotationView - Custom view for displaying musical notation
//------------------------------------------------------------------------
class NotationView : public VSTGUI::CView {
public:
  NotationView(const VSTGUI::CRect &size);
  ~NotationView() override = default;

  // CView overrides
  void draw(VSTGUI::CDrawContext *context) override;

  // Update the currently active notes
  void setActiveNotes(const std::vector<int> &notes);

  // Set the key signature
  void setKeySignature(KeySignature keySignature);

private:
  // Drawing methods
  void drawStaff(VSTGUI::CDrawContext *context, const VSTGUI::CRect &rect);
  void drawTrebleClef(VSTGUI::CDrawContext *context, double x, double y,
                      double scale);
  void drawBassClef(VSTGUI::CDrawContext *context, double x, double y,
                    double scale);
  void drawNotes(VSTGUI::CDrawContext *context, const VSTGUI::CRect &rect);
  void drawNote(VSTGUI::CDrawContext *context, double x, double y,
                bool filled = true);
  void drawAccidental(VSTGUI::CDrawContext *context, double x, double y,
                      bool isSharp);
  void drawNatural(VSTGUI::CDrawContext *context, double x, double y);
  void drawLedgerLine(VSTGUI::CDrawContext *context, double x, double y,
                      double width);
  void drawKeySignature(VSTGUI::CDrawContext *context,
                        const VSTGUI::CRect &rect);

  // Helper methods
  double getStaffPosition(int midiNote, bool &isOnTrebleStaff,
                          bool &needsAccidental, bool &isSharp,
                          bool &isNatural);
  bool needsLedgerLine(int midiNote, bool isOnTrebleStaff);
  double getHorizontalOffset(int noteIndex,
                             const std::vector<double> &staffPositions);

  // Key signature helper methods
  bool isNoteInKeySignature(int noteClass) const;
  bool keySignatureUsesSharp(int noteClass) const;

  // Smart note positioning helpers
  std::vector<std::vector<int>>
  groupNotesByPosition(const std::vector<int> &sortedNotes,
                       const std::vector<double> &staffPositions,
                       const std::vector<bool> &needsAccidental);
  bool needsSideBySidePositioning(const std::vector<int> &group,
                                  const std::vector<double> &staffPositions,
                                  const std::vector<bool> &needsAccidental);

  // Note mapping data
  void initializeNoteMappings();

  std::vector<int> activeNotes;
  std::map<int, double> noteToStaffPosition; // MIDI note to staff line position
  std::map<int, bool> noteNeedsAccidental;   // Which notes need sharps/flats
  std::map<int, bool> noteIsSharp;           // True for sharp, false for flat

  // Key signature data
  KeySignature currentKeySignature = static_cast<KeySignature>(0); // C Major
  static const bool keySignatureAccidentals[15][7]; // Which note classes have
                                                    // accidentals in each key
  static const bool
      keySignatureIsSharp[15][7]; // Whether each accidental is sharp or flat

  // Drawing constants
  static constexpr double STAFF_LINE_HEIGHT = 8.0;
  static constexpr double STAFF_SPACING =
      STAFF_LINE_HEIGHT * 4.0; // 4 spaces between 5 lines
  static constexpr double GRAND_STAFF_GAP =
      STAFF_LINE_HEIGHT; // Just one staff line height between staves
  static constexpr double NOTE_WIDTH = 10.0; // Slightly bigger noteheads
  static constexpr double NOTE_HEIGHT = 6.5; // Proportional increase
  static constexpr double CLEF_WIDTH = 40.0;
  static constexpr double LEFT_MARGIN = 50.0;
  static constexpr double RIGHT_MARGIN = 20.0;
};

//------------------------------------------------------------------------
} // namespace Ursulean