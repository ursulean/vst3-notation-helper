//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#include "notation_editor.h"
#include "vstgui/lib/cviewcontainer.h"

namespace Ursulean {

//------------------------------------------------------------------------
NotationEditor::NotationEditor(void *controller,
                               VSTGUI::UTF8StringPtr templateName,
                               VSTGUI::UTF8StringPtr xmlFile)
    : VST3Editor(static_cast<Steinberg::Vst::EditController *>(controller),
                 templateName, xmlFile) {}

//------------------------------------------------------------------------
bool NotationEditor::open(void *parent,
                          const VSTGUI::PlatformType &platformType) {
  // Call parent implementation
  if (!VST3Editor::open(parent, platformType)) {
    return false;
  }

  // Get the frame and add our notation view
  if (auto frame = getFrame()) {
    // Remove any existing views to make room for our notation view
    frame->removeAll();

    // Set frame size
    VSTGUI::CRect frameSize(0, 0, 600, 400);
    frame->setSize(frameSize.getWidth(), frameSize.getHeight());

    // Create our notation view
    VSTGUI::CRect notationRect(10, 10, 590, 390);
    notationView = new NotationView(notationRect);
    frame->addView(notationView);

    // Don't set test notes here anymore - they'll come from the controller
  }

  return true;
}

//------------------------------------------------------------------------
void NotationEditor::close() {
  if (notationView) {
    notationView = nullptr; // The frame will handle deletion
  }
  VST3Editor::close();
}

//------------------------------------------------------------------------
void NotationEditor::setActiveNotes(const std::vector<int> &notes) {
  if (notationView) {
    notationView->setActiveNotes(notes);
  }
}

//------------------------------------------------------------------------
} // namespace Ursulean