#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <functional>
#include <memory>
#include <atomic>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <comdef.h>
#endif

#ifdef __linux__
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#endif

class SystemAudioCapture
{
public:
    SystemAudioCapture();
    ~SystemAudioCapture();

    // Initialize the capture system
    bool initialize(double sampleRate = 44100.0, int numChannels = 2);
    void shutdown();

    // Start/stop capture
    bool startCapture();
    void stopCapture();
    bool isCapturing() const { return isCapturing_.load(); }

    // Audio callback - called when new audio data is available
    std::function<void(const float* audioData, int numSamples, int numChannels)> onAudioDataReceived;

    // Get available capture devices
    struct CaptureDevice
    {
        std::string id;
        std::string name;
        bool isDefault = false;
    };
    std::vector<CaptureDevice> getAvailableDevices();
    bool selectDevice(const std::string& deviceId);

    // Volume control
    void setVolume(float volume); // 0.0 to 1.0
    float getVolume() const;

private:
    std::atomic<bool> isCapturing_{false};
    std::atomic<bool> shouldStop_{false};
    
    double targetSampleRate_ = 44100.0;
    int targetChannels_ = 2;
    
    std::unique_ptr<std::thread> captureThread_;
    std::string selectedDeviceId_;
    
#ifdef _WIN32
    // WASAPI members
    IMMDeviceEnumerator* deviceEnumerator_ = nullptr;
    IMMDevice* captureDevice_ = nullptr;
    IAudioClient* audioClient_ = nullptr;
    IAudioCaptureClient* captureClient_ = nullptr;
    ISimpleAudioVolume* volumeControl_ = nullptr;
    WAVEFORMATEX* mixFormat_ = nullptr;
    HANDLE captureEvent_ = nullptr;
    
    // WASAPI helper methods
    bool initializeWASAPI();
    void shutdownWASAPI();
    void captureThreadWASAPI();
    bool setupAudioClient();
    std::vector<CaptureDevice> getWASAPIDevices();
    
    // Format conversion helpers
    void convertToFloat(const void* input, float* output, int numSamples, 
                       int inputChannels, WAVEFORMATEX* format);
    void resampleAudio(const float* input, float* output, 
                      int inputSamples, int outputSamples, int channels);
                      
#endif

#ifdef __linux__
    // PulseAudio members
    pa_simple* pulseStream_ = nullptr;
    pa_sample_spec pulseSpec_;
    
    // PulseAudio helper methods
    bool initializePulseAudio();
    void shutdownPulseAudio();
    void captureThreadPulse();
    std::vector<CaptureDevice> getPulseDevices();
    
#endif

    // Cross-platform buffer management
    std::vector<float> conversionBuffer_;
    std::vector<float> resampleBuffer_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SystemAudioCapture)
};