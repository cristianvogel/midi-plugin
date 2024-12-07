<script lang="ts">
    import {onMount} from "svelte";
    import {RegisterMessagesFromHost} from "./lib/NativeMessage.svelte";
    import {IncomingMIDI, UIConsole} from "./state/customState.svelte";
    import MidiSendButtons from "./Components/MidiSendButtons.svelte";
    import {fade} from "svelte/transition";

    let midiMessageRaw = $derived(IncomingMIDI.current);
    let log = $derived(UIConsole.current);

    let newMessage = $state(false);

    onMount(() => {
        RegisterMessagesFromHost();
    })

    $effect(() => {
        if (IncomingMIDI.snapshot.length > 0 ) {
            newMessage = true;
            setTimeout(() => {
                newMessage = false;
            }, 1000);
        }
    })

</script>


<div>
    <h1>🪬 Hello Mindful MIDI ! </h1>
    <div>
        <MidiSendButtons/>
    </div>
</div>
{#if newMessage}
    <div transition:fade style="position: absolute; left: 300px">
        <div class="message-rcv-console">MIDI message received</div>
        <pre class="midi-message">{midiMessageRaw}</pre>
    </div>
{/if}


    <div class="log">
        Message from JSContext: {@html log }
    </div>

<style>
    .midi-message {
        color: lawngreen;
        font-size: x-large;
    }


    .message-rcv-console {
        font-family: 'Courier New', Courier, monospace;
        font-size: small;
        white-space: pre-line;
    }

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