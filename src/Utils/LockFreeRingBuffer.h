#pragma once

#include <atomic>
#include <memory>

template<typename T>
class LockFreeRingBuffer
{
public:
    explicit LockFreeRingBuffer(size_t capacity)
        : capacity_(capacity + 1), buffer_(std::make_unique<T[]>(capacity_))
    {
        readIndex_.store(0);
        writeIndex_.store(0);
    }

    bool write(const T* data, size_t count)
    {
        const auto currentWriteIndex = writeIndex_.load();
        const auto currentReadIndex = readIndex_.load();
        
        size_t availableSpace;
        if (currentWriteIndex >= currentReadIndex)
        {
            availableSpace = capacity_ - currentWriteIndex + currentReadIndex - 1;
        }
        else
        {
            availableSpace = currentReadIndex - currentWriteIndex - 1;
        }
        
        if (count > availableSpace)
        {
            return false; // Not enough space
        }
        
        for (size_t i = 0; i < count; ++i)
        {
            buffer_[(currentWriteIndex + i) % capacity_] = data[i];
        }
        
        writeIndex_.store((currentWriteIndex + count) % capacity_);
        return true;
    }

    bool read(T* data, size_t count)
    {
        const auto currentReadIndex = readIndex_.load();
        const auto currentWriteIndex = writeIndex_.load();
        
        size_t availableData;
        if (currentWriteIndex >= currentReadIndex)
        {
            availableData = currentWriteIndex - currentReadIndex;
        }
        else
        {
            availableData = capacity_ - currentReadIndex + currentWriteIndex;
        }
        
        if (count > availableData)
        {
            return false; // Not enough data
        }
        
        for (size_t i = 0; i < count; ++i)
        {
            data[i] = buffer_[(currentReadIndex + i) % capacity_];
        }
        
        readIndex_.store((currentReadIndex + count) % capacity_);
        return true;
    }

    size_t available() const
    {
        const auto currentReadIndex = readIndex_.load();
        const auto currentWriteIndex = writeIndex_.load();
        
        if (currentWriteIndex >= currentReadIndex)
        {
            return currentWriteIndex - currentReadIndex;
        }
        else
        {
            return capacity_ - currentReadIndex + currentWriteIndex;
        }
    }

    size_t space() const
    {
        return capacity_ - available() - 1;
    }

private:
    const size_t capacity_;
    std::unique_ptr<T[]> buffer_;
    std::atomic<size_t> readIndex_;
    std::atomic<size_t> writeIndex_;
};