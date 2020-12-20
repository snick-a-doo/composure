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

#include <compose.hh>
#include <random.hh>

#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <getopt.h>
#include <optional>
#include <random>

std::string size_and_time(const std::vector<Note>& notes)
{
    auto sec = static_cast<int>(notes.back().time + notes.back().duration);
    std::ostringstream os;
    os << std::setw(4) << notes.size() << " notes "
       << std::setw(2) << sec/60 << ':'
       << std::setw(2) << std::setfill('0') << sec % 60;
    return os.str();
}

/// Make a new composition and write it to out.midi.
int main(int argc, char* argv[])
{
    std::string output = "composure.midi";
    int voices = 6;
    int passes = 8;
    int range = 24;
    int tempo = 60;
    std::optional<int> key;
    std::optional<int> seed;
    bool monophonic = false;
    bool chromatic = false;

    while (true)
    {
        static struct option options[] = {
            {"output", required_argument, nullptr, 'o'},
            {"voices", required_argument, nullptr, 'v'},
            {"passes", required_argument, nullptr, 'p'},
            {"range", required_argument, nullptr, 'r'},
            {"tempo", required_argument, nullptr, 't'},
            {"key", required_argument, nullptr, 'k'},
            {"seed", required_argument, nullptr, 's'},
            {"monophonic", no_argument, nullptr, 'm'},
            {"chromatic", no_argument, nullptr, 'c'},
            {0, 0, 0, 0}};
        int index;
        int c = getopt_long(argc, argv, "o:v:p:r:t:k:s:mc", options, &index);

        if (c == -1)
            break;
        switch (c)
        {
        case 0:
            break;
        case 'o':
            output = optarg;
            break;
        case 'v':
            voices = std::atoi(optarg);
            break;
        case 'p':
            passes = std::atoi(optarg);
            break;
        case 'r':
            range = std::atoi(optarg);
            break;
        case 't':
            tempo = std::atoi(optarg);
            break;
        case 'k':
            key = std::atoi(optarg);
            break;
        case 's':
            seed = std::atoi(optarg);
            break;
        case 'm':
            monophonic = true;
            break;
        case 'c':
            chromatic = true;
            break;
        default:
            std::cerr << "Usage: composure [options]\n"
                      << "    -o --output=  Output file name (" << output << ")\n"
                      << "    -v --voices=  Number of voices (" << voices << ")\n"
                      << "    -p --passes=  Number of compose/edit passes (" << passes << ")\n"
                      << "    -r --range=   Maximum range of notes (" << range << ")\n"
                      << "    -t --tempo=   Beats per minute (" << tempo << ")\n"
                      << "    -k --key=     60 for middle C (random 54 to 65)\n"
                      << "    -s --seed=    Random seed (random)\n"
                      << "    -m --monophonic\n"
                      << "    -c --chromatic"
                      << std::endl;
        }
    }

    std::ofstream log("composure.log");
    for (int i = 0; i < argc; ++i)
        log << argv[i] << ' ';
    log << '\n';
    log << "voices: " << voices << '\n'
        << "passes: " << passes << '\n'
        << "ragne: "  << range << '\n'
        << "tempo: "  << tempo << '\n';

    log << "seed " << (seed ? "" : "(random)") << ": ";
    std::random_device random;
    if (!seed)
        seed = random();
    log << *seed << '\n';

    set_random_seed(*seed);

    // Pick a random key from MIDI note 54 to 65: F# below middle C to F above.
    log << "key " << (key ? "" : "(random)") << ": ";
    if (!key)
        key = pick(54, 65);
    log << *key << '\n'
        << "monophonic: " << (monophonic ? "yes" : "no") << '\n'
        << "monophonic: " << (chromatic  ? "yes" : "no") << '\n';

    // Start with an empty phrase and iterate.
    Phrase phrase(tempo);
    for (int i = 0; i < passes; ++i)
    {
        log << "pass " << i + 1 << '/' << passes << '\n';
        phrase.compose(*key, voices, range, chromatic);
        log << "  compose: " << size_and_time(phrase.notes()) << '\n';
        phrase.edit();
        log << "  edit   : " << size_and_time(phrase.notes()) << '\n';
    }

    std::ofstream file(output);
    phrase.write_midi(file, monophonic);

    std::ofstream note_log("composure.notes");
    for (const auto& note : phrase.notes())
        note_log << note.time << ' ' << note.time + note.duration << ' '
                 << note.pitch << ' ' << note.generation << std::endl;

    return 0;
}
