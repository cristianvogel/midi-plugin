<script lang="ts">

    import {NativeMessage} from "../lib/NativeMessage.svelte";

    let midiMessage = $state("00 00 00");
    let log = $state("")

    const testArray = [
        "90 3C 50",  // C4 (Middle C) at velocity 80
        "90 3D 52",  // C#4 at velocity 82
        "90 3E 54",  // D4 at velocity 84
        "90 3F 56",  // D#4 at velocity 86
    ];

    function testOneByOne() {
        testArray.forEach(function (msg, index) {
            NativeMessage.sendMIDI([msg]);
            log += `${msg} sent.\n `
        });
    }

    function testAllAtOnce() {
        NativeMessage.sendMIDI(testArray);
        log = "Sent test array of " + testArray.length + " messages."
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