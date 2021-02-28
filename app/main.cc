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

#include <getopt.h>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <optional>
#include <random>
#include <regex>

static std::string version = "1.1.0";

double beat_to_sec(double beats, double tempo)
{
    return beats * 60.0 / tempo;
}

std::string size_and_time(const std::vector<Note>& notes, int tempo)
{
    // Convert note times from beats to seconds.
    auto sec = notes.empty()
        ? 0
        : static_cast<int>(beat_to_sec(notes.back().time + notes.back().duration, tempo));
    std::ostringstream os;
    os << std::setw(4) << notes.size() << " notes "
       << std::setw(2) << sec/60 << ':'
       << std::setw(2) << std::setfill('0') << sec % 60;
    return os.str();
}

/// The command-line options and defaults. Optional entries are set randomly if not
/// specified.
struct Options
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
};

/// If a log file was passed as an argument, set the command-line options from the
/// settings in the file.
void read_options(int argc, char* argv[], Options& opt)
{
    auto set = [&opt](const auto& label, const auto& value, bool random) {
        // Don't set the option if it was originally chosen randomly. It must be chosen
        // randomly again (from the same seed) so the generator is in the same state as in
        // the original when it gets to compose().
        if (random && label != "seed")
            return;
        if (label == "output")
            opt.output = value;
        else if (label == "voices")
            opt.voices = std::stoi(value);
        else if (label == "passes")
            opt.passes = std::stoi(value);
        else if (label == "range")
            opt.range = std::stoi(value);
        else if (label == "tempo")
            opt.tempo = std::stoi(value);
        else if (label == "seed")
            opt.seed = std::stoul(value);
        else if (label == "key")
            opt.key = std::stoi(value);
        else if (label == "monophonic")
            opt.monophonic = value == "yes";
        else if (label == "chromatic")
            opt.chromatic = value == "yes";
        else
            assert(false); // Unknown label/value pair in log file.
    };

    for (int i = 1; i < argc; ++i)
    {
        std::filesystem::path arg(argv[i]);
        if (arg.extension() == ".log")
        {
            std::ifstream log(arg);
            std::string line;
            // Match lines like
            // key: 60
            // key (random): 60
            std::regex label_val_re("^([a-z]+)(.*): ([a-z0-9]+)$");
            std::smatch match;
            while (log)
            {
                std::getline(log, line);
                if (std::regex_match(line, match, label_val_re) && match.size() >= 3)
                    set(match[1].str(), match[3].str(), match[2].length() != 0);
            }
        }
    }
}

/// Make a new composition and write it to a MIDI file.
int main(int argc, char* argv[])
{
    Options opt;
    // Set default options from the log file if specified. They may be overridden by
    // command-line options parsed below.
    read_options(argc, argv, opt);
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
            opt.output = optarg;
            break;
        case 'v':
            opt.voices = std::stoi(optarg);
            break;
        case 'p':
            opt.passes = std::stoi(optarg);
            break;
        case 'r':
            opt.range = std::stoi(optarg);
            break;
        case 't':
            opt.tempo = std::stoi(optarg);
            break;
        case 'k':
            opt.key = std::stoi(optarg);
            break;
        case 's':
            opt.seed = std::stoul(optarg);
            break;
        case 'm':
            opt.monophonic = true;
            break;
        case 'c':
            opt.chromatic = true;
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
                      << "    -o --output=    Output file name (" << opt.output << ")\n"
                      << "    -p --passes=    Number of compose/edit passes (" << opt.passes << ")\n"
                      << "    -r --range=     Maximum range of notes (" << opt.range << ")\n"
                      << "    -s --seed=      Random seed (random)\n"
                      << "    -t --tempo=     Beats per minute (" << opt.tempo << ")\n"
                      << "    -v --voices=    Number of voices (" << opt.voices << ")\n"
                      << std::endl;
            exit(c == 'h' ? 0 : 1);
        }
    }

    std::ofstream log(opt.output + ".log");
    for (int i = 0; i < argc; ++i)
        log << argv[i] << ' ';
    log << '\n';
    log << "version: " << version << '\n'
        << "output: " << opt.output << '\n'
        << "voices: " << opt.voices << '\n'
        << "passes: " << opt.passes << '\n'
        << "range: "  << opt.range << '\n'
        << "tempo: "  << opt.tempo << '\n';

    log << "seed" << (opt.seed ? "" : " (random)") << ": ";
    std::random_device random;
    if (!opt.seed)
        opt.seed = random();
    log << *opt.seed << '\n';

    set_random_seed(*opt.seed);

    // Pick a random key from MIDI note 54 to 65: F# below middle C to F above.
    log << "key" << (opt.key ? "" : " (random)") << ": ";
    if (!opt.key)
        opt.key = pick(54, 65);
    log << *opt.key << '\n'
        << "monophonic: " << (opt.monophonic ? "yes" : "no") << '\n'
        << "chromatic: " << (opt.chromatic  ? "yes" : "no") << '\n';

    // Start with an empty phrase and iterate.
    Phrase phrase(opt.tempo);
    for (int i = 0; i < opt.passes; ++i)
    {
        log << "pass " << i + 1 << '/' << opt.passes << '\n';
        phrase.compose(*opt.key, opt.voices, opt.range, opt.chromatic);
        log << "  compose: " << size_and_time(phrase.notes(), opt.tempo) << '\n';
        phrase.edit();
        log << "  edit   : " << size_and_time(phrase.notes(), opt.tempo) << '\n';
    }

    std::ofstream file(opt.output + ".midi");
    phrase.write_midi(file, opt.monophonic);

    // Write a text file with information about each note.
    std::ofstream note_log(opt.output + ".notes");
    for (const auto& note : phrase.notes())
    {
        note_log << beat_to_sec(note.time, opt.tempo) << ' '
                 << beat_to_sec(note.time + note.duration, opt.tempo) << ' '
                 << note.pitch << ' ' << note.generation << std::endl;
    }

    // Print out the number of notes, total time, and random seed.
    std::cout << size_and_time(phrase.notes(), opt.tempo) << "  "
              << *opt.seed << std::endl;
    return 0;
}
