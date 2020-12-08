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

#include "session.hh"

Session::Session(const std::string& csd_text)
{
    load(csd_text);
}

Session::~Session()
{
    if (!mp_perf_thread)
        return;
    mp_perf_thread->Stop();
    mp_perf_thread->Join();
}

void Session::load(const std::string& csd_text)
{
    m_csd_text = csd_text;
    init();
    start();
}

void Session::init()
{
    m_csound.GetChannelPtr(mp_factor, "factor",
                           CSOUND_CONTROL_CHANNEL | CSOUND_INPUT_CHANNEL);
    *mp_factor = 1.0;
}

void Session::start()
{
    if (m_csound.CompileCsdText(m_csd_text.c_str()) != 0)
        return;
    m_csound.Start();
    mp_perf_thread = std::make_unique<CsoundPerformanceThread>(&m_csound);
    mp_perf_thread->Play();
}

void Session::restart()
{
    stop();
    init();
    start();
}

void Session::play()
{
    if (mp_perf_thread)
        mp_perf_thread->Play();
}

void Session::pause()
{
    if (mp_perf_thread)
        mp_perf_thread->Pause();
}

void Session::rewind()
{
    m_csound.RewindScore();
}

void Session::stop()
{
    if (mp_perf_thread)
    {
        if (is_running())
            mp_perf_thread->Stop();
        mp_perf_thread->Join();
        mp_perf_thread.reset();
    }
    m_csound.Reset();
}

void Session::set_score(const std::string& score)
{
    m_csound.ReadScore(score.c_str());
}

bool Session::is_running() const
{
    return mp_perf_thread && mp_perf_thread->GetStatus() == 0;
}
