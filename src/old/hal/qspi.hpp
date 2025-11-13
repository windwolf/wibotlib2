#pragma once

#include "peripheral.hpp"
#include "wait-handler.hpp"

#define FSCCAutoPollingTypeDef       QSPI_AutoPollingTypeDef
#define FSCC_EVENT_AUTO_POLLING_CPLT 0x08000000

namespace wibot {
union QSPIConfig {
    struct {
        DataWidth dataWidth      : 2;
        bool      useTxDma       : 1;
        bool      useRxDma       : 1;
        u8        dummyCycles    : 4;
        u8        txDmaThreshold : 8;
        u8        rxDmaThreshold : 8;
        u8        mode           : 1;
        u32                      : 7;
    };
    u32 value;
};

class QSPI : private Initializable {
   public:
    QSPI(QSPI_CTOR_ARG);

    Result read(void *data, u32 size, WaitHandler &waitHandler);
    Result write(void *data, u32 size, WaitHandler &waitHandler);

   private:
    void init();

   public:
    QSPIConfig config;

   private:
    QSPI_FIELD_DECL
    union {
        struct {
            bool isTxDmaEnabled : 1;
            bool isRxDmaEnabled : 1;
        };
        u32 value;
    } _status;

    WaitHandler *_readWaitHandler;
    WaitHandler *_writeWaitHandler;
    Slice        _txBuffer;
    Slice        _rxBuffer;

   protected:
    static void _on_read_complete_callback(QSPI_CALLBACK_ARG);
    static void _on_write_complete_callback(QSPI_CALLBACK_ARG);
    static void _on_command_complete_callback(QSPI_CALLBACK_ARG);
    static void _on_status_match_callback(QSPI_CALLBACK_ARG);
    static void OnErrorCb(QSPI_CALLBACK_ARG);
};

}  // namespace wibot
