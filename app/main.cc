// Copyright © 2020-2021 Sam Varner
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

#include <phrase.hh>
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

static std::string version = "1.1.0";

std::string size_and_time(const std::vector<Note>& notes, int tempo)
{
    // Convert note times from beats to seconds.
    auto sec = notes.empty()
        ? 0 : static_cast<int>(60.0/tempo * (notes.back().time + notes.back().duration));
    std::ostringstream os;
    os << std::setw(4) << notes.size() << " notes "
       << std::setw(2) << sec/60 << ':'
       << std::setw(2) << std::setfill('0') << sec % 60;
    return os.str();
}

/// Make a new composition and write it to a MIDI file.
int main(int argc, char* argv[])
{
    std::string output = "composure";
    int voices = 6;
    int passes = 8;
    int range = 24;
    int tempo = 60;
    std::optional<int> key;
    std::optional<unsigned int> seed;
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
            {"help", no_argument, nullptr, 'h'},
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
            voices = std::stoi(optarg);
            break;
        case 'p':
            passes = std::stoi(optarg);
            break;
        case 'r':
            range = std::stoi(optarg);
            break;
        case 't':
            tempo = std::stoi(optarg);
            break;
        case 'k':
            key = std::stoi(optarg);
            break;
        case 's':
            seed = std::stoul(optarg);
            break;
        case 'm':
            monophonic = true;
            break;
        case 'c':
            chromatic = true;
            break;
        case 'h':
            std::cerr << "Composure: Naive music composition\n"
                      << "Version " << version << " © 2021 Sam Varner\n"
                      << "https://github.com/snick-a-doo/composure\n";
            // Fall through
        default:
            std::cerr << "\nUsage: composure [options]\n"
                      << "    -c --chromatic  (false)\n"
                      << "    -h --help       Display this help and exit.\n"
                      << "    -k --key=       60 for middle C (random 54 to 65)\n"
                      << "    -m --monophonic (false)\n"
                      << "    -o --output=    Output file name (" << output << ")\n"
                      << "    -p --passes=    Number of compose/edit passes (" << passes << ")\n"
                      << "    -r --range=     Maximum range of notes (" << range << ")\n"
                      << "    -s --seed=      Random seed (random)\n"
                      << "    -t --tempo=     Beats per minute (" << tempo << ")\n"
                      << "    -v --voices=    Number of voices (" << voices << ")\n"
                      << std::endl;
            exit(c == 'h' ? 0 : 1);
        }
    }

    std::ofstream log(output + ".log");
    for (int i = 0; i < argc; ++i)
        log << argv[i] << ' ';
    log << '\n';
    log << "version: " << version << '\n'
        << "voices: " << voices << '\n'
        << "passes: " << passes << '\n'
        << "range: "  << range << '\n'
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
        << "chromatic: " << (chromatic  ? "yes" : "no") << '\n';

    // Start with an empty phrase and iterate.
    Phrase phrase(tempo);
    for (int i = 0; i < passes; ++i)
    {
        log << "pass " << i + 1 << '/' << passes << '\n';
        phrase.compose(*key, voices, range, chromatic);
        log << "  compose: " << size_and_time(phrase.notes(), tempo) << '\n';
        phrase.edit();
        log << "  edit   : " << size_and_time(phrase.notes(), tempo) << '\n';
    }

    std::ofstream file(output + ".midi");
    phrase.write_midi(file, monophonic);

    // Write a text file with information about each note.
    std::ofstream note_log(output + ".notes");
    for (const auto& note : phrase.notes())
        note_log << note.time << ' ' << note.time + note.duration << ' '
                 << note.pitch << ' ' << note.generation << std::endl;

    // Print out the number of notes and total time.
    std::cout << size_and_time(phrase.notes(), tempo) << std::endl;
    return 0;
}
