#include "Message.h"

GioNet::Message::Message(Buffer&& data): data(std::move(data))
{  }
