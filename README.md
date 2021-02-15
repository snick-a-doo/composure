# Composure
A naive music composition algorithm

# Usage
    composure [options]
        -c --chromatic  (false)
        -h --help       Display this help and exit.
        -k --key=       60 for middle C (random 54 to 65)
        -m --monophonic (false)
        -o --output=    Output file name (composure)
        -p --passes=    Number of compose/edit passes (8)
        -r --range=     Maximum range of notes (24)
        -s --seed=      Random seed (random)
        -t --tempo=     Beats per minute (60)
        -v --voices=    Number of voices (6)

All parameters are optional. Defaults are in parnetheses. Key is specified by MIDI note number. If unspecified, a random key is chosen from F# below to F above middle C. If a seed is not specified, the random number generator is seeded with std::random_device to give unpredictable output.

Three output files are produced:
* <filename>.midi
* <filename>.log
* <filename>.notes

where <filename> is the name passed to the -o or --output= option, or the default, "composure". The .midi file can be imported into editors like [MuseScore](https://musescore.org) or played by players like [TiMidity++](http://timidity.sourceforge.net). Example usage:

    composure -v12 -t120
    timidity composure.midi --force-program=12

The log file shows the parameters used used to generate the output, including the seed. This allows the output to be recreated from the log. The .notes file contains space-separated data for each note: start time, stop time, pitch, and generation, i.e. which pass the note was generated on. The file scripts/plot-notes.r contains the function plot.notes written in the language [R](https://www.r-project.org/) that produces something like

![Example output of the plot.notes function](example/flapple.png)

The color shows the generation.

The monophonic option prevents overlapping notes, which is good for sustained patches.
Otherwise, a decaying patch, like piano or marimba, is recommended. Notes are all in the key unless the chromatic option is given.

# Algorithm
Pieces are constructed by repeated compose/edit passes. In the "compose" part, notes are generated and appended to any existing notes. Initially, all voices are set to tonic. We loop over the voices and add notes one by one. Each time through the loop, a "dissonance" function is applied to each voice's note. A voice is then chosen at random, weighted by dissonance. The chosen voice's note is then changed randomly by -2, -1, 0, 1, or 2 scale degrees. A second voice may be chosen for shifting up or down by one degree, weighted by how long it's been since it changed.

Each note starts 1, 1/2, or 1/4 beat after the previous note. The shorter durations are weighted more heavily when the range of notes in the voices is wide. To make things rhythmically interesting, each voice's note is then shifted backward in time by n*d*(v+1), where n is the index of the voice, d is the delay from the previous note, v is the number of voices. The result is that notes heard near each other were generated in different loops over the voices. Note generation stops when the range of notes exceeds a threshold (the *range* command line parameter).

The "edit" part of the pass takes chunks of the notes generated so far and sticks them together. The start of each chunk is determined by a "point of interest" function. This function takes a running "consonance" measure and records each point where it reaches its first peak after crossing the midpoint from below.

When the compose/edit sequence is iterated, "compose" takes the last note for each voice from the output of "edit" as its starting point and appends new material. Then "edit" splices together the interesting bits from the whole piece so far.

There's very little music theory or traditional rules of composition baked in. Sometimes the output is tedious, but often it makes me happy. And if one composition doesn't hit the spot, you can always generate another.

# Example
The file example/flapple.midi is produced by

    composure -t 70 -s 2564151080 -o flapple

Too tonal? Try example/spilt.midi. For solo violin, try example/monophone.midi.

# Building

    mkdir build
    cd build
    meson setup
    ninja (or build with another back-end)
