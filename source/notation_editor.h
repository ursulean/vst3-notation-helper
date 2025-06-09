//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#pragma once

#include "notation_view.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/plugin-bindings/vst3editor.h"

namespace Ursulean {

// Forward declaration to avoid circular include
enum KeySignature;

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
  void setKeySignature(KeySignature keySignature);

  // VST3Editor overrides for parameter updates
  void valueChanged(VSTGUI::CControl *pControl) override;

protected:
  NotationView *notationView = nullptr;
  VSTGUI::COptionMenu *keySignatureMenu = nullptr;
};

//------------------------------------------------------------------------
} // namespace Ursulean