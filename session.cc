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
