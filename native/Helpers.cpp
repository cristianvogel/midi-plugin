//
// Created by Cristian Vogel on 06/12/2024.
//

//======= HELPERS ==============================
// A helper for locating bundled asset files

#include "Helpers.h"

namespace mh
{
    namespace util
    {
        elem::js::Value wrapChordsToJsValue(const std::vector<MindfulMIDI::ChordNotes>& chordProgession)
        {
            elem::js::Array chordProgressionJs;

            // Convert std::vector<ChordNotes> to elem::js::Array
            for (const auto& chord : chordProgession)
            {
                elem::js::Array chordJs;
                for (const uint8_t note : chord.noteNumbers)
                {
                    chordJs.push_back(static_cast<elem::js::Number>(note));
                }
                chordProgressionJs.push_back(chordJs);
            }
            return elem::js::Value(chordProgressionJs);
        }

        //////////////////////////////////////////
        /////////////////////////////////////////
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
} // end mh
