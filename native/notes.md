# MIDI Message Storage with choc library

To store the MIDI messages, I'm sing first-in-first-out queue from choc library, in my `PluginProcessor.h`
```c++
using MIDIClock = std::chrono::high_resolution_clock;
struct IncomingMIDIEvent
{
    MIDIClock::time_point time;
    choc::midi::ShortMessage message;
};
choc::fifo::SingleReaderSingleWriterFIFO<IncomingMIDIEvent> midiFifoQueue;
each event has a time and a message
in PluginProcessor.cpp I'm syncing the midi messages using distpatchStateChange to the web context (that's where elementary DSP code lives + UI all in one)
void UPPluginProcessor::dispatchStateChange()
{
    const uint32_t midiCount = midiFifoQueue.getUsedSlots();
    elem::js::Array vec;
    if(midiCount > 0) {
        IncomingMIDIEvent m;
        while(midiFifoQueue.pop(m)) {
            vec.push_back(elem::js::Value(m.message.toHexString()));
        };
    }
    try {
        auto serializedState = elem::js::serialize(state);
        auto serializedMidi = elem::js::serialize(vec);
        uph::WebContext *wc = getWebContext();
        if(wc != nullptr) {
            wc->syncState(serializedState, serializedMidi);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "UPPluginProcessor::dispatchStateChange Catch:" << '\n';
        std::cerr << e.what() << '\n';
    }
}
```
And this is how the midi messages arrive to the plugin from VST wrapper
```c++
void UPPluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

    juce::ScopedNoDenormals noDenormals;
    auto now = MIDIClock::now();

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    scratchBuffer.makeCopyOf(buffer, true);
    scratchBuffer.setSize(numChannels + 4, numSamples, true, true, true);
    buffer.clear();

    // Store midi messages
    if(!midiMessages.isEmpty()) {
        for (const auto metadata : midiMessages) {
            auto bytes = metadata.getMessage().getRawData();
            auto m = choc::midi::ShortMessage (bytes[0], bytes[1], bytes[2]);
            midiFifoQueue.push({ now, m });
        }
        triggerAsyncUpdate();
    }

    //... other code

}
```
on the receiving end in JS, I expose a function to receive messages and I deconstruct the midi messages based on the midi standard