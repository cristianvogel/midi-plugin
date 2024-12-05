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