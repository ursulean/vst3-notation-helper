//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#pragma once

#include "notation_view.h"
#include "vstgui/plugin-bindings/vst3editor.h"

namespace Ursulean {

//------------------------------------------------------------------------
// NotationEditor - Custom VST3 Editor with notation display
//------------------------------------------------------------------------
class NotationEditor : public VSTGUI::VST3Editor {
public:
  NotationEditor(void *controller, VSTGUI::UTF8StringPtr templateName,
                 VSTGUI::UTF8StringPtr xmlFile);
  virtual ~NotationEditor() = default;

  // VST3Editor overrides
  bool PLUGIN_API open(void *parent,
                       const VSTGUI::PlatformType &platformType) override;
  void PLUGIN_API close() override;

  // Update the notation display
  void setActiveNotes(const std::vector<int> &notes);

protected:
  NotationView *notationView = nullptr;
};

//------------------------------------------------------------------------
} // namespace Ursulean