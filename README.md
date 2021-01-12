# Composure
A naive music composition algorithm

Fork it on GitHub: https://github.com/snick-a-doo/composure

# Usage
    composure [options]
        -o --output=    Output file name (composure.midi)
        -v --voices=    Number of voices (6)
        -p --passes=    Number of compose/edit passes (10)
        -r --range=     Maximum range of notes (24)
        -t --tempo=     Beats per minute (60)
        -k --key=       60 for middle C (-1 random)
        -s --seed=      Random seed (-1 random)
        -m --monophonic (false)
        -c --chromatic  (false)

All parameters are optional.  Key is specified by MIDI note number.  A key of -1 gives a random key from F# below to F above middle C.  A seed of -1 means to seed the random number generator with std::random_device to get unpredictable output.  Example:

    compose -v4 -p4 -r12
    timidity composure.midi --force-program=12

The monophonic options prevents overlapping notes, which is good for sustained patches.
Otherwise, a decaying patch, like piano or marimba, is a good choice.  Notes are all in the key unless the chromatic option is given.

# Algorithm
Pieces are constructed by an iterated compose/edit pass.  In the "compose" part, notes are generated and appended to any existing notes.  Initially, all voices are set to tonic.  We loop over the voices and add notes one by one.  Each time through the loop, a  "dissonance" function is applied to each voice's note.  A voice is then chosen at random, weighted by dissonance.  The chosen voice's note is then changed randomly by -2, -1, 0, 1, or 2 scale degrees.  A second voice is chosen weighted by how long it's been since it changed.

A duration of 1, 1/2, or 1/4 beat is assigned to each note.  The shorter durations are weighted more heavily when the range of notes in the voices is wide.  To make things more rhythmically interesting, each voice's note is shifted backward in time by n*d*(v+1)/v, where n is the index of the voice, d is the duration, v is the number of voices.  Notes heard near each other were generated in different loops over the voices.  Note generation stops when the range of notes exceeds a threshold (the *range* command line parameter).

The "edit" part of the pass takes chunks of the notes generated so far and sticks them together.  The start of each chunk is determined by a "point of interest" function.  This function takes a running "consonance" measure and records where it goes from below its midpoint value to above.  This tends to provide some repetition with subtle variation and occasionally some more substantial changes.

When the compose/edit sequence is iterated, "compose" takes the last note for each voice from the output of "edit" as its starting point and appends new material.  Then "edit" splices together the interesting bits from the whole piece so far.

There's very little music theory or traditional rules of composition baked in.  Sometimes the output is tedious, but often it makes me happy.  And if one composition doesn't hit the spot, you can always generate another.

# Example
The file composed.midi was produced by

    compose 8 15 24

It takes a minute to get rolling, and maybe it go on for too long between major changes, but really enjoyed that one.  It sounds great on marimba.

# Building

    mkdir build
    meson setup
    meson compile -C build
