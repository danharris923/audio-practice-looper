#pragma once

#include <cmath>

template<typename FloatType>
class ParameterSmoother
{
public:
    ParameterSmoother() = default;

    void setSampleRate(double sampleRate)
    {
        sampleRate_ = static_cast<FloatType>(sampleRate);
        updateCoefficients();
    }

    void setSmoothingTimeMs(FloatType smoothingTimeMs)
    {
        smoothingTimeMs_ = smoothingTimeMs;
        updateCoefficients();
    }

    void setTargetValue(FloatType targetValue)
    {
        targetValue_ = targetValue;
    }

    void setCurrentAndTargetValue(FloatType value)
    {
        currentValue_ = targetValue_ = value;
    }

    void skipToTargetValue()
    {
        currentValue_ = targetValue_;
    }

    FloatType getTargetValue() const noexcept
    {
        return targetValue_;
    }

    FloatType getCurrentValue() const noexcept
    {
        return currentValue_;
    }

    FloatType getNextValue() noexcept
    {
        if (currentValue_ == targetValue_)
        {
            return currentValue_;
        }

        currentValue_ += (targetValue_ - currentValue_) * coefficient_;
        
        // Snap to target if very close
        if (std::abs(currentValue_ - targetValue_) < static_cast<FloatType>(1e-6))
        {
            currentValue_ = targetValue_;
        }
        
        return currentValue_;
    }

    bool isSmoothing() const noexcept
    {
        return currentValue_ != targetValue_;
    }

    void reset()
    {
        currentValue_ = targetValue_;
    }

private:
    void updateCoefficients()
    {
        if (sampleRate_ > 0 && smoothingTimeMs_ > 0)
        {
            coefficient_ = static_cast<FloatType>(1.0 - std::exp(-1.0 / (smoothingTimeMs_ * 0.001 * sampleRate_)));
        }
        else
        {
            coefficient_ = static_cast<FloatType>(1.0);
        }
    }

    FloatType sampleRate_ = 44100.0f;
    FloatType smoothingTimeMs_ = 50.0f;
    FloatType coefficient_ = 0.0f;
    FloatType currentValue_ = 0.0f;
    FloatType targetValue_ = 0.0f;
};