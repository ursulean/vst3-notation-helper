//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#include "processor.h"
#include "cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

using namespace Steinberg;

namespace Ursulean {
//------------------------------------------------------------------------
// NotationChordHelperProcessor
//------------------------------------------------------------------------
NotationChordHelperProcessor::NotationChordHelperProcessor() {
  //--- set the wanted controller for our processor
  setControllerClass(kNotationChordHelperControllerUID);
}

//------------------------------------------------------------------------
NotationChordHelperProcessor::~NotationChordHelperProcessor() {}

//------------------------------------------------------------------------
tresult PLUGIN_API NotationChordHelperProcessor::initialize(FUnknown *context) {
  // Here the Plug-in will be instantiated

  //---always initialize the parent-------
  tresult result = AudioEffect::initialize(context);
  // if everything Ok, continue
  if (result != kResultOk) {
    return result;
  }

  //--- create Audio IO ------
  // addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
  addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

  /* If you don't need an event bus, you can remove the next line */
  addEventInput(STR16("Event In"), 1);

  return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API NotationChordHelperProcessor::terminate() {
  // Here the Plug-in will be de-instantiated, last possibility to remove some
  // memory!

  //---do not forget to call parent ------
  return AudioEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API NotationChordHelperProcessor::setActive(TBool state) {
  //--- called when the Plug-in is enable/disable (On/Off) -----
  if (!state) {
    // Clear active notes when plugin is deactivated
    std::lock_guard<std::mutex> lock(activeNotesMutex);
    activeNotes.clear();
  }
  return AudioEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API
NotationChordHelperProcessor::process(Vst::ProcessData &data) {
  //--- Process MIDI events first
  if (data.inputEvents) {
    processMidiEvents(data.inputEvents);
  }

  // Send parameter updates if notes changed
  if (activeNotesChanged && data.outputParameterChanges) {
    std::lock_guard<std::mutex> lock(activeNotesMutex);

    // Convert active notes to vector and limit to 10 notes max
    std::vector<int> notesList(activeNotes.begin(), activeNotes.end());
    if (notesList.size() > 10) {
      notesList.resize(10); // Limit to first 10 notes
    }

    // Send each note to its corresponding parameter slot
    for (int i = 0; i < 10; i++) {
      int32 index = 0;
      auto *paramQueue =
          data.outputParameterChanges->addParameterData(i, index);
      if (paramQueue) {
        if (i < notesList.size()) {
          // Send the note as normalized value (0-127 maps to 0.0-1.0)
          double paramValue = static_cast<double>(notesList[i]) / 127.0;
          paramQueue->addPoint(0, paramValue, index);
        } else {
          // No note for this slot, send 0.0
          paramQueue->addPoint(0, 0.0, index);
        }
      }
    }

    activeNotesChanged = false;
  }

  //--- First : Read inputs parameter changes-----------
  // if (data.inputParameterChanges) {
  //   int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
  //   for (int32 index = 0; index < numParamsChanged; index++) {
  //     if (auto *paramQueue =
  //             data.inputParameterChanges->getParameterData(index)) {
  //       Vst::ParamValue value;
  //       int32 sampleOffset;
  //       int32 numPoints = paramQueue->getPointCount();
  //       switch (paramQueue->getParameterId()) {}
  //     }
  //   }

  //--- Here you have to implement your processing

  if (data.numSamples > 0) {
    //--- ------------------------------------------
    // here as example a default implementation where we try to copy the
    // inputs to the outputs: if less input than outputs then clear outputs
    //--- ------------------------------------------

    int32 minBus = std::min(data.numInputs, data.numOutputs);
    for (int32 i = 0; i < minBus; i++) {
      int32 minChan =
          std::min(data.inputs[i].numChannels, data.outputs[i].numChannels);
      for (int32 c = 0; c < minChan; c++) {
        // do not need to be copied if the buffers are the same
        if (data.outputs[i].channelBuffers32[c] !=
            data.inputs[i].channelBuffers32[c]) {
          memcpy(data.outputs[i].channelBuffers32[c],
                 data.inputs[i].channelBuffers32[c],
                 data.numSamples * sizeof(Vst::Sample32));
        }
      }
      data.outputs[i].silenceFlags = data.inputs[i].silenceFlags;

      // clear the remaining output buffers
      for (int32 c = minChan; c < data.outputs[i].numChannels; c++) {
        // clear output buffers
        memset(data.outputs[i].channelBuffers32[c], 0,
               data.numSamples * sizeof(Vst::Sample32));

        // inform the host that this channel is silent
        data.outputs[i].silenceFlags |= ((uint64)1 << c);
      }
    }
    // clear the remaining output buffers
    for (int32 i = minBus; i < data.numOutputs; i++) {
      // clear output buffers
      for (int32 c = 0; c < data.outputs[i].numChannels; c++) {
        memset(data.outputs[i].channelBuffers32[c], 0,
               data.numSamples * sizeof(Vst::Sample32));
      }
      // inform the host that this bus is silent
      data.outputs[i].silenceFlags =
          ((uint64)1 << data.outputs[i].numChannels) - 1;
    }
  }

  return kResultOk;
}

//------------------------------------------------------------------------
void NotationChordHelperProcessor::processMidiEvents(Vst::IEventList *events) {
  if (!events)
    return;

  int32 numEvents = events->getEventCount();
  for (int32 i = 0; i < numEvents; i++) {
    Vst::Event event;
    if (events->getEvent(i, event) == kResultOk) {
      switch (event.type) {
      case Vst::Event::kNoteOnEvent:
        if (event.noteOn.velocity > 0) {
          handleNoteOn(event.noteOn.pitch, event.noteOn.velocity);
        } else {
          // Velocity 0 is treated as note off
          handleNoteOff(event.noteOn.pitch);
        }
        break;
      case Vst::Event::kNoteOffEvent:
        handleNoteOff(event.noteOff.pitch);
        break;
      default:
        break;
      }
    }
  }
}

//------------------------------------------------------------------------
void NotationChordHelperProcessor::handleNoteOn(int pitch, int velocity) {
  std::lock_guard<std::mutex> lock(activeNotesMutex);
  activeNotes.insert(pitch);
  activeNotesChanged = true;
}

//------------------------------------------------------------------------
void NotationChordHelperProcessor::handleNoteOff(int pitch) {
  std::lock_guard<std::mutex> lock(activeNotesMutex);
  activeNotes.erase(pitch);
  activeNotesChanged = true;
}

//------------------------------------------------------------------------
tresult PLUGIN_API
NotationChordHelperProcessor::setupProcessing(Vst::ProcessSetup &newSetup) {
  //--- called before any processing ----
  return AudioEffect::setupProcessing(newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API
NotationChordHelperProcessor::canProcessSampleSize(int32 symbolicSampleSize) {
  // by default kSample32 is supported
  if (symbolicSampleSize == Vst::kSample32)
    return kResultTrue;

  // disable the following comment if your processing support kSample64
  /* if (symbolicSampleSize == Vst::kSample64)
          return kResultTrue; */

  return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API NotationChordHelperProcessor::setState(IBStream *state) {
  // called when we load a preset, the model has to be reloaded
  if (!state)
    return kResultFalse;

  IBStreamer streamer(state, kLittleEndian);

  // Read active notes from state
  int32 numNotes = 0;
  if (streamer.readInt32(numNotes) == kResultFalse)
    return kResultFalse;

  std::lock_guard<std::mutex> lock(activeNotesMutex);
  activeNotes.clear();

  for (int32 i = 0; i < numNotes; i++) {
    int32 note = 0;
    if (streamer.readInt32(note) == kResultFalse)
      return kResultFalse;
    if (note >= 0 && note <= 127) {
      activeNotes.insert(note);
    }
  }

  return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API NotationChordHelperProcessor::getState(IBStream *state) {
  // here we need to save the model
  if (!state)
    return kResultFalse;

  IBStreamer streamer(state, kLittleEndian);

  // Write active notes to state
  std::lock_guard<std::mutex> lock(activeNotesMutex);

  // Write number of active notes
  int32 numNotes = static_cast<int32>(activeNotes.size());
  if (streamer.writeInt32(numNotes) == kResultFalse)
    return kResultFalse;

  // Write each note
  for (int note : activeNotes) {
    if (streamer.writeInt32(static_cast<int32>(note)) == kResultFalse)
      return kResultFalse;
  }

  return kResultOk;
}

//------------------------------------------------------------------------
} // namespace Ursulean
