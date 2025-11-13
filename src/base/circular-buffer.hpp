#pragma once

//
// Created by zhouj on 2023/3/29.
//

#include "type.hpp"
#include "buffer.hpp"


namespace wibot {
/**
 * @brief Circular buffer implementation using mirror flag technique
 * @tparam TE Element type stored in the buffer
 */
template <typename TE>
class CircularBuffer {
   private:
    TE *_buffer;    ///< Pointer to the underlying buffer storage
    u32 _capacity;  ///< Capacity of the buffer (must be power of 2)
    u32 _write;     ///< Write index with mirror flag
    u32 _read;      ///< Read index with mirror flag

   public:
    /**
     * @brief Construct a new Circular Slice object.
     * @note This Circular buffer use mirror flag to indicate the capacity, so the Size of the
     * buffer must be power of 2, and all the underlying memory can be used.
     * @xrefitem https://zh.wikipedia.org/wiki/%E7%92%B0%E5%BD%A2%E7%B7%A9%E8%A1%9D%E5%8D%80
     * @param buffer underline buffer.
     * @param capacity Underline buffer size.
     * @note Must be power of 2.
     */
    CircularBuffer(Slice buffer);

    /**
     * @brief Get the logical capacity of the buffer
     * @return The capacity of the buffer (power of 2)
     */
    u32 getCapacity() const;

    /**
     * @brief Get the memory capacity of the buffer
     * @return The actual memory size in bytes
     */
    u32 getMemCapacity() const;

    /**
     * @brief Get the data width of each element
     * @return The size of each element in bytes
     */
    u32 getDataWidth() const;

    /**
     * @brief Check if the buffer is full
     * @return true if the buffer is full, false otherwise
     */
    bool isFull() const;

    /**
     * @brief Check if the buffer is empty
     * @return true if the buffer is empty, false otherwise
     */
    bool isEmpty() const;

    /**
     * @brief Get the current number of elements in the buffer
     * @return The number of elements currently stored
     */
    u32 getSize() const;

    /**
     * @brief Get the available space in the buffer
     * @return The number of elements that can be written
     */
    u32 getSpace() const;

    /**
     * @brief Write data into buffer
     * @param data Data to be written.
     * @param length Length of data to be written.
     * @param allowCover Allow cover the data in buffer if the buffer is full.
     * If not, only the data that can be written will be written.
     * @note If allowCover is true, this function will modify the read index,
     * and user code should handle the concurrent problem.
     * @return The length of data that is written into buffer.
     */
    u32 write(const TE *data, u32 length, bool allowCover = true);

    /**
     * @brief Forward the write index after the data is written into the buffer by the
     * external device such as DMA.
     * @note User code should handle the concurrency issue, if the start index
     * is pushed over the end index.
     * @remark This function is mainly used for DMA transfer, so it can not
     * avoid the data cover issue. But generally speaking, the DMA speed is much
     * slower than the CPU, so it is not easy to cause data cover.
     * @param length The length of data that has been written into buffer by the
     * external device.
     * @return If overflow occurs, return true, otherwise return false.
     */
    bool writeVirtual(u32 length);

    /**
     * @brief Read data from buffer. If buffer has enough data, read all requested data.
     * @param data Slice to store the read data
     * @param length Number of elements to read
     * @return Actual length of data read.
     */
    u32 read(TE *data, u32 length);

    /**
     * @brief Forward the read index after the data is read from the buffer by
     * the external device such as DMA.
     * @note User code should handle the concurrency issue, if the start index
     * is pushed over the end index.
     * @param length The length of data that has been read from buffer.
     * @return If overflow occurs, return true, otherwise return false.
     */
    bool readVirtual(u32 length);

    /**
     * @brief Read data from buffer without changing the read index.
     *
     * @param data A buffer to store the data read.
     * @param start The start index of data to be read (relative to current read position).
     * @param length The length of data to be read.
     * @return The actual length of data has been read.
     */
    u32 peek(TE *data, u32 start = 0, u32 length = 1);

    /**
     * @brief Get the pointer of the data at the specified offset.
     *
     * @param offset Offset from the current read position
     * @param force Force to return pointer even if offset is out of range
     * @return TE* Pointer to the element at the specified offset, or nullptr if invalid
     */
    TE *peekPtr(u32 offset, bool force = false);

    /**
     * @brief Clear the buffer by resetting read and write indices
     * @return Result indicating success or failure
     */
    Result clear();

    /**
     * @brief Get the pointer to the underlying buffer
     * @return TE* Pointer to the buffer storage
     */
    TE *getDataPtr();

    /**
     * @brief Get the current write position pointer
     * @return TE* Pointer to the current write position
     */
    TE *getWritePtr() const;

    /**
     * @brief Get the current read position pointer
     * @return TE* Pointer to the current read position
     */
    TE *getReadPtr() const;

    /**
     * @brief Get the size of data from read index to the end of the buffer or data.
     * Used for zero copy operation.
     * @return If data size is smaller than the rest room from read index to the end, return the
     * data size, otherwise return the rest room size.
     */
    u32 getSizeWithoutMemWrap() const;

    /**
     * @brief Get the size of data from write index to the end of the buffer or data.
     * @note Calculates the length between two memory indices in the buffer
     * @param end The end index in memory index space (not logic index space).
     * @param start The start index in memory index space (not logic index space).
     * @return The length between start and end indices
     */
    u32 getLengthByMemIndex(u32 end, u32 start);
};

/**
 * @brief Type alias for CircularBuffer with u8 element type
 */
typedef CircularBuffer<u8> CircularBuffer8;

}  // namespace wibot
