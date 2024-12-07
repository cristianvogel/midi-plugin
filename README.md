[NativeMessage.svelte.ts](src%2Flib%2FNativeMessage.svelte.ts)

is where the View establishes its communication scripts with the underlying embedded QuickJS engine

[main.js](dsp%2Fmain.js)

is running in the QuickJS context, so it can define its own globalThis and send and receive that way

[PluginProcessor.cpp](native%2FPluginProcessor.cpp)
On the back end, a series of dispatchers in the processor are emitting to both the View and the QuickJS context