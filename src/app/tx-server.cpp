#include "tx-server.hpp"
#include "type.hpp"

namespace wibot {
TxServer::TxServer(AsyncWriter<Slice>* writer)
    : _writer(writer),
      _fifo("tx", _fifoBuffer.data, sizeof(Buffer<MAX_TX_SERVER_FRAME_SIZE>),
            MAX_TX_SERVER_FIFO_LENGTH) {
    ASSERT(_writer != nullptr, "TxServer writer must not be null.");
};

TxServer::~TxServer() {
}

void TxServer::run() {
    Buffer<MAX_TX_SERVER_FRAME_SIZE> dataBuffer;
    while (true) {
        auto rst = _fifo.receive(&dataBuffer, TIMEOUT_FOREVER);
        if (rst.isOk()) {
            _writer->write(dataBuffer).wait(TIMEOUT_FOREVER);
        }
    }
}

Result TxServer::send(const Buffer<MAX_TX_SERVER_FRAME_SIZE>& data) {
    return _fifo.send(&data, TIMEOUT_FOREVER);
}
}  // namespace wibot
