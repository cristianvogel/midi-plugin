<script lang="ts">

    import {NativeMessage} from "../lib/NativeMessage.svelte";
    import {generateMIDIMessage} from "../utils/helpers";

    let midiMessage = $state("00 00 00");
    let log = $state("")

    const testChord = [
        "90 3C 50",
        "90 3F 52",
        "90 3A 54",
        "90 2F 56",
    ];

    function testOneByOne() {
        let randomNote: string = generateMIDIMessage();
        NativeMessage.sendMIDI( [ randomNote ] )
        log += `${randomNote} sent.\n `
    }

    function testAllAtOnce() {
        NativeMessage.sendMIDI(testChord);
        log = "Sent test array of " + testChord.length + " messages.\n"
    }

</script>
<div>
    <button onclick={testOneByOne}>
        One by One Test
    </button>

    <button onclick={testAllAtOnce}>
        All at Once Test
    </button>

    <pre class="log">
    {log}
</pre>
</div>
<style>
    .log {
        padding: 15px;
        background-color: darkslategray;
        border: 1px solid #ddd;
        border-radius: 5px;
        overflow-y: auto;
        width: 20%;
        height: 100px; /* Adjust as necessary */
        font-family: 'Courier New', Courier, monospace;
        font-size: x-small;
        white-space: pre-line; /* Keeps the formatted text look (like in `<pre>`), but wraps text */
    }
</style>