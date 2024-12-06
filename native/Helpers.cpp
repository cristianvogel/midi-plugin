//
// Created by Cristian Vogel on 06/12/2024.
//

//======= HELPERS ==============================
 // A helper for locating bundled asset files

#include "Helpers.h"
#include <elem/Value.h>

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
