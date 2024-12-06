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
        UIConsole.update('UI::Registered message handlers..');
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
    <div transition:fade>
        <div class="message-rcv-console">MIDI message received</div>
        <pre class="midi-message">{midiMessageRaw}</pre>
    </div>
{/if}

{#if log.length}
    <div class="ui-console">
        {@html log }
    </div>
{/if}
<style>
    .midi-message {
        color: lawngreen;
        font-size: larger;
    }

    .ui-console {
        font-family: 'Courier New', Courier, monospace;
        font-size: x-small;
        white-space: pre-line;
        position: absolute;
        top: 50%;
        left: 35%;
    }

    .message-rcv-console {
        font-family: 'Courier New', Courier, monospace;
        font-size: small;
        white-space: pre-line;
    }
</style>