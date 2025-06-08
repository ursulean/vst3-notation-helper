//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include <mutex>
#include <set>
#include <vector>

namespace Ursulean {

//------------------------------------------------------------------------
//  NotationChordHelperProcessor
//------------------------------------------------------------------------
class NotationChordHelperProcessor : public Steinberg::Vst::AudioEffect {
public:
  NotationChordHelperProcessor();
  ~NotationChordHelperProcessor() SMTG_OVERRIDE;

  // Create function
  static Steinberg::FUnknown *createInstance(void * /*context*/) {
    return (Steinberg::Vst::IAudioProcessor *)new NotationChordHelperProcessor;
  }

  //--- ---------------------------------------------------------------------
  // AudioEffect overrides:
  //--- ---------------------------------------------------------------------
  /** Called at first after constructor */
  Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown *context)
      SMTG_OVERRIDE;

  /** Called at the end before destructor */
  Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

  /** Switch the Plug-in on/off */
  Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) SMTG_OVERRIDE;

  /** Will be called before any process call */
  Steinberg::tresult PLUGIN_API
  setupProcessing(Steinberg::Vst::ProcessSetup &newSetup) SMTG_OVERRIDE;

  /** Asks if a given sample size is supported see SymbolicSampleSizes. */
  Steinberg::tresult PLUGIN_API
  canProcessSampleSize(Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

  /** Here we go...the process call */
  Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData &data)
      SMTG_OVERRIDE;

  /** For persistence */
  Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream *state)
      SMTG_OVERRIDE;
  Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream *state)
      SMTG_OVERRIDE;

  // Get currently active notes for the UI
  std::vector<int> getActiveNotes() const {
    std::lock_guard<std::mutex> lock(activeNotesMutex);
    return std::vector<int>(activeNotes.begin(), activeNotes.end());
  }

  //------------------------------------------------------------------------
protected:
  void processMidiEvents(Steinberg::Vst::IEventList *events);
  void handleNoteOn(int pitch, int velocity);
  void handleNoteOff(int pitch);

private:
  std::set<int> activeNotes;           // Currently pressed MIDI notes (0-127)
  mutable std::mutex activeNotesMutex; // Protect access to activeNotes
  bool activeNotesChanged = false;     // Flag to indicate notes have changed
};

//------------------------------------------------------------------------
} // namespace Ursulean
