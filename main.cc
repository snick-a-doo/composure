// Copyright © 2020 Sam Varner
//
// This file is part of Composure.
//
// Composure is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// Composure is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with Composure.
// If not, see <http://www.gnu.org/licenses/>.

#include "composer.hh"
#include "random.hh"
#include "session.hh"

#include <cassert>
#include <iostream>
#include <string>

std::string csd_text = "<CsoundSynthesizer>\n"
    "<CsOptions>\n"
    "--midioutfile=compose.midi --nosound\n"
    "</CsOptions>\n"
    "<CsInstruments>\n"
    "instr 1\n"
    "  inote = p5\n"
    "  ivel = 101\n"
    "  midion 1, inote, ivel\n"
    "endin\n"
    "</CsInstruments>\n"
    "<CsScore>\n"
    "</CsScore>\n"
    "</CsoundSynthesizer>\n";


/// Make a new composition and write it to out.midi.
int main(int argc, char** argv)
{
    int voices = 6;
    int cycles = 8;
    int range = 24;
    int tempo = 60;
    int seed = -1;
    if (argc > 1)
        voices = std::stoi(argv[1]);
    if (argc > 2)
        cycles = std::stoi(argv[2]);
    if (argc > 3)
        range = std::stoi(argv[3]);
    if (argc > 4)
        tempo = std::stoi(argv[4]);
    if (argc > 5)
        seed = std::stoi(argv[5]);
    if (argc > 6)
        std::cerr << "Usage [voices cycles range tempo seed]" << argv[0] << std::endl;

    if (seed != -1)
        set_random_seed(seed);
    // Pick a random key from MIDI note 54 to 65: F# below middle C to F above.
    int tonic = pick(54, 65);
    // Start with an empty phrase and iterate.
    Phrase phrase(tempo);
    // phrase.set_notes(3, 1, {60, 59, 60}, 1);
    // phrase.set_notes(1.5, 1, {64, 63, 64}, 0.5);
    for (int i = 0; i < cycles; ++i)
        phrase = edit(compose(phrase, tonic, voices, range));

    phrase.write_midi("midi.midi");

    // Start a Csound and write the MIDI file.
    Session s(csd_text);
    s.set_score(phrase.score());
    sleep(2); // Give it time to finish.

    return 0;
}
