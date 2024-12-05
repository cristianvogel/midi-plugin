// global declarations
// noinspection ES6ConvertVarToLetConst

declare module 'midi-parser-js';

declare global {
    var __postNativeMessage__: (arg: string) => void;
    var __receiveMIDI__: (data: any) => void;
    var MidiParser: {
        parse: (data: Uint8Array, callback: (obj: any) => void) => void;
    };
}