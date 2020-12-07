# Composer
A niave music compoliton algorithm.

# Usage
    compose voices cycles range tempo

All parameters are optional.  Default values are 6, 8, 24, 60.  Example:

    compose 4 4 12
    timidity compose.midi --force-program=12

All notes are sustained, so a decaying patch, like piano or marimba is a good choice.

# Algorithm
Pieces are constructed by an iterated compose/edit cycle.  In the "compose" part notes are generated and appended to any existing notes.  Initially, all voices are set to tonic.  We loop over the voices and add the notes one by one.  Each time through the loop, a  "dissonance" function is applied to each voice's note.  A voice is then chosen an random, weighted by dissonance.  The chosen voice's note is then changed randomly by -2, -1, 0, 1, or 2 scale degrees.  A second voice is chosen weighted by how long it's been since it changed.

A duration of 1, 1/2, or 1/4 beat is assigned to each note.  The shorter durations are weighted more heavily when the range of notes in the voices is wide.  To make things more interesting, each voice's note is shifted backward in time.  Notes heard near each other were generated in different loops over the voices.  Note generation stops when the range of notes exceeds a threshold (the *range* command line parameter).

The "edit" part takes chunks of the notes generated so far and sticks them together.  The start of each chunk is determined by a "point of interest" function.  This function takes a running "consonance" measure and records where it goes from below its midpoint value to above.  This tends to provide some repetition with subtle variation and some more substantial changes.

When the compose/edit sequence is iterated, "compose" takes the last note for each voice from the output of "edit" as its starting point and appends new material.  The "edit" splices together the interesting bits from the whole piece so far.

There's very little music theory or traditional rules of composition baked in.  Sometimes the output is tedious, but often it makes me happy.  And if one composition doesn't hit the spot, you can always generate another.

# Building
Meson looks for a pkg-config file for Csound.  There's one in the project root in case
there's not one on the system.

    mkdir build
    meson setup --pkg-config-path=. build
