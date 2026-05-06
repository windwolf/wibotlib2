#pragma once

#include "circular-buffer.hpp"
#include "os/os.hpp"
#include "bus.hpp"

#ifndef MAX_TX_SERVER_FRAME_SIZE
#define MAX_TX_SERVER_FRAME_SIZE 16
#endif
#ifndef MAX_TX_SERVER_FIFO_LENGTH
#define MAX_TX_SERVER_FIFO_LENGTH 2
#endif

namespace wibot {
class TxServer : public Worker {
   public:
    TxServer(AsyncWriter<Slice>* writer);
    ~TxServer();
    Result send(const Buffer<MAX_TX_SERVER_FRAME_SIZE>& data);

   private:
    void run() override;

   private:
    AsyncWriter<Slice>* _writer;

    Buffer<sizeof(Buffer<MAX_TX_SERVER_FRAME_SIZE>) * MAX_TX_SERVER_FIFO_LENGTH> _fifoBuffer;
    MessageQueue                                                                 _fifo;
};
}  // namespace wibot
