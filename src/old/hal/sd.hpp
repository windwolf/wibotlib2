#pragma once

#include "peripheral.hpp"

namespace wibot {

SD_PER_DECL

union SdConfig {
    struct {
        bool useTxDma : 1;
        bool useRxDma : 1;
    };
    u32 value;
};

class SdCard : private Initializable {
   public:
    SdCard(SD_CTOR_ARG);
    CardInfo &card_info_get();
    Result    read(void *data, u32 num, u32 count, WaitHandler WaitHandler);
    Result    write(void *data, u32 num, u32 count, WaitHandler WaitHandler);
    Result    erase(u32 num, u32 count, WaitHandler WaitHandler);

    Result status_query();
    Result card_init();

   private:
    void init() override;

   public:
    SdConfig config;

   private:
    SD_FIELD_DECL
    union {
        struct {
            bool isTxDmaEnabled : 1;
            bool isRxDmaEnabled : 1;
        };
        u32 value;
    } _status;

    CardInfo     _cardInfo;
    WaitHandler *_waitHandler;
    Slice        _txBuffer;
    Slice        _rxBuffer;

   protected:
    static void _on_read_complete_callback(SD_CALLBACK_ARG);
    static void _on_write_complete_callback(SD_CALLBACK_ARG);
    static void OnErrorCb(SD_CALLBACK_ARG);

}

class SdCardBlock : public Block {
   public:
    SdCardBlock(SdCard &sdcard, Slice buffer);
    Result card_init();

   private:
    SdCard &_instance;
    Slice   _buffer;

   protected:
    Result media_read(void *data, u32 num, u32 size, WaitHandler &waitHandler) override;
    Result media_write(void *data, u32 num, u32 size, WaitHandler &waitHandler) override;
    Result media_erase(u32 num, u32 size, WaitHandler &waitHandler) override;
}

}  // namespace wibot
