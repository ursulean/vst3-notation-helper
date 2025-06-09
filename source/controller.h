//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#pragma once

#include "notation_editor.h"
#include "notation_view.h"
#include "public.sdk/source/vst/vsteditcontroller.h"

namespace Ursulean {

// Parameter IDs for communication (support up to 10 simultaneous notes)
enum {
  kNote1Param = 0,
  kNote2Param = 1,
  kNote3Param = 2,
  kNote4Param = 3,
  kNote5Param = 4,
  kNote6Param = 5,
  kNote7Param = 6,
  kNote8Param = 7,
  kNote9Param = 8,
  kNote10Param = 9,
  kKeySignatureParam = 10,
  kNumParams = 11
};

// Key signature enumeration
enum KeySignature {
  kCMajor = 0,      // No accidentals
  kGMajor = 1,      // 1 sharp (F#)
  kDMajor = 2,      // 2 sharps (F#, C#)
  kAMajor = 3,      // 3 sharps (F#, C#, G#)
  kEMajor = 4,      // 4 sharps (F#, C#, G#, D#)
  kBMajor = 5,      // 5 sharps (F#, C#, G#, D#, A#)
  kFSharpMajor = 6, // 6 sharps (F#, C#, G#, D#, A#, E#)
  kCSharpMajor = 7, // 7 sharps (F#, C#, G#, D#, A#, E#, B#)
  kFMajor = 8,      // 1 flat (Bb)
  kBflatMajor = 9,  // 2 flats (Bb, Eb)
  kEflatMajor = 10, // 3 flats (Bb, Eb, Ab)
  kAflatMajor = 11, // 4 flats (Bb, Eb, Ab, Db)
  kDflatMajor = 12, // 5 flats (Bb, Eb, Ab, Db, Gb)
  kGflatMajor = 13, // 6 flats (Bb, Eb, Ab, Db, Gb, Cb)
  kCflatMajor = 14, // 7 flats (Bb, Eb, Ab, Db, Gb, Cb, Fb)
  kNumKeySigs = 15
};

//------------------------------------------------------------------------
//  NotationChordHelperController
//------------------------------------------------------------------------
class NotationChordHelperController : public Steinberg::Vst::EditControllerEx1 {
public:
  //------------------------------------------------------------------------
  NotationChordHelperController() = default;
  ~NotationChordHelperController() SMTG_OVERRIDE = default;

  // Create function
  static Steinberg::FUnknown *createInstance(void * /*context*/) {
    return (Steinberg::Vst::IEditController *)new NotationChordHelperController;
  }

  //--- from IPluginBase -----------------------------------------------
  Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown *context)
      SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

  //--- from EditController --------------------------------------------
  Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream *state)
      SMTG_OVERRIDE;
  Steinberg::IPlugView *PLUGIN_API createView(Steinberg::FIDString name)
      SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream *state)
      SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream *state)
      SMTG_OVERRIDE;

  // Parameter handling
  Steinberg::tresult PLUGIN_API
  setParamNormalized(Steinberg::Vst::ParamID tag,
                     Steinberg::Vst::ParamValue value) SMTG_OVERRIDE;

  // Custom methods for notation display
  void setActiveNotes(const std::vector<int> &notes);
  KeySignature getCurrentKeySignature() const { return currentKeySignature; }

  //---Interface---------
  DEFINE_INTERFACES
  // Here you can add more supported VST3 interfaces
  // DEF_INTERFACE (Vst::IXXX)
  END_DEFINE_INTERFACES(EditController)
  DELEGATE_REFCOUNT(EditController)

  //------------------------------------------------------------------------
protected:
  NotationEditor *currentEditor = nullptr;
  std::vector<int> lastActiveNotes;
  std::vector<int>
      currentNoteParams; // Track which MIDI note is in each parameter slot
  KeySignature currentKeySignature = kCMajor;
};

//------------------------------------------------------------------------
} // namespace Ursulean
