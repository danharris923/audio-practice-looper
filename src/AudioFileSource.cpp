#include "AudioFileSource.h"

// Include FFmpeg headers with C linkage
extern "C" 
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libswresample/swresample.h>
}

AudioFileSource::AudioFileSource() = default;

AudioFileSource::~AudioFileSource()
{
    closeFile();
}

bool AudioFileSource::initializeFFmpeg()
{
    // Initialize FFmpeg (only needed once globally, but safe to call multiple times)
    av_register_all();
    avcodec_register_all();
    return true;
}

bool AudioFileSource::loadFile(const juce::File& file)
{
    closeFile(); // Close any previously loaded file
    
    if (!file.exists())
        return false;

    initializeFFmpeg();

    // Open input file
    const char* filename = file.getFullPathName().toRawUTF8();
    int ret = avformat_open_input(&formatContext, filename, nullptr, nullptr);
    if (ret < 0)
    {
        juce::Logger::writeToLog("Failed to open input file: " + file.getFileName());
        return false;
    }

    // Retrieve stream information
    ret = avformat_find_stream_info(formatContext, nullptr);
    if (ret < 0)
    {
        juce::Logger::writeToLog("Failed to find stream info");
        cleanupFFmpeg();
        return false;
    }

    // Find audio stream
    audioStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++)
    {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1)
    {
        juce::Logger::writeToLog("No audio stream found");
        cleanupFFmpeg();
        return false;
    }

    if (!openCodec())
    {
        cleanupFFmpeg();
        return false;
    }

    if (!setupResampler())
    {
        cleanupFFmpeg();
        return false;
    }

    // Get audio properties
    AVStream* audioStream = formatContext->streams[audioStreamIndex];
    sampleRate = codecContext->sample_rate;
    numChannels = codecContext->channels;
    
    // Calculate total duration
    if (audioStream->duration != AV_NOPTS_VALUE)
    {
        totalDurationSeconds = audioStream->duration * av_q2d(audioStream->time_base);
    }
    else if (formatContext->duration != AV_NOPTS_VALUE)
    {
        totalDurationSeconds = formatContext->duration / static_cast<double>(AV_TIME_BASE);
    }
    else
    {
        totalDurationSeconds = 0.0;
    }

    // Allocate frame and packet
    frame = av_frame_alloc();
    packet = av_packet_alloc();

    if (!frame || !packet)
    {
        juce::Logger::writeToLog("Failed to allocate frame or packet");
        cleanupFFmpeg();
        return false;
    }

    fileLoaded.store(true);
    currentPositionSeconds.store(0.0);
    
    juce::Logger::writeToLog("Successfully loaded: " + file.getFileName() + 
                            " (" + juce::String(totalDurationSeconds, 2) + "s, " +
                            juce::String(static_cast<int>(sampleRate)) + "Hz, " +
                            juce::String(numChannels) + " channels)");
    
    return true;
}

bool AudioFileSource::openCodec()
{
    AVCodecParameters* codecParams = formatContext->streams[audioStreamIndex]->codecpar;
    
    // Find decoder
    AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec)
    {
        juce::Logger::writeToLog("Unsupported codec");
        return false;
    }

    // Allocate codec context
    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        juce::Logger::writeToLog("Failed to allocate codec context");
        return false;
    }

    // Copy codec parameters to context
    int ret = avcodec_parameters_to_context(codecContext, codecParams);
    if (ret < 0)
    {
        juce::Logger::writeToLog("Failed to copy codec parameters");
        return false;
    }

    // Open codec
    ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0)
    {
        juce::Logger::writeToLog("Failed to open codec");
        return false;
    }

    return true;
}

bool AudioFileSource::setupResampler()
{
    // Setup resampler to convert to stereo float
    swrContext = swr_alloc_set_opts(nullptr,
                                   AV_CH_LAYOUT_STEREO,     // Output channel layout
                                   AV_SAMPLE_FMT_FLT,       // Output sample format
                                   static_cast<int>(sampleRate), // Output sample rate
                                   codecContext->channel_layout ? codecContext->channel_layout : 
                                   (codecContext->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO),
                                   codecContext->sample_fmt,     // Input sample format
                                   codecContext->sample_rate,    // Input sample rate
                                   0, nullptr);

    if (!swrContext)
    {
        juce::Logger::writeToLog("Failed to allocate resampler context");
        return false;
    }

    int ret = swr_init(swrContext);
    if (ret < 0)
    {
        juce::Logger::writeToLog("Failed to initialize resampler");
        return false;
    }

    return true;
}

void AudioFileSource::closeFile()
{
    fileLoaded.store(false);
    cleanupFFmpeg();
}

void AudioFileSource::cleanupFFmpeg()
{
    if (frame)
    {
        av_frame_free(&frame);
        frame = nullptr;
    }

    if (packet)
    {
        av_packet_free(&packet);
        packet = nullptr;
    }

    if (swrContext)
    {
        swr_free(&swrContext);
        swrContext = nullptr;
    }

    if (codecContext)
    {
        avcodec_free_context(&codecContext);
        codecContext = nullptr;
    }

    if (formatContext)
    {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
    }

    audioStreamIndex = -1;
    totalDurationSeconds = 0.0;
    currentPositionSeconds.store(0.0);
}

bool AudioFileSource::isFileLoaded() const
{
    return fileLoaded.load();
}

void AudioFileSource::setPlaybackPosition(double positionInSeconds)
{
    if (isFileLoaded())
    {
        currentPositionSeconds.store(juce::jlimit(0.0, totalDurationSeconds, positionInSeconds));
        seekToPosition(currentPositionSeconds.load());
    }
}

double AudioFileSource::getCurrentPosition() const
{
    return currentPositionSeconds.load();
}

double AudioFileSource::getTotalLength() const
{
    return totalDurationSeconds;
}

void AudioFileSource::setLoopPoints(double startSeconds, double endSeconds)
{
    loopStartSeconds.store(juce::jlimit(0.0, totalDurationSeconds, startSeconds));
    loopEndSeconds.store(juce::jlimit(startSeconds, totalDurationSeconds, endSeconds));
}

void AudioFileSource::setLoopEnabled(bool enabled)
{
    loopEnabled.store(enabled);
}

void AudioFileSource::seekToPosition(double seconds)
{
    if (!isFileLoaded() || !formatContext || audioStreamIndex < 0)
        return;

    AVStream* audioStream = formatContext->streams[audioStreamIndex];
    int64_t timestamp = seconds / av_q2d(audioStream->time_base);
    
    int ret = av_seek_frame(formatContext, audioStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
    if (ret >= 0)
    {
        avcodec_flush_buffers(codecContext);
    }
}

int AudioFileSource::readNextFrame(float* outputBuffer, int numSamplesToRead)
{
    if (!isFileLoaded() || !formatContext || !codecContext)
        return 0;

    int samplesDecoded = 0;
    
    while (samplesDecoded < numSamplesToRead)
    {
        int ret = av_read_frame(formatContext, packet);
        if (ret < 0)
        {
            // End of file or error
            break;
        }

        if (packet->stream_index != audioStreamIndex)
        {
            av_packet_unref(packet);
            continue;
        }

        ret = avcodec_send_packet(codecContext, packet);
        if (ret < 0)
        {
            av_packet_unref(packet);
            continue;
        }

        while (ret >= 0)
        {
            ret = avcodec_receive_frame(codecContext, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            
            if (ret < 0)
                break;

            // Resample to stereo float
            float* resampledData[2] = { 
                outputBuffer + samplesDecoded * 2,       // Left channel
                outputBuffer + samplesDecoded * 2 + 1    // Right channel (interleaved)
            };

            int outputSamples = swr_convert(swrContext, 
                                          (uint8_t**)resampledData, 
                                          numSamplesToRead - samplesDecoded,
                                          (const uint8_t**)frame->data, 
                                          frame->nb_samples);

            if (outputSamples > 0)
            {
                samplesDecoded += outputSamples;
            }
        }

        av_packet_unref(packet);
    }

    return samplesDecoded;
}

// AudioProcessor implementation
const juce::String AudioFileSource::getName() const
{
    return "AudioFileSource";
}

void AudioFileSource::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
    // Sample rate conversion is handled by FFmpeg resampler
}

void AudioFileSource::releaseResources()
{
    // Resources are managed in closeFile()
}

void AudioFileSource::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    buffer.clear();

    if (!isFileLoaded())
        return;

    // Interleaved buffer for FFmpeg
    std::vector<float> interleavedBuffer(numSamples * 2);
    
    int samplesRead = readNextFrame(interleavedBuffer.data(), numSamples);
    
    // Convert from interleaved to JUCE buffer format
    for (int channel = 0; channel < juce::jmin(numChannels, 2); ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < samplesRead; ++sample)
        {
            channelData[sample] = interleavedBuffer[sample * 2 + channel];
        }
    }
    
    // Update position
    double samplesAdvanced = static_cast<double>(samplesRead) / sampleRate;
    double newPosition = currentPositionSeconds.load() + samplesAdvanced;
    
    // Handle looping
    if (loopEnabled.load())
    {
        double loopStart = loopStartSeconds.load();
        double loopEnd = loopEndSeconds.load();
        
        if (newPosition >= loopEnd && loopEnd > loopStart)
        {
            newPosition = loopStart;
            seekToPosition(newPosition);
        }
    }
    
    currentPositionSeconds.store(newPosition);
}