<script lang="ts">
    import {onMount} from "svelte";
    import {RegisterMessagesFromHost} from "./lib/NativeMessage.svelte";
    import {IncomingMIDI} from "./state/customState.svelte";
    import MidiSendButton from "./Components/MidiSendButton.svelte";


    let midiMessageRaw = $derived(IncomingMIDI.current);


  onMount( ()=>
  {
      console.log( 'UI::Registering message handlers..')
      RegisterMessagesFromHost();
  })


</script>


<div>
    <h1>🪬 Hello Mindful MIDI ! </h1>
</div>
<div>
    <pre class="midi-message">{midiMessageRaw}</pre>
</div>
<div>
    <MidiSendButton />
</div>

<style>
    .midi-message {
            color: lawngreen;
            font-size: larger;
        }
</style>