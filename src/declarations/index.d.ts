// type declarations




i

interface ChordNotes {
    noteNumbers: number[];
}

export interface TableContent {
    tableContent: {
        noteNumbers: ChordNotes
        chordProgression: ChordNotes[]
    }
}

