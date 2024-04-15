#include "net/Channel.h"
#include "net/EventLoop.h"
#include <sstream>

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      state_(0),
      tied_(false) {}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::HandleEvent(Timestamp ReceiveTime)
{
    if (tied_)
    {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard)
        {
            HandleEventWithGuard(ReceiveTime);
        }
    }
    else
    {
        HandleEventWithGuard(ReceiveTime);
    }
}

void Channel::HandleEventWithGuard(Timestamp ReceiveTime)
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
            ReadCallback_(ReceiveTime);
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

std::string Channel::ReventsToString() const
{
  return EventsToString(fd_, revents_);
}

std::string Channel::EventsToString() const
{
  return EventsToString(fd_, events_);
}

std::string Channel::EventsToString(int fd, int ev) const
{
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & POLLIN)
    oss << "IN ";
  if (ev & POLLPRI)
    oss << "PRI ";
  if (ev & POLLOUT)
    oss << "OUT ";
  if (ev & POLLHUP)
    oss << "HUP ";
  if (ev & POLLRDHUP)
    oss << "RDHUP ";
  if (ev & POLLERR)
    oss << "ERR ";
  if (ev & POLLNVAL)
    oss << "NVAL ";

  return oss.str();
}