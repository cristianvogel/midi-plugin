//
// Created by Cristian Vogel on 06/12/2024.
//

#ifndef HELPERS_H
#define HELPERS_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <elem/Value.h>

#include "PluginProcessor.h"
namespace mh
{
    namespace util
    {
        juce::File getAssetsDirectory();
        bool isOdd(int num);

        elem::js::Value wrapChordsToJsValue(const std::vector<MindfulMIDI::ChordNotes>& chordProgession);

    } // namespace util
} // namespace mh
#endif //HELPERS_H
