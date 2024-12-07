#ifndef PLUGINPROCESSOR_H
#define PLUGINPROCESSOR_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include <choc_javascript.h>
#include <choc_javascript_Console.h>
#include <choc_HighResolutionSteadyClock.h>
#include <choc_MIDI.h>
#include <choc_SingleReaderSingleWriterFIFO.h>
#include <elem/Runtime.h>

// Forward Declarations
class WebViewEditor;

//==============================================================================
class MindfulMIDI final : public juce::AudioProcessor,
                          public juce::AudioProcessorParameter::Listener,
                          private juce::AsyncUpdater

{
public:
    //==============================================================================
    MindfulMIDI();
    ~MindfulMIDI() override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    WebViewEditor* editor = nullptr;
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void handleMidiOut(const std::string& _msg, int index);

    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    //==============================================================================
    /** Implement the AudioProcessorParameter::Listener interface. */
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override;
    //==============================================================================
    /** Implement the AsyncUpdater interface. */
    void handleAsyncUpdate() override;
    //==============================================================================
    /** Internal helper for initializing the embedded JS engine. */
    void initJavaScriptEngine();
    static std::string serialize(const std::string& function, const elem::js::Object& data,
                      const juce::String& replacementChar = "%");
    static std::string serialize(const std::string& function, const choc::value::Value& data,
                          const juce::String& replacementChar = "%");
    //=== Dispatchers
    void dispatchStateChange();
    void dispatchTableContentStateChange();
    void dispatchError(std::string const& name, std::string const& message);
    void dispatchLogToUI( std::string const& text ) const;

    //=== MIDI business
    void dispatchMIDItoJS( );

    //=== Harmony Persistent State
    struct ChordNotes
    {
        std::vector<int> noteNumbers ={ 0, 0, 0 };
    };
    struct TableContent
    {
        std::vector<ChordNotes> chordProgression = {};
    };



    //=== State
    elem::js::Object state;
    elem::js::Object tableContent;

private:
    //=== MIDI business
    using MIDIClock = choc::HighResolutionSteadyClock;

    struct IncomingMIDIEvent
    {
        MIDIClock::time_point time;
        choc::midi::ShortMessage message;
    };

    struct OutgoingMIDIEvent
    {
        //MIDIClock::time_point time;
        choc::midi::ShortMessage message;
        int index = 0;
    };
    choc::fifo::SingleReaderSingleWriterFIFO<IncomingMIDIEvent> midi_in_fifo_queue;
    choc::fifo::SingleReaderSingleWriterFIFO<OutgoingMIDIEvent> midi_out_fifo_queue;



    //=== JS Engine
    choc::javascript::Context jsEngine;

    //=== Audio Engine
    std::atomic<bool> runtimeSwapRequired{false};
    std::atomic<bool> shouldInitialize { false };
    double lastKnownSampleRate = 0;
    int lastKnownBlockSize = 0;
    juce::AudioBuffer<float> scratchBuffer;
    std::unique_ptr<elem::Runtime<float>> elementaryRuntime;
    std::map<std::string, juce::AudioParameterFloat*> parameterMap;

    //==============================================================================
    // A simple "dirty list" abstraction here for propagating realtime parameter
    // value changes
    struct ParameterReadout {
        float value = 0;
        bool dirty = false;
    };

    std::list<std::atomic<ParameterReadout>> paramReadouts;
    static_assert(std::atomic<ParameterReadout>::is_always_lock_free);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MindfulMIDI)
};

namespace staticNames
{
    inline std::string TABLE_CONTENT = "tableContent";
    inline std::string MAIN_DSP_JS_FILE = "dsp.main.js";
    inline std::string SAMPLE_RATE = "sampleRate";
    inline std::string NATIVE_MESSAGE_FUNCTION_NAME = "__postNativeMessage__";
    inline std::string LOG_FUNCTION_NAME = "__log__";
    inline std::string NOTE_NUMEBRS = "noteNumbers";
    inline std::string CHORD_PROGRESSION = "chordProgression";
}


namespace jsFunctions
{
    inline auto receiveMidiScript = R"script(
    (function() {
      if (typeof globalThis.__receiveMIDI__ !== 'function')
        return false;

      globalThis.__receiveMIDI__(%);
      return true;
    })();
    )script";

    inline auto hydrateScript = R"script(
(function() {
  if (typeof globalThis.__receiveHydrationData__ !== 'function')
    return false;

  globalThis.__receiveHydrationData__(%);
  return true;
})();
)script";

    inline auto dispatchScript = R"script(
(function() {
  if (typeof globalThis.__receiveStateChange__ !== 'function')
    return false;

  globalThis.__receiveStateChange__(%);
  return true;
})();
)script";

    inline auto errorScript = R"script(
(function() {
  if (typeof globalThis.__receiveError__ !== 'function')
    return false;

  let e = new Error(%);
  e.name = @;

  globalThis.__receiveError__(e);
  return true;
})();
)script";

    inline auto logToViewScript = R"script(
(function() {
    if (typeof globalThis.__receiveLog__ !== 'function')
        return false;

    globalThis.__receiveLog__(%);
    return true;
    })();
)script";

inline auto receiveStateChangeScript =     R"script(
(function() {
  if (typeof globalThis.__receiveStateChange__ !== 'function')
    return false;

  globalThis.__receiveStateChange__(%);
  return true;
})();
)script";

    inline auto receiveTableContentChangeScript =     R"script(
(function() {
  if (typeof globalThis.__receiveTableContent__ !== 'function')
    return false;

  globalThis.__receiveTableContent__(%);
  return true;
})();
)script";

    inline auto vfsKeysScript = R"script(
(function() {
    if (typeof globalThis.__receiveVFSKeys__ !== 'function')
        return false;

    globalThis.__receiveVFSKeys__(%);
    return true;
    })();
)script";
} // namespace jsFunctions

#endif //PLUGINPROCESSOR_H
