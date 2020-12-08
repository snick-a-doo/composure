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

#include <csound/csound.hpp>
#include <csound/csPerfThread.hpp>

#include <memory>
#include <string>

/// A threaded Csound session
class Session
{
public:
    /// Load a Csound definition.
    Session(const std::string& csd_text);
    ~Session();

    /// Start playing from the beginning.
    void restart();
    /// Load a new file.
    void load(const std::string& csd_file);
    void play();
    void pause();
    void rewind();
    void stop();

    /// Change the score.
    void set_score(const std::string& score);
    bool is_running() const;

private:
    void init();
    void start();

    Csound m_csound;
    std::string m_csd_text;
    std::unique_ptr<CsoundPerformanceThread> mp_perf_thread;
    /// A variable parameter in the csd file.
    MYFLT* mp_factor = nullptr;
};
