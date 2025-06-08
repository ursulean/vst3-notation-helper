//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace Ursulean {
//------------------------------------------------------------------------
static const Steinberg::FUID kNotationChordHelperProcessorUID(0xDEA1730E,
                                                              0x1F515AF1,
                                                              0xB8D0AA16,
                                                              0x0EA0F195);
static const Steinberg::FUID kNotationChordHelperControllerUID(0x8C9D4372,
                                                               0x75B15546,
                                                               0xAA9E0A0F,
                                                               0x81415EF4);

#define NotationChordHelperVST3Category "Instrument"

//------------------------------------------------------------------------
} // namespace Ursulean
