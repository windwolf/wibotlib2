//
// Created by zhouj on 2024/3/30.
//

#include "hal-flash.hpp"

namespace wibot {

const void* HalFlash::read(u32 address, u32 size) {
    ASSERT(address + size <= kFlashSize, "erasePage: offset + size > kFlashSize");

    return reinterpret_cast<const void*>(address);
}
Result HalFlash::erasePage(u32 address, u32 size) {
    ASSERT(address + size <= kFlashSize, "erasePage: offset + size > kFlashSize");
    ASSERT((address & kPageOffsetMask) == 0, "erasePage: offset is not page aligned");
    ASSERT((size & kPageOffsetMask) == 0, "erasePage: size is not page aligned");

    FLASH_EraseInitTypeDef erase_info;
    erase_info.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_info.Banks     = FLASH_BANK_1;
    erase_info.NbPages   = size / kPageSize;
#ifdef STM32F1xx
    erase_info.PageAddress = address;
#else
    erase_info.Page = (address - kBaseAddress) / kPageSize;
#endif
    u32 page_error = 0;

    HAL_FLASH_Unlock();

    HAL_FLASHEx_Erase(&erase_info, &page_error);

    HAL_FLASH_Lock();
    if (page_error != 0xFFFFFFFF) {
        return Result(Result::ResultStatus::kError, page_error);
    }
    return Result::kOk;
}
Result HalFlash::writePage(u32 address, const void* data, u32 size) {
    auto rst = HAL_OK;

    ASSERT(address + size <= kFlashSize, "erasePage: offset + size > kFlashSize");
    ASSERT((address & kPageOffsetMask) == 0, "erasePage: offset is not page aligned");
    u32                    sizeAligned = (size + kPageOffsetMask) & ~kPageOffsetMask;
    FLASH_EraseInitTypeDef erase_info;
    erase_info.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_info.Banks     = FLASH_BANK_1;
    erase_info.NbPages   = sizeAligned / kPageSize;
#ifdef STM32F1xx
    erase_info.PageAddress = address;
#else
    erase_info.Page = (address - kBaseAddress) / kPageSize;
#endif
    u32 page_error = 0;

    HAL_FLASH_Unlock();

    HAL_FLASHEx_Erase(&erase_info, &page_error);

    auto dataAddr   = static_cast<u64*>(static_cast<void*>(&data));
    auto remainSize = size;
    while (remainSize > 0) {
        auto halRst = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, *dataAddr);
        if (halRst != HAL_OK) {
            rst = halRst;
            break;
        }
        address += kWriteSize;
        dataAddr += 1;
        remainSize -= kWriteSize;
    }
    HAL_FLASH_Lock();
    return Result(rst);
}
}  // namespace wibot
