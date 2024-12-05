<script lang="ts">
    import {onMount} from "svelte";
    import {RegisterMessagesFromHost} from "./lib/NativeMessage.svelte";
    import {IncomingMIDI} from "./state/customState.svelte";


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

<style>
    .midi-message {
            color: lawngreen;
            font-size: larger;
        }
</style>