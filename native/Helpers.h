//
// Created by Cristian Vogel on 06/12/2024.
//

#ifndef HELPERS_H
#define HELPERS_H

#include <choc_MIDI.h>
#include <juce_audio_basics/juce_audio_basics.h>

namespace util
{
    juce::File getAssetsDirectory();
    bool isOdd(int num);
    choc::midi::ShortMessage generateRandomMIDINoteMessage();

} // namespace util

#endif //HELPERS_H
