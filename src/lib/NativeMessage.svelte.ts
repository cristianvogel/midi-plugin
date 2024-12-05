import { IncomingRawMIDI } from "../state/customState.svelte"
import {data} from "autoprefixer";


export declare var globalThis: any;



function processHostState(state: any) {
  let parsedEntries: { [key: string]: number } = {};

  try {
    parsedEntries = JSON.parse(state);
  } catch (e) {
    console.warn("Bad state received", parsedEntries);
  }

}

export const MessageToHost = {
  updateHost: function (paramId: string, value: number) {
    if (typeof globalThis.__postNativeMessage__ === "function") {
      globalThis.__postNativeMessage__("setParameterValue", {
        paramId,
        value
      });
    }
  },

  /** ━━━━━━━
   * Send a ready message to the host.
   */
  requestReady: function () {
    if (typeof globalThis.__postNativeMessage__ === "function") {
      globalThis.__postNativeMessage__("ready", {});
    }
  },


  /** ━━━━━━━
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

/* ━━━━━━━━━━━━━━
 * Registers the message handlers for receiving and processing messages from the host.
 * This function sets up global functions that handle state changes, mesh state changes,
 * license activation, error handling, and host information.
 * ━━━━━━━━━━━━━━
 */

export function RegisterMessagesFromHost() {
  /* ━━━━━━━━━━━━━━
   * Handles the state change received from the host.
   * @param state - The host state change object.
   * ━━━━━━━━━━━━━━
   */
  globalThis.__receiveStateChange__ = function (state: any) {
    processHostState(state);
  };

    /* ━━━━━━━━━━━━━━
     * Handles incoming MIDI data
     * ━━━━━━━━━━━━━━
     */
    globalThis.__receiveMIDI__ =  function ( data: any) {
        let parsedMessage: ArrayBuffer ;
        try {
            parsedMessage = JSON.parse(data);
        } catch {
            console.warn("UI::Error parsing MIDI data -> ", data);
            parsedMessage = new Uint8Array([0,0,0])
        }
        IncomingRawMIDI.update( parsedMessage )
    };

  /** ━━━━━━━
   * DEV: Handles an error received from the host.
   * @param error - The error object.
   */
  globalThis.__receiveError__ = function (error: any) {
    //ConsoleText.set("Error: " + error);
    console.log("PLUGIN:: " + error);
  };

  globalThis.__log__ = function (log: any) {
    //ConsoleText.set("Error: " + error);
    console.warn("PLUGIN:: ", log);
  };
}
