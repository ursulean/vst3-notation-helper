//------------------------------------------------------------------------
// Copyright(c) 2025 Paul Ursulean.
//------------------------------------------------------------------------

#pragma once

namespace Ursulean {

// Key signature enumeration
enum KeySignature {
  kCMajor = 0,      // No accidentals
  kGMajor = 1,      // 1 sharp (F#)
  kDMajor = 2,      // 2 sharps (F#, C#)
  kAMajor = 3,      // 3 sharps (F#, C#, G#)
  kEMajor = 4,      // 4 sharps (F#, C#, G#, D#)
  kBMajor = 5,      // 5 sharps (F#, C#, G#, D#, A#)
  kFSharpMajor = 6, // 6 sharps (F#, C#, G#, D#, A#, E#)
  kCSharpMajor = 7, // 7 sharps (F#, C#, G#, D#, A#, E#, B#)
  kFMajor = 8,      // 1 flat (Bb)
  kBflatMajor = 9,  // 2 flats (Bb, Eb)
  kEflatMajor = 10, // 3 flats (Bb, Eb, Ab)
  kAflatMajor = 11, // 4 flats (Bb, Eb, Ab, Db)
  kDflatMajor = 12, // 5 flats (Bb, Eb, Ab, Db, Gb)
  kGflatMajor = 13, // 6 flats (Bb, Eb, Ab, Db, Gb, Cb)
  kCflatMajor = 14, // 7 flats (Bb, Eb, Ab, Db, Gb, Cb, Fb)
  kNumKeySigs = 15
};

} // namespace Ursulean