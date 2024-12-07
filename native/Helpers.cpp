//
// Created by Cristian Vogel on 06/12/2024.
//

//======= HELPERS ==============================
// A helper for locating bundled asset files
#include <random>
#include "Helpers.h"

namespace util
{
    choc::midi::ShortMessage generateRandomMIDINoteMessage() {
        // Set up the random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(40, 70);

        const int note = dis(gen);
        constexpr int velocity = 60;

        // Convert to hexadecimal and format a MIDI message for channel 1 using juce::String
        const choc::midi::ShortMessage midiMessage( 0x90, note, velocity );
        return midiMessage;
    }


    juce::File getAssetsDirectory()
    {
#if JUCE_MAC
        auto assetsDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentApplicationFile)
            .getChildFile("Contents/Resources/dist");
#elif JUCE_WINDOWS
     auto assetsDir =
         juce::File::getSpecialLocation(
             juce::File::SpecialLocationType::currentExecutableFile) // Plugin.vst3/Contents/<arch>/Plugin.vst3
             .getParentDirectory()                                   // Plugin.vst3/Contents/<arch>/
             .getParentDirectory()                                   // Plugin.vst3/Contents/
             .getChildFile("Resources/dist");
#else
#error "We only support Mac and Windows here yet."
#endif

        return assetsDir;
    }
} // end util
