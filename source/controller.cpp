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

  // Register parameters for communication
  parameters.addParameter(STR16("Active Notes"), nullptr, 0, 0,
                          Steinberg::Vst::ParameterInfo::kIsReadOnly,
                          kActiveNotesParam);

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
  // Here you get the state of the component (Processor part)
  if (!state)
    return kResultFalse;

  // Read the active notes from the processor state
  IBStreamer streamer(state, kLittleEndian);

  // Read number of active notes
  int32 numNotes = 0;
  if (streamer.readInt32(numNotes) == kResultFalse)
    return kResultOk;

  // Read the notes
  std::vector<int> activeNotes;
  for (int32 i = 0; i < numNotes; i++) {
    int32 note = 0;
    if (streamer.readInt32(note) == kResultOk) {
      if (note >= 0 && note <= 127) {
        activeNotes.push_back(note);
      }
    }
  }

  // Update the notation display
  setActiveNotes(activeNotes);

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

  if (tag == kActiveNotesParam) {
    // For now, just trigger a test note display when parameter changes
    // In a full implementation, we'd decode the actual note data
    std::vector<int> testNotes;
    if (value > 0.5) { // Simple test: if parameter value > 0.5, show some notes
      testNotes = {60, 64, 67}; // C Major chord
    }
    setActiveNotes(testNotes);
  }

  return result;
}

//------------------------------------------------------------------------
} // namespace Ursulean
