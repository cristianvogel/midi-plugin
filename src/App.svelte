<script lang="ts">
    import {onMount} from "svelte";
    import MidiParser from 'midi-parser-js'
    import {RegisterMessagesFromHost} from "./lib/NativeMessage.svelte";
    import {IncomingRawMIDI} from "./state/customState.svelte";

    let midiMessageRaw = $derived( IncomingRawMIDI.current )

  onMount( ()=>
  {
      console.log( 'UI::Registering message handlers..')
      RegisterMessagesFromHost();
  })

   $effect( ()=> {
            MidiParser.parse(new Uint8Array(IncomingRawMIDI.current), function (obj: any) {
                console.log('parsed MIDI message', obj);
            });
        }
    );

</script>


<div>
    <h1>🪬 Hello Mindful MIDI ! </h1>
</div>
<div>
    <pre>{midiMessageRaw}</pre>
</div>