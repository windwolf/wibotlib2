#pragma once

#include "buffer.hpp"
#include "circular-buffer.hpp"
#include "os/os.hpp"
#include "comm/msg/message-reader.hpp"

#ifndef MAX_RX_SERVER_FRAME_SIZE
#define MAX_RX_SERVER_FRAME_SIZE 16
#endif

namespace wibot {
class RxServer : public Worker {
   public:
    RxServer(MessageReader& reader);
    ~RxServer();

   public:
    void setTimeout(u32 timeoutMs);

   private:
    void run() override;

    Result startServer(bool retry = true);
    Result stopServer(bool retry = true);

   protected:
    virtual bool validateFrame(const MessageFrame& frame)       = 0;
    virtual void processCommandFrame(const MessageFrame& frame) = 0;

   protected:
    MessageReader& _reader;
    u32            _timeoutMs = 1000;
};
}  // namespace wibot
