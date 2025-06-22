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
  void drawTrebleClef(VSTGUI::CDrawContext *context, double x, double y);
  void drawBassClef(VSTGUI::CDrawContext *context, double x, double y);
  void drawNotes(VSTGUI::CDrawContext *context, const VSTGUI::CRect &rect);
  void drawNote(VSTGUI::CDrawContext *context, double x, double y,
                bool filled = true);
  void drawAccidental(VSTGUI::CDrawContext *context, double x, double y,
                      bool isSharp);
  void drawNatural(VSTGUI::CDrawContext *context, double x, double y);
  void drawLedgerLine(VSTGUI::CDrawContext *context, double x, double y,
                      double width);
  void drawLedgerLinesForNote(VSTGUI::CDrawContext *context, double x,
                              double noteY, int midiNote);
  void drawKeySignature(VSTGUI::CDrawContext *context,
                        const VSTGUI::CRect &rect);

  // Helper methods
  double getStaffPosition(int midiNote, bool &isOnTrebleStaff,
                          bool &needsAccidental, bool &isSharp,
                          bool &isNatural);
  bool needsLedgerLine(int midiNote, bool isOnTrebleStaff);

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

private:
  // Proportional sizing helper - all dimensions based on view size
  struct Dimensions {
    double width;
    double height;

    // Intuitive proportional constants based on semantic layout
    static constexpr double STAFF_LINE_HEIGHT_RATIO =
        0.05; // 5% of window height per staff line space
    static constexpr double GRAND_STAFF_GAP_RATIO =
        0.1; // 15% gap between staves

    // Layout margins and spacing
    static constexpr double LEFT_MARGIN_RATIO =
        0.15; // 15% for clefs and key signature
    static constexpr double RIGHT_MARGIN_RATIO = 0.05; // 5% right margin
    static constexpr double CLEF_WIDTH_RATIO =
        0.1; // 10% of width for the clef symbol area

    // Spacing and positioning
    static constexpr double ACCIDENTAL_SPACING_RATIO =
        0.025; // 2.5% between key sig accidentals
    static constexpr double NOTE_GROUP_SPACING_RATIO =
        0.05; // 5% between chord groups
    static constexpr double LEDGER_LINE_WIDTH_RATIO =
        0.04; // 4% ledger line width
    static constexpr double ACCIDENTAL_OFFSET_RATIO =
        0.04; // 4% offset for accidentals from notes
    static constexpr double KEY_SIGNATURE_PADDING_RATIO =
        0.03; // 3% padding around key signature

    // Symbol drawing proportions (relative to smaller dimension for
    // consistency)
    static constexpr double SYMBOL_BASE_SIZE_RATIO =
        0.03; // 3% of smaller dimension

    // Calculate actual dimensions
    double staffLineHeight() const { return height * STAFF_LINE_HEIGHT_RATIO; }
    double grandStaffGap() const { return staffLineHeight() * 2.0; }
    double noteWidth() const { return staffLineHeight() * 1.3; }
    double noteHeight() const { return staffLineHeight() * 0.94; }
    double clefWidth() const { return width * CLEF_WIDTH_RATIO; }
    double leftMargin() const { return width * LEFT_MARGIN_RATIO; }
    double rightMargin() const { return width * RIGHT_MARGIN_RATIO; }
    double clefFontSize() const { return staffLineHeight() * 6.0; }
    double accidentalSpacing() const {
      return width * ACCIDENTAL_SPACING_RATIO;
    }
    double noteGroupSpacing() const { return width * NOTE_GROUP_SPACING_RATIO; }
    double ledgerLineWidth() const { return noteWidth() * 1.5; }
    double accidentalOffset() const { return noteWidth() * 2.0; }
    double keySignaturePadding() const {
      return width * KEY_SIGNATURE_PADDING_RATIO;
    }

    // Symbol drawing proportions
    double symbolBaseSize() const {
      return std::min(width, height) * SYMBOL_BASE_SIZE_RATIO;
    }
  };

  Dimensions getDimensions() const {
    VSTGUI::CRect rect = getViewSize();
    return {rect.getWidth(), rect.getHeight()};
  }
};

//------------------------------------------------------------------------
} // namespace Ursulean