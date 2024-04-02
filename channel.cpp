#include "channel.hpp"
#include "eventloop.hpp"

void Channel::remove()
{
    loop_->RemoveChannel(this);
}
void Channel::update()
{
    loop_->UpdateChannel(this);
}