#pragma once

#include "cista.h"
#include <string>

using std::string;

class IMessageSender {

    public:
    virtual void sendRawMessage(string type,cista::byte_buf contents) = 0;

    template <typename T>
    void sendMessage(T contents) {
        auto byte_buf = cista::serialize(contents);

        sendRawMessage(T::getMessageType().c_str(),byte_buf);
    }

    virtual ~IMessageSender() {}; 

};