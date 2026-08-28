#pragma once

#include <cstdint>

#include <daisysp.h>

#include "models/karplus_string.h"

namespace yayu
{
enum class SourceMode : uint8_t
{
    DirectInput,
    KarplusOnly,
    InputExcitesKarplus,
    InputPlusKarplus,
};

enum class DriveMode : uint8_t
{
    Clean,
    Overdrive,
    Distortion,
    Fuzz,
    BitCrush,
};

// Each chain has two serial slots. Delay and Flanger are interface placeholders
// until their DSP implementations are added.
enum class EffectType : uint8_t
{
    Bypass,
    ToneDrive,
    Delay,
    Flanger,
};

enum class ReverbRoute : uint8_t
{
    Feedback,
    Post,
};

struct ToneDriveParams
{
    DriveMode drive_mode {DriveMode::Clean};
    float     gain {1.0f};
    float     drive {0.0f};
    float     low_gain_db {0.0f};
    float     mid_gain_db {0.0f};
    float     high_gain_db {0.0f};
};

struct ReverbParams
{
    bool        enabled {false};
    ReverbRoute route {ReverbRoute::Feedback};
    float       mix {0.5f};
    float feedback {0.6f};
    float lpf_hz {12000.0f};
};

struct EffectSlot
{
    EffectType      type {EffectType::Bypass};
    float           mix {1.0f};
    ToneDriveParams tone {};
};

struct Params
{
    SourceMode source_mode {SourceMode::DirectInput};

    float model_note {40.0f};
    float model_mix {0.5f};
    float input_excitation {1.0f};
    float seed_level {0.0f};

    // Source -> feedback effects -> feedback return -> post effects -> limiter.
    EffectSlot feedback_fx[2] {{EffectType::ToneDrive}, {EffectType::Bypass}};
    EffectSlot post_fx[2] {{EffectType::Bypass}, {EffectType::Bypass}};
    ReverbParams reverb {};

    float feedback_gain {0.0f};
    float feedback_delay_s {0.003f};
    float loop_saturation {1.0f};

    float output_gain {0.7f};
};

class Engine
{
  public:
    void Init(float sample_rate, daisysp::ReverbSc* reverb);
    void SetParams(const Params& params);
    void Process(float in_l, float in_r, float& out_l, float& out_r);

  private:
    static constexpr size_t kMaxFeedbackDelaySamples = 8192;

    void ProcessEffect(const EffectSlot& slot,
                       size_t chain,
                       size_t slot_index,
                       float input_l,
                       float input_r,
                       float& output_l,
                       float& output_r);
    void ProcessReverb(float input_l,
                       float input_r,
                       float& output_l,
                       float& output_r);
    float ProcessTone(float input,
                      const ToneDriveParams& tone,
                      size_t chain,
                      size_t slot_index,
                      size_t channel);
    float ProcessDrive(float input,
                       const ToneDriveParams& tone,
                       size_t chain,
                       size_t slot_index,
                       size_t channel);
    float ProcessLoopProtection(float input) const;
    float DbToLinear(float db) const;

    float sample_rate_ {48000.0f};
    Params params_ {};

    infrasonic::KarplusString strings_[2];
    daisysp::WhiteNoise noise_;
    daisysp::DelayLine<float, kMaxFeedbackDelaySamples> feedback_delay_[2];
    daisysp::Svf tone_eq_[2][2][2];
    daisysp::Overdrive overdrive_[2][2][2];
    daisysp::Bitcrush bitcrush_[2][2][2];
    daisysp::ReverbSc* reverb_ {nullptr};
};
} // namespace yayu
