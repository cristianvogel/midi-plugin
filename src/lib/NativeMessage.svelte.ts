/*  NativeMessage.ts 
 *  This code sets up the handlers for receiving messages and sending
 *  messages between the UI layer and the backend native code.
 *
 *  These observers will be destroyed when the UI is closed.
 * 
 */

import {IncomingMIDI, UIConsole} from "../state/customState.svelte"
import {isValidMidiHex} from "../utils/helpers";
export declare var globalThis: any;

// ** processHostState
// here we will be processing host state changes
// emitted by the backend , which are relevant
// to changes in the UI
//
function processHostState(state: any) {
    let parsedEntries: { [key: string]: number } = {};

    try {
        parsedEntries = JSON.parse(state);
    } catch (e) {
        console.warn("Bad state received", parsedEntries);
    }

}

/*  RegisterMessagesFromHost 
 * Register the message handlers for receiving and processing messages from the host.
 * This function sets up global functions that handle state changes, mesh state changes,
 * license activation, error handling, and host information.
 * 
 */

export function RegisterMessagesFromHost() {
    /* 
     * Handles the state change received from the host.
     * @param state - The host state change object.
     */
    globalThis.__receiveStateChange__ = function (state: any) {
        processHostState(state);
    };

    /* 
     * Handles incoming MIDI data
     */
    globalThis.__receiveMIDI__ = function (data: any) {
        let hexBytes: string[];
        try {
            hexBytes = JSON.parse(data);
            IncomingMIDI.update(hexBytes)
        } catch {
            console.warn("UI::Error receiving MIDI data -> ", data);
        }
    };

    /* 
     * DEV ONLY: Handles logging and errors
     */
    globalThis.__receiveError__ = function (error: any) {
        //ConsoleText.set("Error: " + error);
        console.log("PLUGIN:: " + error);
    };

    globalThis.__log__ = function (log: any) {
        //ConsoleText.set("Error: " + error);
        console.warn("PLUGIN:: ", log);
    };

    globalThis.__receiveLog__ = function (text: string) {
        UIConsole.extend( text )
    }
}



/*  NativeMessage 
* Register messages to the back end from the UI, such as the 'requestReady' init call
* and of course a means of updating host params from the UI via updateHost.
* 
*/
export const NativeMessage = {

    /** 
     * Update a paramID to a new value in the host
     */
    setParameterValue: function (paramId: string, value: number) {
        if (typeof globalThis.__postNativeMessage__ === "function") {
            globalThis.__postNativeMessage__("setParameterValue", {
                paramId,
                value
            });
        }
    },
    /** 
     * Send MIDI three byte messages from the plugin
     * @param messages eg: [ "90 3C 64", "80 4b 7f" ... ]
     * Posts all the messages passed by the array and an index
     * to keep track of an order of events
     */
    sendMIDI: function ( messages: Array<string> ) {
        if (typeof globalThis.__postNativeMessage__ === "function") {
            let index = 0;
            for (let message of messages) {
                if (isValidMidiHex(message)) {
                    globalThis.__postNativeMessage__("sendMIDI", {
                        message,
                        index
                    });
                    index++;
                }
            }
        }
    },

    /** 
     * Send a ready message to the host.
     */
    requestReady: function () {
        if (typeof globalThis.__postNativeMessage__ === "function") {
            globalThis.__postNativeMessage__("ready", {});
        }
    },


    /** 
     * Hot reload the DSP during development.
     */
    __bindHotReload: function () {
        if (process.env.NODE_ENV !== "production") {
            //@ts-ignore
            import.meta.hot?.on("reload-dsp", () => {
                // console.log("Sending reload dsp message");

                if (typeof globalThis.__postNativeMessage__ === "function") {
                    globalThis.__postNativeMessage__("reload");
                }
            });
        }
    },


};
