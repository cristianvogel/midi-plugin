
import {hexStringToUint8Array} from "../utils/helpers";


// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// This reactive function will handle new incoming MIDI
// Can return a raw hex string, as received
// or parse it into Uint8 byte array
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━

export const IncomingMIDI = rawMIDI(["00 00 00"]);
function rawMIDI(initial: string[]) {
    let current = $state(initial);
    return {
        get current() {
            return current;
        },
        update(newValues: string[]) {
            current = newValues;
        },
        get snapshot() {
            return $state.snapshot(current);
        },
        get parsed() {
            return hexStringToUint8Array( current[0] )
        }

    };
}

// Better logging direct to UI
export const UIConsole = consoleText("");
function consoleText(initial: string) {
    let current = $state(initial);
    let extended = $state(initial);

    return {
        get current(): string {
            return current;
        },
        update(newValues: string) {
            // after 3 seconds , reset the console text
            setTimeout(() => {
                current = "";
            }, 3000);
            current = newValues;
        },
        get snapshot() {
            return $state.snapshot(current);
        },
        extend(newText: string) {
            setTimeout(() => {
                current = "";
            }, 2000);
            current = ( extended + newText );
        },
        get extended() {
            return extended;
        }
    };
}


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