#include <juce_core/juce_core.h>
#include "LockFreeRingBuffer.h"
#include <thread>
#include <vector>

class LockFreeRingBufferTests : public juce::UnitTest
{
public:
    LockFreeRingBufferTests() : juce::UnitTest("LockFreeRingBuffer Tests") {}

    void runTest() override
    {
        beginTest("Ring Buffer Basic Operations");
        {
            LockFreeRingBuffer<float> buffer(1024);
            
            expect(buffer.getSize() == 1024, "Buffer size should be correct");
            expect(buffer.getNumAvailable() == 0, "Initially no data available");
            expect(buffer.getFreeSpace() == 1023, "Initially has max free space (size-1)");
        }

        beginTest("Ring Buffer Write and Read");
        {
            LockFreeRingBuffer<float> buffer(1024);
            std::vector<float> testData = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
            
            // Write data
            int written = buffer.write(testData.data(), static_cast<int>(testData.size()));
            expect(written == static_cast<int>(testData.size()), "Should write all data");
            expect(buffer.getNumAvailable() == static_cast<int>(testData.size()), 
                   "Available data should match written amount");
            
            // Read data back
            std::vector<float> readData(testData.size());
            int read = buffer.read(readData.data(), static_cast<int>(readData.size()));
            expect(read == static_cast<int>(testData.size()), "Should read all data");
            
            // Verify data
            for (size_t i = 0; i < testData.size(); ++i)
            {
                expect(readData[i] == testData[i], "Read data should match written data");
            }
            
            expect(buffer.getNumAvailable() == 0, "Buffer should be empty after reading");
        }

        beginTest("Ring Buffer Wraparound");
        {
            LockFreeRingBuffer<float> buffer(8); // Small buffer to test wraparound
            std::vector<float> data1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
            std::vector<float> data2 = {6.0f, 7.0f, 8.0f};
            
            // Fill most of the buffer
            buffer.write(data1.data(), static_cast<int>(data1.size()));
            
            // Read some data to create space
            std::vector<float> temp(3);
            buffer.read(temp.data(), 3);
            
            // Write more data (should wrap around)
            int written = buffer.write(data2.data(), static_cast<int>(data2.size()));
            expect(written == static_cast<int>(data2.size()), "Should write wraparound data");
            
            // Read remaining data
            std::vector<float> remaining(5); // 2 from first write + 3 from second
            int read = buffer.read(remaining.data(), 5);
            expect(read == 5, "Should read all remaining data");
        }

        beginTest("Ring Buffer Overflow Protection");
        {
            LockFreeRingBuffer<float> buffer(8);
            std::vector<float> largeData(10, 1.0f); // More data than buffer can hold
            
            int written = buffer.write(largeData.data(), static_cast<int>(largeData.size()));
            expect(written == 7, "Should only write what fits (size-1)");
            expect(buffer.getNumAvailable() == 7, "Available should match written amount");
        }

        beginTest("Ring Buffer Thread Safety");
        {
            LockFreeRingBuffer<float> buffer(1024);
            constexpr int numSamples = 1000;
            std::vector<float> writeData(numSamples);
            std::vector<float> readData(numSamples);
            
            // Fill write data with test pattern
            for (int i = 0; i < numSamples; ++i)
            {
                writeData[i] = static_cast<float>(i);
            }
            
            // Writer thread
            std::thread writer([&]() {
                int totalWritten = 0;
                while (totalWritten < numSamples)
                {
                    int toWrite = juce::jmin(100, numSamples - totalWritten);
                    int written = buffer.write(writeData.data() + totalWritten, toWrite);
                    totalWritten += written;
                    if (written == 0)
                        std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
            });
            
            // Reader thread
            std::thread reader([&]() {
                int totalRead = 0;
                while (totalRead < numSamples)
                {
                    int toRead = juce::jmin(50, numSamples - totalRead);
                    int read = buffer.read(readData.data() + totalRead, toRead);
                    totalRead += read;
                    if (read == 0)
                        std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
            });
            
            writer.join();
            reader.join();
            
            // Verify all data was transferred correctly
            for (int i = 0; i < numSamples; ++i)
            {
                expect(readData[i] == writeData[i], "Thread-safe transfer should preserve data");
            }
        }
    }
};

static LockFreeRingBufferTests lockFreeRingBufferTests;