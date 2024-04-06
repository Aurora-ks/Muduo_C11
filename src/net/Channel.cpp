#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop *loop, int fd) : loop_(loop),
                                            fd_(fd),
                                            events_(0),
                                            revents_(0),
                                            state_(0) {}

void Channel::HandleEvent(TimeStamp time)
{
    if (revents_ & POLLHUP && !(revents_ & POLLIN))
    {
        if (CloseCallback_)
            CloseCallback_();
    }
    if (revents_ & POLLERR)
    {
        if (ErrorCallback_)
            ErrorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI))
    {
        if (ReadCallback_)
            ReadCallback_(time);
    }
    if (revents_ & POLLOUT)
    {
        if (WriteCallback_)
            WriteCallback_();
    }
}

void Channel::EnableReading()
{
    events_ |= (POLLIN | POLLPRI);
    update();
}

void Channel::DisableReading()
{
    events_ &= ~(POLLIN | POLLPRI);
    update();
}

void Channel::EnableWriting()
{
    events_ |= POLLOUT;
    update();
}

void Channel::DisableWriting()
{
    events_ &= ~POLLOUT;
    update();
}

void Channel::DisableAll()
{
    events_ = 0;
    update();
}

void Channel::remove()
{
    loop_->RemoveChannel(this);
}
void Channel::update()
{
    loop_->UpdateChannel(this);
}