#include "net/Signals.h"
#include "base/Logger.h"
#include <sys/signalfd.h>
#include <signal.h>
#include <format>

using namespace std;

int CreatSignalfd(initializer_list<int> SignalList)
{
    sigset_t sig;
    ::sigemptyset(&sig);
    for (auto i : SignalList)
    {
        if (sigaddset(&sig, i) != 0)
        {
            LOG_FATAL << "Signals::CreatSignalfd()::sigaddset";
        }
    }
    if (::sigprocmask(SIG_BLOCK, &sig, NULL) != 0)
    {
        LOG_FATAL << "Signals::CreatSignalfd()::sigprocmask";
    }
    int fd = ::signalfd(-1, &sig, SFD_NONBLOCK | SFD_CLOEXEC);
    if (fd < 0)
    {
        LOG_FATAL << "Signals::CreatSignalfd()::signalfd";
    }
    return fd;
}

Signals::Signals(EventLoop *loop, initializer_list<int> SignalList)
    : loop_(loop),
      SignalFd_(CreatSignalfd(SignalList)),
      SignalChannel_(loop, SignalFd_),
      callback_()
{
}

Signals::~Signals()
{
    SignalChannel_.DisableAll();
    SignalChannel_.remove();
    ::close(SignalFd_);
}

void Signals::wait(const SignalCallback &f)
{
    callback_ = move(f);
    SignalChannel_.SetReadCallback(bind(&Signals::HandleRead, this));
    SignalChannel_.EnableReading();
}

void Signals::HandleRead()
{
    signalfd_siginfo SignalInfo;
    size_t n = ::read(SignalFd_, &SignalInfo, sizeof(SignalInfo));
    if(n != sizeof(SignalInfo))
    {
        LOG_ERROR << format("Signals::HandleRead() read {} bytes instead of {}", n, sizeof(SignalInfo));
    }
    callback_();
}