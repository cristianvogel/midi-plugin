import {Renderer, el} from '@elemaudio/core';
import {RefMap} from './RefMap';
import synth from "./synth.js";

// First, we initialize a custom Renderer instance that marshals our instruction
// batches through the __postNativeMessage__ function to direct the underlying native
// engine.
let core = new Renderer((batch) => {
    __postNativeMessage__(JSON.stringify(batch));
});

// Next, a RefMap for coordinating our refs
let refs = new RefMap(core);

// Holding onto the previous state allows us a quick way to differentiate
// when we need to fully re-render versus when we can just update refs
let prevState = null;

function shouldRender(prevState, nextState) {
    return (prevState === null) || (prevState.sampleRate !== nextState.sampleRate);
}

// Here we register a state change callback with the native
// side. This callback will be hit with the current processor state any time that
// state changes.
//
// Given the new state, we simply update our refs or perform a full render depending
// on the result of our `shouldRender` check.
globalThis.__receiveStateChange__ = (serializedState) => {
    const state = JSON.parse(serializedState);
    if (shouldRender(prevState, state)) {
        // ref definitions for synth voice will go here
        let stats = core.render(...synth({
            key: 'synth',
            sampleRate: state.sampleRate,
        }, el.in({channel: 0}), el.in({channel: 1})));

        console.log(stats);
    } else {
        console.log('Updating refs');
        // refs update for synth voice will go here
    }
    prevState = state;
};

////////////////////////////////////////////////////////////////////
// MIDI events as serialised three byte messages come in to the headless
// JSContext (here) from the native side. What gets received in this context will
// function for MIDI or audio param updates even when the WebView is closed or not available.
//
// We also receive the same callback data in the WebView which should be used for UI updates.
//
// In each context, the messages will need to be parsed
// using the standard MIDI spec before being used.
//
// TODO:
// Define another global function to perform the parsing in one place
//
globalThis.__receiveMIDI__ = (data) => {
    console.log('QUICKJS::received raw MIDI message', JSON.parse(data))
}

//---------------------------------------------------------------------
////SRVB Detail//////////////////////////////////////////////////////
// NOTE: Experimental ◬
// This hook allows the native side to inject serialized graph state from
// the running elem::Runtime instance so that we can throw away and reinitialize
// the JavaScript engine and then inject necessary state for coordinating with
// the underlying engine.
globalThis.__receiveHydrationData__ = (data) => {
    const payload = JSON.parse(data);
    const nodeMap = core._delegate.nodeMap;

    for (let [k, v] of Object.entries(payload)) {
        nodeMap.set(parseInt(k, 16), {
            symbol: '__ELEM_NODE__',
            kind: '__HYDRATED__',
            hash: parseInt(k, 16),
            props: v,
            generation: {
                current: 0,
            },
        });
    }
};

// Finally, an error callback which just logs back to native
globalThis.__receiveError__ = (err) => {
    console.log(`[Error: ${err.name}] ${err.message}`);
};
