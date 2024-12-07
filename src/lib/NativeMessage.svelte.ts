/*  NativeMessage.ts 
 *  This code sets up the handlers for receiving messages and sending
 *  messages between the UI layer and the backend native code.
 *
 *  These observers will be destroyed when the UI is closed.
 * 
 */

import {IncomingMIDI, UIConsole} from "../state/customState.svelte"
import {isValidMidiHex} from "../utils/helpers";
import {TableContent} from "../declarations";

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

    // MindfulHarmony ////////////////////////////////////////////////
    globalThis.__receiveTableContent__ = (data: any) =>
    {
        let parsedData: TableContent = JSON.parse(data);

        UIConsole.extend( theLog( JSON.stringify(parsedData) as string ) )
    }

    /* 
     * Handles incoming MIDI data
     */
    globalThis.__receiveMIDI__ = function (data: any) {
        let hexBytes: string[];
        try {
            hexBytes = JSON.parse(data);
            IncomingMIDI.update(hexBytes)
        } catch {
            UIConsole.update("Error receiving MIDI data -> " + JSON.parse(data));
        }
    };

    /* 
     * DEV ONLY: Handles logging and errors
     * via the WebView browser tools ( press right mouse on View to
     * get access to the console via 'Inspect Element'
     */
    globalThis.__receiveError__ = function (error: any) {
        //ConsoleText.set("Error: " + error);
        UIConsole.update("ERROR_FROM_PLUGIN:: " + error);
        console.log()
    };

    globalThis.__log__ = function (log: any) {
        console.warn("ELEM:: ", log);
    };

    globalThis.__receiveLog__ = function (text: string) {
        console.log( "TO_VIEW_FROM_PLUGIN::" + text )
    }
}



/*  NativeMessage 
* Register messages to the back end from the UI, such as the 'requestReady' init call
* and of course a means of updating host params from the UI via updateHost.
* 
*/
export const NativeMessage = {

    /**
     * Reset the tableContent persistent store
     */
    resetTableContent: function (){
        if (typeof globalThis.__postNativeMessage__ === "function") {
            globalThis.__postNativeMessage__("resetTableContent")
            UIConsole.reset();
        }
    },

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


 function theLog( text: string ): string
{
        let customLog: string = "";
        let html = "<code style=\"display:block;overflow:hidden;margin-top:3px;border-bottom:1px solid #000;padding:3px;\">";
        customLog += html + text + "</code>";
return customLog;
}

