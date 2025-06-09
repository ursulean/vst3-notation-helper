//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#include "notation_editor.h"
#include "controller.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/lib/controls/ctextlabel.h"
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

  // Get the frame and add our controls
  if (auto frame = getFrame()) {
    // Remove any existing views to make room for our custom interface
    frame->removeAll();

    // Set frame size (increased height to accommodate dropdown)
    VSTGUI::CRect frameSize(0, 0, 600, 450);
    frame->setSize(frameSize.getWidth(), frameSize.getHeight());

    // Create a label for the key signature dropdown
    VSTGUI::CRect labelRect(10, 10, 120, 30);
    auto *label = new VSTGUI::CTextLabel(labelRect, "Key Signature:");
    label->setFontColor(VSTGUI::CColor(0, 0, 0, 255));
    label->setBackColor(VSTGUI::CColor(250, 250, 250, 255));
    label->setFrameColor(VSTGUI::CColor(250, 250, 250, 255));
    frame->addView(label);

    // Create the key signature dropdown
    VSTGUI::CRect menuRect(130, 10, 300, 30);
    keySignatureMenu =
        new VSTGUI::COptionMenu(menuRect, this, kKeySignatureParam);

    // Add all key signature options
    keySignatureMenu->addEntry("C Major");
    keySignatureMenu->addEntry("G Major (1♯)");
    keySignatureMenu->addEntry("D Major (2♯)");
    keySignatureMenu->addEntry("A Major (3♯)");
    keySignatureMenu->addEntry("E Major (4♯)");
    keySignatureMenu->addEntry("B Major (5♯)");
    keySignatureMenu->addEntry("F♯ Major (6♯)");
    keySignatureMenu->addEntry("C♯ Major (7♯)");
    keySignatureMenu->addEntry("F Major (1♭)");
    keySignatureMenu->addEntry("B♭ Major (2♭)");
    keySignatureMenu->addEntry("E♭ Major (3♭)");
    keySignatureMenu->addEntry("A♭ Major (4♭)");
    keySignatureMenu->addEntry("D♭ Major (5♭)");
    keySignatureMenu->addEntry("G♭ Major (6♭)");
    keySignatureMenu->addEntry("C♭ Major (7♭)");

    // Set initial value to C Major (index 0)
    keySignatureMenu->setValue(0.0f);
    frame->addView(keySignatureMenu);

    // Create our notation view (positioned below the dropdown)
    VSTGUI::CRect notationRect(10, 50, 590, 440);
    notationView = new NotationView(notationRect);
    frame->addView(notationView);

    // Set initial key signature in notation view
    if (auto controller =
            dynamic_cast<NotationChordHelperController *>(getController())) {
      notationView->setKeySignature(controller->getCurrentKeySignature());
    }
  }

  return true;
}

//------------------------------------------------------------------------
void NotationEditor::close() {
  if (notationView) {
    notationView = nullptr; // The frame will handle deletion
  }
  if (keySignatureMenu) {
    keySignatureMenu = nullptr; // The frame will handle deletion
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
void NotationEditor::setKeySignature(KeySignature keySignature) {
  if (notationView) {
    notationView->setKeySignature(keySignature);
  }

  // Update the dropdown to reflect the new key signature
  if (keySignatureMenu) {
    // Set the dropdown to the correct index directly
    keySignatureMenu->setValue(static_cast<float>(keySignature));
  }
}

//------------------------------------------------------------------------
void NotationEditor::valueChanged(VSTGUI::CControl *pControl) {
  if (pControl == keySignatureMenu) {
    // User changed the key signature dropdown
    float selectedIndex = keySignatureMenu->getValue();

    // Convert dropdown index to normalized parameter value
    float normalizedValue = selectedIndex / (kNumKeySigs - 1);

    // Update the parameter through the controller
    if (auto controller = getController()) {
      controller->setParamNormalized(kKeySignatureParam, normalizedValue);
      controller->performEdit(kKeySignatureParam, normalizedValue);
    }
  }

  // Call parent implementation
  VST3Editor::valueChanged(pControl);
}

//------------------------------------------------------------------------
} // namespace Ursulean