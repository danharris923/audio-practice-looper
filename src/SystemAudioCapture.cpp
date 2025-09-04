#include "SystemAudioCapture.h"
#include <thread>
#include <chrono>

SystemAudioCapture::SystemAudioCapture() = default;

SystemAudioCapture::~SystemAudioCapture()
{
    shutdown();
}

bool SystemAudioCapture::initialize(double sampleRate, int numChannels)
{
    targetSampleRate_ = sampleRate;
    targetChannels_ = numChannels;
    
#ifdef _WIN32
    return initializeWASAPI();
#elif defined(__linux__)
    return initializePulseAudio();
#else
    return false; // Unsupported platform
#endif
}

void SystemAudioCapture::shutdown()
{
    stopCapture();
    
#ifdef _WIN32
    shutdownWASAPI();
#elif defined(__linux__)
    shutdownPulseAudio();
#endif
}

bool SystemAudioCapture::startCapture()
{
    if (isCapturing_.load())
        return true;
    
    shouldStop_.store(false);
    isCapturing_.store(true);
    
#ifdef _WIN32
    captureThread_ = std::make_unique<std::thread>(&SystemAudioCapture::captureThreadWASAPI, this);
#elif defined(__linux__)
    captureThread_ = std::make_unique<std::thread>(&SystemAudioCapture::captureThreadPulse, this);
#endif
    
    return captureThread_ != nullptr;
}

void SystemAudioCapture::stopCapture()
{
    if (!isCapturing_.load())
        return;
    
    shouldStop_.store(true);
    isCapturing_.store(false);
    
    if (captureThread_ && captureThread_->joinable())
    {
        captureThread_->join();
        captureThread_.reset();
    }
}

std::vector<SystemAudioCapture::CaptureDevice> SystemAudioCapture::getAvailableDevices()
{
#ifdef _WIN32
    return getWASAPIDevices();
#elif defined(__linux__)
    return getPulseDevices();
#else
    return {};
#endif
}

bool SystemAudioCapture::selectDevice(const std::string& deviceId)
{
    selectedDeviceId_ = deviceId;
    
    // If currently capturing, restart with new device
    if (isCapturing_.load())
    {
        stopCapture();
        shutdown();
        initialize(targetSampleRate_, targetChannels_);
        return startCapture();
    }
    
    return true;
}

void SystemAudioCapture::setVolume(float volume)
{
    volume = juce::jlimit(0.0f, 1.0f, volume);
    
#ifdef _WIN32
    if (volumeControl_)
    {
        volumeControl_->SetMasterVolume(volume, nullptr);
    }
#endif
    // Note: PulseAudio volume control would be implemented here for Linux
}

float SystemAudioCapture::getVolume() const
{
#ifdef _WIN32
    if (volumeControl_)
    {
        float volume = 0.0f;
        if (SUCCEEDED(volumeControl_->GetMasterVolume(&volume)))
        {
            return volume;
        }
    }
#endif
    return 1.0f;
}

#ifdef _WIN32

bool SystemAudioCapture::initializeWASAPI()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        return false;
    
    // Create device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                         __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator_);
    if (FAILED(hr))
        return false;
    
    // Get capture device
    if (selectedDeviceId_.empty())
    {
        // Use default device
        hr = deviceEnumerator_->GetDefaultAudioEndpoint(eRender, eLoopback, &captureDevice_);
    }
    else
    {
        // Use selected device
        std::wstring wideId(selectedDeviceId_.begin(), selectedDeviceId_.end());
        hr = deviceEnumerator_->GetDevice(wideId.c_str(), &captureDevice_);
    }
    
    if (FAILED(hr))
        return false;
    
    // Activate audio client
    hr = captureDevice_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audioClient_);
    if (FAILED(hr))
        return false;
    
    return setupAudioClient();
}

void SystemAudioCapture::shutdownWASAPI()
{
    if (captureEvent_)
    {
        CloseHandle(captureEvent_);
        captureEvent_ = nullptr;
    }
    
    if (mixFormat_)
    {
        CoTaskMemFree(mixFormat_);
        mixFormat_ = nullptr;
    }
    
    if (volumeControl_)
    {
        volumeControl_->Release();
        volumeControl_ = nullptr;
    }
    
    if (captureClient_)
    {
        captureClient_->Release();
        captureClient_ = nullptr;
    }
    
    if (audioClient_)
    {
        audioClient_->Release();
        audioClient_ = nullptr;
    }
    
    if (captureDevice_)
    {
        captureDevice_->Release();
        captureDevice_ = nullptr;
    }
    
    if (deviceEnumerator_)
    {
        deviceEnumerator_->Release();
        deviceEnumerator_ = nullptr;
    }
    
    CoUninitialize();
}

bool SystemAudioCapture::setupAudioClient()
{
    // Get mix format
    HRESULT hr = audioClient_->GetMixFormat(&mixFormat_);
    if (FAILED(hr))
        return false;
    
    // Create event for buffer notifications
    captureEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!captureEvent_)
        return false;
    
    // Initialize audio client for loopback capture
    REFERENCE_TIME bufferDuration = 10000000; // 1 second
    hr = audioClient_->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                 AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                 bufferDuration, 0, mixFormat_, nullptr);
    if (FAILED(hr))
        return false;
    
    // Set event callback
    hr = audioClient_->SetEventHandle(captureEvent_);
    if (FAILED(hr))
        return false;
    
    // Get capture client
    hr = audioClient_->GetService(__uuidof(IAudioCaptureClient), (void**)&captureClient_);
    if (FAILED(hr))
        return false;
    
    // Get volume control
    hr = audioClient_->GetService(__uuidof(ISimpleAudioVolume), (void**)&volumeControl_);
    // Volume control is optional, don't fail if not available
    
    return true;
}

void SystemAudioCapture::captureThreadWASAPI()
{
    // Start the audio client
    HRESULT hr = audioClient_->Start();
    if (FAILED(hr))
    {
        isCapturing_.store(false);
        return;
    }
    
    // Allocate conversion buffers
    const int maxFrames = 4096;
    conversionBuffer_.resize(maxFrames * targetChannels_);
    resampleBuffer_.resize(maxFrames * targetChannels_);
    
    while (!shouldStop_.load())
    {
        // Wait for buffer to be ready
        DWORD waitResult = WaitForSingleObject(captureEvent_, 100);
        if (waitResult != WAIT_OBJECT_0)
            continue;
        
        UINT32 numFramesAvailable = 0;
        hr = captureClient_->GetNextPacketSize(&numFramesAvailable);
        if (FAILED(hr))
            break;
        
        if (numFramesAvailable == 0)
            continue;
        
        // Get the captured data
        BYTE* audioData = nullptr;
        DWORD flags = 0;
        hr = captureClient_->GetBuffer(&audioData, &numFramesAvailable, &flags, nullptr, nullptr);
        if (FAILED(hr))
            break;
        
        if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT) && onAudioDataReceived)
        {
            // Convert to float and resample if necessary
            convertToFloat(audioData, conversionBuffer_.data(), numFramesAvailable,
                          mixFormat_->nChannels, mixFormat_);
            
            // Call the callback with processed audio
            onAudioDataReceived(conversionBuffer_.data(), numFramesAvailable, targetChannels_);
        }
        
        // Release the buffer
        hr = captureClient_->ReleaseBuffer(numFramesAvailable);
        if (FAILED(hr))
            break;
    }
    
    audioClient_->Stop();
    isCapturing_.store(false);
}

std::vector<SystemAudioCapture::CaptureDevice> SystemAudioCapture::getWASAPIDevices()
{
    std::vector<CaptureDevice> devices;
    
    if (!deviceEnumerator_)
        return devices;
    
    IMMDeviceCollection* deviceCollection = nullptr;
    HRESULT hr = deviceEnumerator_->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &deviceCollection);
    if (FAILED(hr))
        return devices;
    
    UINT deviceCount = 0;
    hr = deviceCollection->GetCount(&deviceCount);
    if (FAILED(hr))
    {
        deviceCollection->Release();
        return devices;
    }
    
    // Get default device ID for comparison
    IMMDevice* defaultDevice = nullptr;
    std::string defaultId;
    if (SUCCEEDED(deviceEnumerator_->GetDefaultAudioEndpoint(eRender, eLoopback, &defaultDevice)))
    {
        LPWSTR defaultDeviceId = nullptr;
        if (SUCCEEDED(defaultDevice->GetId(&defaultDeviceId)))
        {
            defaultId = std::string(defaultDeviceId, defaultDeviceId + wcslen(defaultDeviceId));
            CoTaskMemFree(defaultDeviceId);
        }
        defaultDevice->Release();
    }
    
    for (UINT i = 0; i < deviceCount; ++i)
    {
        IMMDevice* device = nullptr;
        hr = deviceCollection->Item(i, &device);
        if (FAILED(hr))
            continue;
        
        CaptureDevice captureDevice;
        
        // Get device ID
        LPWSTR deviceId = nullptr;
        if (SUCCEEDED(device->GetId(&deviceId)))
        {
            captureDevice.id = std::string(deviceId, deviceId + wcslen(deviceId));
            captureDevice.isDefault = (captureDevice.id == defaultId);
            CoTaskMemFree(deviceId);
        }
        
        // Get device name
        IPropertyStore* propertyStore = nullptr;
        if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &propertyStore)))
        {
            PROPVARIANT friendlyName;
            PropVariantInit(&friendlyName);
            if (SUCCEEDED(propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName)))
            {
                captureDevice.name = std::string(friendlyName.pwszVal, 
                                               friendlyName.pwszVal + wcslen(friendlyName.pwszVal));
            }
            PropVariantClear(&friendlyName);
            propertyStore->Release();
        }
        
        devices.push_back(captureDevice);
        device->Release();
    }
    
    deviceCollection->Release();
    return devices;
}

void SystemAudioCapture::convertToFloat(const void* input, float* output, int numSamples,
                                       int inputChannels, WAVEFORMATEX* format)
{
    if (format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
    {
        // Already float format
        const float* floatInput = static_cast<const float*>(input);
        
        if (inputChannels == targetChannels_)
        {
            std::memcpy(output, floatInput, numSamples * inputChannels * sizeof(float));
        }
        else if (inputChannels > targetChannels_)
        {
            // Downmix (simple averaging for stereo)
            for (int i = 0; i < numSamples; ++i)
            {
                for (int ch = 0; ch < targetChannels_; ++ch)
                {
                    output[i * targetChannels_ + ch] = floatInput[i * inputChannels + ch];
                }
            }
        }
        else
        {
            // Upmix (duplicate mono to stereo)
            for (int i = 0; i < numSamples; ++i)
            {
                float monoSample = floatInput[i];
                for (int ch = 0; ch < targetChannels_; ++ch)
                {
                    output[i * targetChannels_ + ch] = monoSample;
                }
            }
        }
    }
    else if (format->wBitsPerSample == 16)
    {
        // 16-bit PCM
        const int16_t* int16Input = static_cast<const int16_t*>(input);
        const float scale = 1.0f / 32768.0f;
        
        for (int i = 0; i < numSamples; ++i)
        {
            if (inputChannels == targetChannels_)
            {
                for (int ch = 0; ch < targetChannels_; ++ch)
                {
                    output[i * targetChannels_ + ch] = int16Input[i * inputChannels + ch] * scale;
                }
            }
            else if (inputChannels > targetChannels_ && targetChannels_ == 2)
            {
                // Downmix to stereo
                float left = int16Input[i * inputChannels] * scale;
                float right = int16Input[i * inputChannels + 1] * scale;
                output[i * 2] = left;
                output[i * 2 + 1] = right;
            }
        }
    }
    else if (format->wBitsPerSample == 32)
    {
        // 32-bit PCM
        const int32_t* int32Input = static_cast<const int32_t*>(input);
        const float scale = 1.0f / 2147483648.0f;
        
        for (int i = 0; i < numSamples; ++i)
        {
            for (int ch = 0; ch < juce::jmin(inputChannels, targetChannels_); ++ch)
            {
                output[i * targetChannels_ + ch] = int32Input[i * inputChannels + ch] * scale;
            }
        }
    }
}

void SystemAudioCapture::resampleAudio(const float* input, float* output,
                                     int inputSamples, int outputSamples, int channels)
{
    // Simple linear interpolation resampling
    float ratio = static_cast<float>(inputSamples) / outputSamples;
    
    for (int i = 0; i < outputSamples; ++i)
    {
        float sourceIndex = i * ratio;
        int index0 = static_cast<int>(sourceIndex);
        int index1 = juce::jmin(index0 + 1, inputSamples - 1);
        float fraction = sourceIndex - index0;
        
        for (int ch = 0; ch < channels; ++ch)
        {
            float sample0 = input[index0 * channels + ch];
            float sample1 = input[index1 * channels + ch];
            output[i * channels + ch] = sample0 + fraction * (sample1 - sample0);
        }
    }
}

#endif // _WIN32

#ifdef __linux__

bool SystemAudioCapture::initializePulseAudio()
{
    pulseSpec_.format = PA_SAMPLE_FLOAT32LE;
    pulseSpec_.channels = targetChannels_;
    pulseSpec_.rate = static_cast<uint32_t>(targetSampleRate_);
    
    int error;
    const char* deviceName = selectedDeviceId_.empty() ? nullptr : selectedDeviceId_.c_str();
    
    pulseStream_ = pa_simple_new(nullptr,               // Server
                                "AudioPracticeLooper",  // Application name
                                PA_STREAM_RECORD,       // Direction
                                deviceName,             // Device
                                "System Audio Capture", // Description
                                &pulseSpec_,           // Sample spec
                                nullptr,               // Channel map
                                nullptr,               // Buffer attributes
                                &error);               // Error code
    
    if (!pulseStream_)
    {
        return false;
    }
    
    return true;
}

void SystemAudioCapture::shutdownPulseAudio()
{
    if (pulseStream_)
    {
        pa_simple_free(pulseStream_);
        pulseStream_ = nullptr;
    }
}

void SystemAudioCapture::captureThreadPulse()
{
    const int bufferSize = 1024;
    conversionBuffer_.resize(bufferSize * targetChannels_);
    
    while (!shouldStop_.load())
    {
        int error;
        int bytesRead = pa_simple_read(pulseStream_, conversionBuffer_.data(),
                                     bufferSize * targetChannels_ * sizeof(float), &error);
        
        if (bytesRead < 0)
        {
            break;
        }
        
        if (onAudioDataReceived)
        {
            int samplesRead = bytesRead / (targetChannels_ * sizeof(float));
            onAudioDataReceived(conversionBuffer_.data(), samplesRead, targetChannels_);
        }
    }
    
    isCapturing_.store(false);
}

std::vector<SystemAudioCapture::CaptureDevice> SystemAudioCapture::getPulseDevices()
{
    std::vector<CaptureDevice> devices;
    
    // This would require more complex PulseAudio introspection API
    // For now, return a basic default device
    CaptureDevice defaultDevice;
    defaultDevice.id = "";
    defaultDevice.name = "Default Monitor";
    defaultDevice.isDefault = true;
    devices.push_back(defaultDevice);
    
    return devices;
}

#endif // __linux__