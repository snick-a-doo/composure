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

#include "compose.hh"
#include "random.hh"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <getopt.h>

/// Make a new composition and write it to out.midi.
int main(int argc, char** argv)
{
    std::string output = "composure.midi";
    int voices = 6;
    int passes = 8;
    int range = 24;
    int tempo = 60;
    int key = -1;
    int seed = -1;
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
            std::cerr << "Usage: composure "
                      << "[[-o|--output=] Output file name "
                      << "[[-v|--voices=] Number of voices "
                      << "[[-p|--passes=] Number of compose/edit passes "
                      << "[[-r|--range=] Maximum range of notes "
                      << "[[-t|--tempo=] Beats per minute "
                      << "[[-k|--key=] 60 for middle C"
                      << "[[-s|--seed=] Random seed "
                      << "[[-m|--monophonic] "
                      << "[[-c|--chromatic] "
                      << std::endl;
        }
    }

    if (seed != -1)
        set_random_seed(seed);
    // Pick a random key from MIDI note 54 to 65: F# below middle C to F above.
    if (key == -1)
        key = pick(54, 65);
    // Start with an empty phrase and iterate.
    Phrase phrase(tempo);
    for (int i = 0; i < passes; ++i)
        phrase = edit(compose(phrase, key, voices, range, chromatic));

    std::ofstream file(output);
    phrase.write_midi(file, monophonic);

    return 0;
}
