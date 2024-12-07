// ReSharper disable CppTooWideScopeInitStatement
#include "PluginProcessor.h"
#include "WebViewEditor.h"

#include <choc_javascript_QuickJS.h>

#include "Helpers.h"


//==============================================================================
MindfulMIDI::MindfulMIDI()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
      , jsEngine(choc::javascript::createQuickJSContext())
{
    // Initialize parameters from the manifest file
#if ELEM_DEV_LOCALHOST
    auto manifestFile = juce::URL("http://localhost:5173/manifest.json");
    auto manifestFileContents = manifestFile.readEntireTextStream().toStdString();
#else
    auto manifestFile = util::getAssetsDirectory().getChildFile("manifest.json");

    if (!manifestFile.existsAsFile())
        return;

    auto manifestFileContents = manifestFile.loadFileAsString().toStdString();
#endif

    auto manifest = elem::js::parseJSON(manifestFileContents);

    if (!manifest.isObject())
        return;

    auto parameters = manifest.getWithDefault("parameters", elem::js::Array());

    for (const auto& descrip : parameters)
    {
        if (!descrip.isObject())
            continue;

        auto paramId = descrip.getWithDefault("paramId", elem::js::String("unknown"));
        auto name = descrip.getWithDefault("name", elem::js::String("Unknown"));
        auto minValue = descrip.getWithDefault("min", static_cast<elem::js::Number>(0));
        auto maxValue = descrip.getWithDefault("max", static_cast<elem::js::Number>(1));
        auto defValue = descrip.getWithDefault("defaultValue", static_cast<elem::js::Number>(0));

        auto* p = new juce::AudioParameterFloat(
            juce::ParameterID(paramId, 1),
            name,
            {static_cast<float>(minValue), static_cast<float>(maxValue)},
            static_cast<float>(defValue)
        );

        p->addListener(this);
        addParameter(p);

        // Push a new ParameterReadout onto the list to represent this parameter
        paramReadouts.emplace_back(ParameterReadout{static_cast<float>(defValue), false});

        // Update our state object with the default parameter value
        state.insert_or_assign(paramId, defValue);
    }
}

MindfulMIDI::~MindfulMIDI()
{
    for (auto& p : getParameters())
    {
        p->removeListener(this);
    }
}

//==============================================================================
juce::AudioProcessorEditor* MindfulMIDI::createEditor()
{
    editor = new WebViewEditor(this, util::getAssetsDirectory(), 800, 500);

    editor->setMidiOut = [this](const std::string& message, const int index)
    {
        handleMidiOut(message, index);
    };

    editor->ready = [this]()
    {
        dispatchStateChange();
    };

    // When setting a parameter value, we simply tell the host. This will in turn
    // fire a parameterValueChanged event, which will catch and propagate through
    // dispatching a state change event
    // When setting a parameter value, we simply tell the host. This will in turn fire
    // a parameterValueChanged event, which will catch and propagate through dispatching
    // a state change event
    editor->setParameterValue = [this](const std::string& paramId, float value)
    {
        if (parameterMap.contains(paramId))
        {
            parameterMap[paramId]->setValueNotifyingHost(value);
        }
    };

#if ELEM_DEV_LOCALHOST
    editor->reload = [this]()
    {
        initJavaScriptEngine();
        dispatchStateChange();
    };

#endif

    return editor;
}

bool MindfulMIDI::hasEditor() const
{
    return true;
}

//==============================================================================
const juce::String MindfulMIDI::getName() const
{
    return JucePlugin_Name;
}

bool MindfulMIDI::acceptsMidi() const
{
    return true;
}

bool MindfulMIDI::producesMidi() const
{
    return true;
}

bool MindfulMIDI::isMidiEffect() const
{
    return true;
}

double MindfulMIDI::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
int MindfulMIDI::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int MindfulMIDI::getCurrentProgram()
{
    return 0;
}

void MindfulMIDI::setCurrentProgram(int /* index */)
{
}

const juce::String MindfulMIDI::getProgramName(int /* index */) { return {}; }

void MindfulMIDI::changeProgramName(int /* index */, const juce::String& /* newName */)
{
}

//==============================================================================
void MindfulMIDI::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Some hosts call `prepareToPlay` on the real-time thread, some call it on the main thread.
    // To address the discrepancy, we check whether anything has changed since our last known
    // call. If it has, we flag for initialization of the Elementary engine and runtime, then
    // trigger an async update.
    //
    // JUCE will synchronously handle the async update if it understands
    // that we're already on the main thread.
    if (sampleRate != lastKnownSampleRate || samplesPerBlock != lastKnownBlockSize)
    {
        lastKnownSampleRate = sampleRate;
        lastKnownBlockSize = samplesPerBlock;

        shouldInitialize.store(true);
    }

    // Now that the environment is set up, push our current state
    triggerAsyncUpdate();
}

void MindfulMIDI::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool MindfulMIDI::isBusesLayoutSupported(const AudioProcessor::BusesLayout& layouts) const
{
    return true;
}

void MindfulMIDI::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;


    // Process all MIDI first
    auto now = MIDIClock::now();

    // Copy the input so that our input and output buffers are distinct
    // scratchBuffer.makeCopyOf(buffer, true);

    // Clear the output buffer to prevent any garbage if our runtime isn't ready
    // buffer.clear();

    // Process the elementary runtime
    // if (elementaryRuntime != nullptr && !runtimeSwapRequired)
    // {
    //     elementaryRuntime->process(
    //         const_cast<const float**>(scratchBuffer.getArrayOfWritePointers()),
    //         getTotalNumInputChannels(),
    //         const_cast<float**>(buffer.getArrayOfWritePointers()),
    //         buffer.getNumChannels(),
    //         buffer.getNumSamples(),
    //         nullptr
    //     );
    // }

    // not sure if this needs to be handled with a runtimeSwapRequired flag
    if (!midiMessages.isEmpty())
    {
        for (const auto metadata : midiMessages)
        {
            const auto bytes = metadata.getMessage().getRawData();
            const auto m = choc::midi::ShortMessage(bytes[0], bytes[1], bytes[2]);
            midi_in_fifo_queue.push({now, m});
        }
        triggerAsyncUpdate();
    }
    // We will re-assign to the MIDI buffer inside the process block,
    // as whatever is assigned at this point, will be sent as MIDI out
    // from the plug in
    midiMessages.clear();

    if (midi_out_fifo_queue.getUsedSlots() > 0)
    {
        OutgoingMIDIEvent m;
        while (midi_out_fifo_queue.pop(m))
        {
            juce::MidiMessage messageOut{m.message.data, m.message.length()};
            midiMessages.addEvent(messageOut, m.index);
        };
    }


    // Elementary needed a runtime swap
    if (runtimeSwapRequired)
    {
        shouldInitialize.store(true);
        triggerAsyncUpdate();
    }
}

void MindfulMIDI::handleMidiOut(const std::string& _msg, int index)
{
    // Split the string into three-two digit strings and parse from hex to unit8 bytes
    const juce::String msg(_msg);
    juce::StringArray bytes;
    bytes.addTokens(msg, " ", "\"");
    std::vector<uint8_t> uint8_bytes;
    for (auto& byte : bytes)
    {
        uint8_bytes.push_back(byte.getHexValue32());
    }
    // This shuold be a standard MIDI note on message
    if (uint8_bytes.size() == 3)
    {
        const choc::midi::ShortMessage messageOut(uint8_bytes[0], uint8_bytes[1], uint8_bytes[2]);
        if (midi_out_fifo_queue.push({messageOut, index}))
        {
            dispatchLogToUI("MIDI Out > [ " + std::to_string(uint8_bytes[0]) + ","
                + std::to_string(uint8_bytes[1]) + ","
                + std::to_string(uint8_bytes[2]) + " ]");
        }
    }
    else
    {
        dispatchLogToUI("MIDI Error: Message was not a 3 byte message.");
    }
}

void MindfulMIDI::parameterValueChanged(int parameterIndex, float newValue)
{
    // Mark the updated parameter value in the dirty list
    auto& pr = *std::next(paramReadouts.begin(), parameterIndex);
    pr.store({newValue, true});
    triggerAsyncUpdate();
}

void MindfulMIDI::parameterGestureChanged(int, bool)
{
    // Not implemented
}

//==============================================================================
void MindfulMIDI::handleAsyncUpdate()
{
    // First things first, we check the flag to identify if we should initialize the Elementary
    // runtime and engine.
    if (shouldInitialize.exchange(false))
    {
        elementaryRuntime = std::make_unique<elem::Runtime<float>>(lastKnownSampleRate, lastKnownBlockSize);
        initJavaScriptEngine();
        runtimeSwapRequired.store(false);
    }

    // Next we iterate over the current parameter values to update our local state
    // object, which we in turn dispatch into the JavaScript engine
    auto& params = getParameters();

    // Reduce over the changed parameters to resolve our updated processor state
    for (size_t i = 0; i < paramReadouts.size(); ++i)
    {
        // We atomically exchange an arbitrary value with a dirty flag false, because
        // we know that the next time we exchange, if the dirty flag is still false, the
        // value can be considered arbitrary. Only when we exchange and find the dirty flag
        // true do we consider the value as having been written by the processor since
        // we last looked.
        auto& current = *std::next(paramReadouts.begin(), i);
        auto pr = current.exchange({0.0f, false});

        if (pr.dirty)
        {
            if (auto* pf = dynamic_cast<juce::AudioParameterFloat*>(params[i]))
            {
                auto paramId = pf->paramID.toStdString();
                state.insert_or_assign(paramId, static_cast<elem::js::Number>(pr.value));
            }
        }
    }

    dispatchStateChange();
    dispatchMIDItoJS();
}

void MindfulMIDI::initJavaScriptEngine()
{
    jsEngine = choc::javascript::createQuickJSContext();

    // initialise the fifos for midi messages
    midi_in_fifo_queue.reset(100);
    midi_out_fifo_queue.reset(100);

    // Install some native interop functions in our JavaScript environment
    jsEngine.registerFunction(staticNames::NATIVE_MESSAGE_FUNCTION_NAME, [this](choc::javascript::ArgumentList args)
    {
        auto const batch = elem::js::parseJSON(args[0]->toString());
        auto const rc = elementaryRuntime->applyInstructions(batch);

        if (rc != elem::ReturnCode::Ok())
        {
            dispatchError("Runtime Error", elem::ReturnCode::describe(rc));
        }

        return choc::value::Value();
    });

    jsEngine.registerFunction(staticNames::LOG_FUNCTION_NAME, [this](choc::javascript::ArgumentList args)
    {
        // Forward logs to the editor if it's available; then logs show up in one place.
        //
        // If not available, we fall back to std out.
        if (auto* editor = dynamic_cast<WebViewEditor*>(getActiveEditor()))
        {
            const auto* kDispatchScript = R"script(
(function() {
  console.log(...JSON.parse(%));
  return true;
})();
)script";
            auto v = choc::value::createEmptyArray();

            for (size_t i = 0; i < args.numArgs; ++i)
            {
                v.addArrayElement(*args[i]);
            }

            auto expr = juce::String(kDispatchScript).replace("%", elem::js::serialize(choc::json::toString(v))).
                                                      toStdString();
            editor->getWebViewPtr()->evaluateJavascript(expr);
        }
        else
        {
            for (size_t i = 0; i < args.numArgs; ++i)
            {
                DBG(choc::json::toString(*args[i]));
            }
        }

        return choc::value::Value();
    });

    // A simple shim to write various console operations to our native __log__ handler
    jsEngine.evaluateExpression(R"shim(
(function() {
  if (typeof globalThis.console === 'undefined') {
    globalThis.console = {
      log(...args) {
        __log__('[embedded:log]', ...args);
      },
      warn(...args) {
          __log__('[embedded:warn]', ...args);
      },
      error(...args) {
          __log__('[embedded:error]', ...args);
      }
    };
  }
})();
    )shim");

    // Load and evaluate our Elementary js main file
#if ELEM_DEV_LOCALHOST
    auto dspEntryFile = juce::URL("http://localhost:5173/dsp.main.js");
    auto dspEntryFileContents = dspEntryFile.readEntireTextStream().toStdString();
#else
    auto dspEntryFile = util::getAssetsDirectory().getChildFile(staticNames::MAIN_DSP_JS_FILE);

    if (!dspEntryFile.existsAsFile())
        return;

    auto dspEntryFileContents = dspEntryFile.loadFileAsString().toStdString();
#endif
    jsEngine.evaluateExpression(dspEntryFileContents);

    // Re-hydrate from current state
    const auto* kHydrateScript = jsFunctions::hydrateScript;

    auto expr = juce::String(kHydrateScript).replace("%", elem::js::serialize(
                                                         elem::js::serialize(elementaryRuntime->snapshot())))
                                            .toStdString();
    jsEngine.evaluateExpression(expr);
}

void MindfulMIDI::dispatchStateChange()
{
    const auto* kDispatchScript = jsFunctions::receiveStateChangeScript;

    // Need the double serialize here to correctly form the string script. The first
    // serialize produces the payload we want, the second serialize ensures we can replace
    // the % character in the above block and produce a valid javascript expression.

    state.insert_or_assign(staticNames::SAMPLE_RATE, lastKnownSampleRate);

    const auto expr = juce::String(kDispatchScript).replace("%", elem::js::serialize(elem::js::serialize(state))).
                                                    toStdString();

    // First we try to dispatch to the UI if it's available
    if (const auto* editor = dynamic_cast<WebViewEditor*>(getActiveEditor()))
    {
        editor->getWebViewPtr()->evaluateJavascript(expr);
    }

    // Next we dispatch to the embedded engine which will evaluate JavaScript
    // here on the main thread
    jsEngine.evaluateExpression(expr);
}

void MindfulMIDI::dispatchTableContentStateChange()
{
    const auto* kDispatchScript = jsFunctions::receiveTableContentChangeScript;
    elem::js::Object wrappedTableContent;
    wrappedTableContent.insert_or_assign(staticNames::CHORD_PROGRESSION, tableContent);

    const auto expr = serialize(kDispatchScript, wrappedTableContent, "%");

    // First we try to dispatch to the UI if it's available
    if (const auto* editor = dynamic_cast<WebViewEditor*>(getActiveEditor()))
    {
        editor->getWebViewPtr()->evaluateJavascript(expr);
    }
    // Next we dispatch to the embedded engine which will evaluate JavaScript
    // here on the main thread
    jsEngine.evaluateExpression(expr);
}

//= Extended logging , so we can post debug messages directly in
//= the plugin UI.
void MindfulMIDI::dispatchLogToUI(const std::string& text) const
{
    const auto* kDispatchScript = jsFunctions::logToViewScript;
    if (const auto* editor = dynamic_cast<WebViewEditor*>(getActiveEditor()))
    {
        const auto wrappedText = choc::value::createString(text);
        const auto expr = serialize(kDispatchScript, wrappedText, "%");
        editor->getWebViewPtr()->evaluateJavascript(expr);
    }
}

//= MIDI out to WebView and jsContext
void MindfulMIDI::dispatchMIDItoJS()
{
    const uint32_t midiCount = midi_in_fifo_queue.getUsedSlots();

    elem::js::Array vec;

    if (midiCount > 0)
    {
        IncomingMIDIEvent m;
        while (midi_in_fifo_queue.pop(m))
        {
            vec.push_back(elem::js::Value(m.message.toHexString()));
        };
    }

    const auto serializedMidi = elem::js::serialize(vec);

    const auto* kDispatchScript = jsFunctions::receiveMidiScript;

    // Need the double serialize here to correctly form the string script. The first
    // serialize produces the payload we want, the second serialize ensures we can replace
    // the % character in the above script block and produce a valid javascript expression.

    const auto expr = juce::String(kDispatchScript).replace("%", elem::js::serialize(
                                                                elem::js::serialize(serializedMidi))).
                                                    toStdString();

    // First we try to dispatch to the UI if it's available, because running this step will
    // just involve placing a message in a queue.
    if (const auto* editor = dynamic_cast<WebViewEditor*>(getActiveEditor()))
    {
        editor->getWebViewPtr()->evaluateJavascript(expr);
    }

    // Next we dispatch to the local engine which will evaluate any necessary JavaScript synchronously
    // here on the main thread
    jsEngine.evaluateExpression(expr);
}


void MindfulMIDI::dispatchError(std::string const& name, std::string const& message)
{
    const auto* kDispatchScript = jsFunctions::errorScript;

    // Need the serialize here to correctly form the string script.
    const auto expr = juce::String(kDispatchScript).replace("@", elem::js::serialize(name)).replace(
        "%", elem::js::serialize(message)).toStdString();

    // First we try to dispatch to the UI if it's available, because running this step will
    // just involve placing a message in a queue.
    if (auto* editor = dynamic_cast<WebViewEditor*>(getActiveEditor()))
    {
        editor->getWebViewPtr()->evaluateJavascript(expr);
    }

    // Next we dispatch to the local engine which will evaluate any necessary JavaScript synchronously
    // here on the main thread
    jsEngine.evaluateExpression(expr);
}

/*▮▮js▮▮▮▮▮▮frontend▮▮▮▮▮▮backend▮▮▮▮▮▮messaging▮▮▮▮▮▮
 * @name serialize
 * @brief Serialize data for js
 */
std::string MindfulMIDI::serialize(const std::string& function, const elem::js::Object& data,
                                   const juce::String& replacementChar)
{
    return juce::String(function)
           .replace(replacementChar, elem::js::serialize(elem::js::serialize(data)))
           .toStdString();
}

std::string MindfulMIDI::serialize(const std::string& function, const choc::value::Value& data,
                                   const juce::String& replacementChar)
{
    return juce::String(function).replace(replacementChar, choc::json::toString(data)).toStdString();
}

//==============================================================================
void MindfulMIDI::getStateInformation(juce::MemoryBlock& destData)
{
    auto serialized = elem::js::serialize(state);
    destData.replaceAll((void*)serialized.c_str(), serialized.size());
}

void MindfulMIDI::setStateInformation(const void* data, int sizeInBytes)
{
    try
    {
        auto str = std::string(static_cast<const char*>(data), sizeInBytes);
        auto parsed = elem::js::parseJSON(str);
        auto o = parsed.getObject();
        for (auto& i : o)
        {
            std::map<std::string, elem::js::Value>::iterator it;
            it = state.find(i.first);
            if (it != state.end())
            {
                state.insert_or_assign(i.first, i.second);
            }
        }
    }
    catch (...)
    {
        // Failed to parse the incoming state, or the state we did parse was not actually
        // an object type. How you handle it is up to you, here we just ignore it
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MindfulMIDI();
}
