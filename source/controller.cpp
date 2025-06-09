//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#include "controller.h"
#include "base/source/fstreamer.h"
#include "cids.h"
#include "notation_editor.h"
#include "vstgui/lib/cviewcontainer.h"
#include "vstgui/plugin-bindings/vst3editor.h"

using namespace Steinberg;

namespace Ursulean {

//------------------------------------------------------------------------
// NotationChordHelperController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API
NotationChordHelperController::initialize(FUnknown *context) {
  // Here the Plug-in will be instantiated

  //---do not forget to call parent ------
  tresult result = EditControllerEx1::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  // Initialize note parameter tracking
  currentNoteParams.resize(10, -1); // -1 means no note in this slot

  // Register parameters for communication (10 note parameters)
  parameters.addParameter(STR16("Note 1"), nullptr, 0, 0,
                          Steinberg::Vst::ParameterInfo::kIsReadOnly,
                          kNote1Param);
  parameters.addParameter(STR16("Note 2"), nullptr, 0, 0,
                          Steinberg::Vst::ParameterInfo::kIsReadOnly,
                          kNote2Param);
  parameters.addParameter(STR16("Note 3"), nullptr, 0, 0,
                          Steinberg::Vst::ParameterInfo::kIsReadOnly,
                          kNote3Param);
  parameters.addParameter(STR16("Note 4"), nullptr, 0, 0,
                          Steinberg::Vst::ParameterInfo::kIsReadOnly,
                          kNote4Param);
  parameters.addParameter(STR16("Note 5"), nullptr, 0, 0,
                          Steinberg::Vst::ParameterInfo::kIsReadOnly,
                          kNote5Param);
  parameters.addParameter(STR16("Note 6"), nullptr, 0, 0,
                          Steinberg::Vst::ParameterInfo::kIsReadOnly,
                          kNote6Param);
  parameters.addParameter(STR16("Note 7"), nullptr, 0, 0,
                          Steinberg::Vst::ParameterInfo::kIsReadOnly,
                          kNote7Param);
  parameters.addParameter(STR16("Note 8"), nullptr, 0, 0,
                          Steinberg::Vst::ParameterInfo::kIsReadOnly,
                          kNote8Param);
  parameters.addParameter(STR16("Note 9"), nullptr, 0, 0,
                          Steinberg::Vst::ParameterInfo::kIsReadOnly,
                          kNote9Param);
  parameters.addParameter(STR16("Note 10"), nullptr, 0, 0,
                          Steinberg::Vst::ParameterInfo::kIsReadOnly,
                          kNote10Param);

  // Add key signature parameter
  parameters.addParameter(STR16("Key Signature"), STR16("Key"), 0, 0,
                          Steinberg::Vst::ParameterInfo::kCanAutomate |
                              Steinberg::Vst::ParameterInfo::kIsList,
                          kKeySignatureParam);

  return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API NotationChordHelperController::terminate() {
  // Here the Plug-in will be de-instantiated, last possibility to remove some
  // memory!

  //---do not forget to call parent ------
  return EditControllerEx1::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API
NotationChordHelperController::setComponentState(IBStream *state) {
  // This method is called when loading plugin state (presets, project loading)
  // NOT for real-time communication - that's handled by setParamNormalized()

  if (!state)
    return kResultFalse;

  // Read saved processor state for preset loading
  IBStreamer streamer(state, kLittleEndian);

  // Read number of saved notes (for preset state)
  int32 numNotes = 0;
  if (streamer.readInt32(numNotes) == kResultFalse)
    return kResultOk;

  // Read the saved notes and restore display state
  std::vector<int> savedNotes;
  for (int32 i = 0; i < numNotes; i++) {
    int32 note = 0;
    if (streamer.readInt32(note) == kResultOk) {
      if (note >= 0 && note <= 127) {
        savedNotes.push_back(note);
      }
    }
  }

  // Only update display if this is a preset load, not real-time MIDI
  // In most cases for a notation display, we'd start with empty staff
  // Real-time updates come through setParamNormalized()

  return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API NotationChordHelperController::setState(IBStream *state) {
  // Here you get the state of the controller

  return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API NotationChordHelperController::getState(IBStream *state) {
  // Here you are asked to deliver the state of the controller (if needed)
  // Note: the real state of your plug-in is saved in the processor

  return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView *PLUGIN_API
NotationChordHelperController::createView(FIDString name) {
  // Here the Host wants to open your editor (if you have one)
  if (FIDStringsEqual(name, Vst::ViewType::kEditor)) {
    // Create our custom notation editor
    currentEditor = new NotationEditor(this, "view", "editor.uidesc");

    // Editor will start with empty staff, ready for MIDI input

    return currentEditor;
  }
  return nullptr;
}

//------------------------------------------------------------------------
void NotationChordHelperController::setActiveNotes(
    const std::vector<int> &notes) {
  if (currentEditor) {
    currentEditor->setActiveNotes(notes);
  }
}

//------------------------------------------------------------------------
tresult PLUGIN_API NotationChordHelperController::setParamNormalized(
    Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue value) {
  // Handle parameter changes from processor
  tresult result = EditControllerEx1::setParamNormalized(tag, value);

  if (tag >= 0 && tag < 10) {
    // Update the note in this parameter slot
    if (value > 0.0) {
      // Decode the MIDI note from the normalized value (0.0-1.0 maps to 0-127)
      int midiNote =
          static_cast<int>(value * 127.0 + 0.5); // Add 0.5 for rounding
      if (midiNote >= 0 && midiNote <= 127) {
        currentNoteParams[tag] = midiNote;
      }
    } else {
      // Value is 0.0, clear this note slot
      currentNoteParams[tag] = -1;
    }

    // Collect all active notes from all parameter slots
    std::vector<int> activeNotes;
    for (int noteParam : currentNoteParams) {
      if (noteParam >= 0) { // -1 means empty slot
        activeNotes.push_back(noteParam);
      }
    }

    // Update the notation display with all active notes
    setActiveNotes(activeNotes);
  } else if (tag == kKeySignatureParam) {
    // Handle key signature parameter change
    int keyIndex = static_cast<int>(value * (kNumKeySigs - 1) + 0.5);
    if (keyIndex >= 0 && keyIndex < kNumKeySigs) {
      currentKeySignature = static_cast<KeySignature>(keyIndex);
      // Update the notation view with the new key signature
      if (currentEditor) {
        currentEditor->setKeySignature(currentKeySignature);
      }
    }
  }

  return result;
}

//------------------------------------------------------------------------
} // namespace Ursulean
