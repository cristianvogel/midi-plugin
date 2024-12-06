// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// keeping helper functions in one place
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


/*  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Converts an array of hexadecimal strings to an array of
 * Uint8Array containing the corresponding byte values.
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 */
export function hexStringToUint8Array(hexString: string) {
    const bytesString = hexString.split(' ');
    const bytesNumber = bytesString.map(byte => parseInt(byte, 16));
    return new Uint8Array(bytesNumber);
}

/*  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 *  Checks if a given string has a valid MIDI message hex
 *  structure like "90 3C 64" (three pairs of hexadecimal
 *  digits separated by spaces):
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 */
export function isValidMidiHex(hexString: string): boolean {
    // Pattern for three pairs of hex digits separated by spaces
    const pattern = /^([0-9A-Fa-f]{2}\s){2}[0-9A-Fa-f]{2}$/;
    return pattern.test(hexString);
}

/*  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 *  generate some random midi notes
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 */
export function generateMIDIMessage(): string {
    // Generate a random MIDI note between 21 (A0) and 108 (C8) inclusive
    const note = Math.floor(Math.random() * (70 - 40 + 1)) + 40;

    // Generate a random velocity between 1 (min velocity) and 127 (max velocity) inclusive
    const velocity = 60;

    // Convert to hexadecimal and format the message
    return `90 ${note.toString(16).toUpperCase()} ${velocity.toString(16).toUpperCase()}`;
}