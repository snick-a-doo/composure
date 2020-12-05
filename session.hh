#include <csound/csound.hpp>
#include <csound/csPerfThread.hpp>

#include <memory>
#include <string>

/// A threaded Csound session
class Session
{
public:
    /// Load a Csound file.
    Session(const std::string& csd_file);
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
    std::string m_csd_file;
    std::unique_ptr<CsoundPerformanceThread> mp_perf_thread;
    /// A variable parameter in the csd file.
    MYFLT* mp_factor = nullptr;
};
