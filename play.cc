#include "composer.hh"
#include "random.hh"
#include "session.hh"

#include <iostream>

Phrase edit(Phrase phrase)
{
    std::vector<Phrase> parts = filter(phrase);
    Phrase edited;
    for (auto p : parts)
    {
        auto dt = edited.end_time - p.notes.front().time;
        p = time_shift(p, dt);
        edited.end_time = p.end_time;
        for (const auto& n : p.notes)
            edited.notes.push_back(n);
    }
    return edited;
}

/// Make a new composition and write it to out.midi.
/// Usage: play CFD_FILE
int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage " << argv[0] << " CFD_FILE" << std::endl;
        exit(1);
    }

    // Pick a random key from MIDI note 54 to 65: F# below middle C to F above.
    int tonic = pick(54, 65);
    // Start with an empty phrase and iterate.
    Phrase phrase;
    for (int i = 0; i < 10; ++i)
        phrase = edit(walk(phrase, tonic, 6, 36));

    // Start a Csound and write the MIDI file.
    Session s(argv[1]);
    s.set_score(score(phrase));
    sleep(2); // Give it time to finish.

    return 0;
}
