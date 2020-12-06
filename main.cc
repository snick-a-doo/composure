#include "composer.hh"
#include "random.hh"
#include "session.hh"

#include <iostream>
#include <string>

std::string csd_text = "<CsoundSynthesizer>\n"
    "<CsOptions>\n"
    "--midioutfile=out.midi --nosound\n"
    "</CsOptions>\n"
    "<CsInstruments>\n"
    "instr 1\n"
    "  inote = p5\n"
    "  ivel = 127\n"
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
    int cycles = 4;
    int range = 24;
    int tempo = 60;
    if (argc > 1)
        voices = std::stoi(argv[1]);
    if (argc > 2)
        cycles = std::stoi(argv[2]);
    if (argc > 3)
        range = std::stoi(argv[3]);
    if (argc > 4)
        tempo = std::stoi(argv[4]);
    if (argc > 5)
    {
        std::cerr << "Usage " << argv[0] << std::endl;
        exit(1);
    }

    // Pick a random key from MIDI note 54 to 65: F# below middle C to F above.
    int tonic = pick(54, 65);
    // Start with an empty phrase and iterate.
    Phrase phrase;
    for (int i = 0; i < cycles; ++i)
        phrase = edit(compose(phrase, tonic, voices, range));
    std::cout << "Time: " << phrase.end_time() << "s\n";

    // Start a Csound and write the MIDI file.
    Session s(csd_text);
    s.set_score(phrase.score(tempo));
    sleep(2); // Give it time to finish.

    return 0;
}
