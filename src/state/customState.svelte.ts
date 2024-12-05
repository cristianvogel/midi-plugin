

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// This reactive function will handle updates to the host state
// which may be synth params or other stateful data
// like a selected chord progression or whatever.
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━
export const HostState = hostState({});
function hostState(initial: any) {
    let current = $state(initial);
    return {
        get current() {
            return current;
        },
        update(newValues: any) {
            current = newValues;
        },
        get snapshot() {
            return $state.snapshot(current);
        }
    };
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// This reactive function will handle new incoming MIDI
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━
let midiMessage: ArrayBuffer = new Uint8Array([0, 0, 0]);
export const IncomingRawMIDI = rawMIDI(midiMessage);
function rawMIDI(initial: ArrayBuffer) {
    let current = $state(initial);
    return {
        get current() {
            return current;
        },
        update(newValues: ArrayBuffer) {
            current = newValues;
        },
        get snapshot() {
            return $state.snapshot(current);
        }
    };
}