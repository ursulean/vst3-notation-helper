//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#pragma once

#include "notation_editor.h"
#include "notation_view.h"
#include "public.sdk/source/vst/vsteditcontroller.h"

namespace Ursulean {

// Parameter IDs for communication
enum { kActiveNotesParam = 0, kNumParams };

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
};

//------------------------------------------------------------------------
} // namespace Ursulean
