#include "tx-server.hpp"
#include "type.hpp"

namespace wibot {
TxServer::TxServer(AsyncWriter<Slice>* writer)
    : _writer(writer),
      _fifo("tx", _fifoBuffer.data, sizeof(Buffer<MAX_TX_SERVER_FRAME_SIZE>),
            MAX_TX_SERVER_FIFO_LENGTH) {};

TxServer::~TxServer() {
}

void TxServer::run() {
    Buffer<MAX_TX_SERVER_FRAME_SIZE> dataBuffer;
    while (true) {
        auto rst = _fifo.receive(&dataBuffer, TIMEOUT_FOREVER);
        while (rst.isOk()) {
            if (rst.isOk()) {
                _writer->write(dataBuffer).wait(TIMEOUT_FOREVER);
            }
        }
    }
}

Result TxServer::send(const Buffer<MAX_TX_SERVER_FRAME_SIZE>& data) {
    return _fifo.send(&data, TIMEOUT_FOREVER);
}
}  // namespace wibot
