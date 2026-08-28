#include "yayu_engine.h"

#include <algorithm>
#include <cmath>

namespace yayu
{
namespace
{
constexpr float kEqFrequencyHz = 900.0f;
constexpr float kEqResonance = 0.2f;
}

void Engine::Init(const float sample_rate,
                  daisysp::ReverbSc* feedback_reverb,
                  daisysp::ReverbSc* post_reverb)
{
    sample_rate_ = sample_rate;
    feedback_reverb_ = feedback_reverb;
    post_reverb_ = post_reverb;
    noise_.Init();

    for(size_t channel = 0; channel < 2; ++channel)
    {
        strings_[channel].Init(sample_rate_);
        feedback_delay_[channel].Init();

        for(size_t chain = 0; chain < 2; ++chain)
        {
            for(size_t slot = 0; slot < 2; ++slot)
            {
                tone_eq_[chain][slot][channel].Init(sample_rate_);
                tone_eq_[chain][slot][channel].SetFreq(kEqFrequencyHz);
                tone_eq_[chain][slot][channel].SetRes(kEqResonance);
                tone_eq_[chain][slot][channel].SetDrive(0.0f);

                overdrive_[chain][slot][channel].Init();
                bitcrush_[chain][slot][channel].Init(sample_rate_);
            }
        }
    }

    if(feedback_reverb_ != nullptr)
    {
        feedback_reverb_->Init(sample_rate_);
    }
    if(post_reverb_ != nullptr)
    {
        post_reverb_->Init(sample_rate_);
    }

    SetParams(params_);
}

void Engine::SetParams(const Params& params)
{
    params_ = params;

    // One ReverbSc instance is allocated for each chain. Keep a second Reverb
    // selection in the same chain as a no-op until another instance is added.
    EffectSlot* chains[] = {params_.feedback_fx, params_.post_fx};
    for(auto* chain_slots : chains)
    {
        bool has_reverb = false;
        for(size_t slot_index = 0; slot_index < 2; ++slot_index)
        {
            auto& slot = chain_slots[slot_index];
            if(slot.type == EffectType::Reverb && has_reverb)
            {
                slot.type = EffectType::Bypass;
            }
            has_reverb = has_reverb || slot.type == EffectType::Reverb;
        }
    }

    const auto string_frequency = daisysp::mtof(params_.model_note);

    noise_.SetAmp(std::max(0.0f, params_.seed_level));
    for(size_t channel = 0; channel < 2; ++channel)
    {
        strings_[channel].SetFreq(string_frequency);
        for(size_t chain = 0; chain < 2; ++chain)
        {
            const auto* chain_slots = chain == 0 ? params_.feedback_fx : params_.post_fx;
            for(size_t slot = 0; slot < 2; ++slot)
            {
                const auto& tone = chain_slots[slot].tone;
                overdrive_[chain][slot][channel].SetDrive(std::clamp(tone.drive, 0.0f, 1.0f));
                bitcrush_[chain][slot][channel].SetBitDepth(
                    std::max(1, 16 - static_cast<int>(tone.drive * 15.0f)));
                bitcrush_[chain][slot][channel].SetCrushRate(
                    std::max(100.0f, sample_rate_ * (1.0f - tone.drive * 0.95f)));
            }
        }
    }

    const auto configure_reverb = [this](daisysp::ReverbSc* reverb, const EffectSlot* slots) {
        if(reverb == nullptr)
        {
            return;
        }
        for(size_t slot = 0; slot < 2; ++slot)
        {
            if(slots[slot].type == EffectType::Reverb)
            {
                reverb->SetFeedback(std::clamp(slots[slot].reverb.feedback, 0.0f, 0.99f));
                reverb->SetLpFreq(
                    std::clamp(slots[slot].reverb.lpf_hz, 100.0f, sample_rate_ * 0.45f));
                return;
            }
        }
    };
    if(feedback_reverb_ != nullptr)
    {
        configure_reverb(feedback_reverb_, params_.feedback_fx);
    }
    if(post_reverb_ != nullptr)
    {
        configure_reverb(post_reverb_, params_.post_fx);
    }
}

void Engine::Process(float in_l, float in_r, float& out_l, float& out_r)
{
    const auto delay_samples = std::clamp(params_.feedback_delay_s * sample_rate_,
                                          1.0f,
                                          static_cast<float>(kMaxFeedbackDelaySamples - 1));
    const float returned_l = feedback_delay_[0].Read(delay_samples);
    const float returned_r = feedback_delay_[1].Read(delay_samples);
    const float seed = noise_.Process();

    const float model_l = strings_[0].Process(returned_l + seed + in_l * params_.input_excitation);
    const float model_r = strings_[1].Process(returned_r + seed + in_r * params_.input_excitation);

    float source_l = in_l;
    float source_r = in_r;
    switch(params_.source_mode)
    {
        case SourceMode::DirectInput:
            source_l += returned_l;
            source_r += returned_r;
            break;
        case SourceMode::KarplusOnly:
            source_l = model_l;
            source_r = model_r;
            break;
        case SourceMode::InputExcitesKarplus:
            source_l = model_l;
            source_r = model_r;
            break;
        case SourceMode::InputPlusKarplus:
            source_l = in_l * (1.0f - params_.model_mix) + model_l * params_.model_mix;
            source_r = in_r * (1.0f - params_.model_mix) + model_r * params_.model_mix;
            break;
    }

    float feedback_l = source_l;
    float feedback_r = source_r;
    for(size_t slot = 0; slot < 2; ++slot)
    {
        ProcessEffect(params_.feedback_fx[slot],
                      0,
                      slot,
                      feedback_l,
                      feedback_r,
                      feedback_l,
                      feedback_r);
    }

    feedback_delay_[0].Write(ProcessLoopProtection(feedback_l) * params_.feedback_gain);
    feedback_delay_[1].Write(ProcessLoopProtection(feedback_r) * params_.feedback_gain);

    float post_l = feedback_l;
    float post_r = feedback_r;
    for(size_t slot = 0; slot < 2; ++slot)
    {
        ProcessEffect(params_.post_fx[slot], 1, slot, post_l, post_r, post_l, post_r);
    }

    out_l = post_l * params_.output_gain;
    out_r = post_r * params_.output_gain;
}

void Engine::ProcessEffect(const EffectSlot& slot,
                           const size_t chain,
                           const size_t slot_index,
                           const float input_l,
                           const float input_r,
                           float& output_l,
                           float& output_r)
{
    const float mix = std::clamp(slot.mix, 0.0f, 1.0f);
    float wet_l = input_l;
    float wet_r = input_r;

    switch(slot.type)
    {
        case EffectType::ToneDrive:
            wet_l = ProcessTone(input_l, slot.tone, chain, slot_index, 0);
            wet_r = ProcessTone(input_r, slot.tone, chain, slot_index, 1);
            break;
        case EffectType::Reverb:
        {
            auto* reverb = chain == 0 ? feedback_reverb_ : post_reverb_;
            if(reverb != nullptr)
            {
                reverb->Process(input_l, input_r, &wet_l, &wet_r);
            }
            break;
        }
        case EffectType::Bypass:
        case EffectType::Delay:
        case EffectType::Flanger:
            break;
    }

    output_l = input_l + (wet_l - input_l) * mix;
    output_r = input_r + (wet_r - input_r) * mix;
}

float Engine::ProcessTone(const float input,
                          const ToneDriveParams& tone,
                          const size_t chain,
                          const size_t slot_index,
                          const size_t channel)
{
    const float gained = input * tone.gain;
    const bool flat_eq = std::fabs(tone.low_gain_db) < 0.001f
                         && std::fabs(tone.mid_gain_db) < 0.001f
                         && std::fabs(tone.high_gain_db) < 0.001f;
    float equalized = gained;
    tone_eq_[chain][slot_index][channel].Process(gained);
    if(!flat_eq)
    {
        equalized = tone_eq_[chain][slot_index][channel].Low() * DbToLinear(tone.low_gain_db)
                    + tone_eq_[chain][slot_index][channel].Band() * DbToLinear(tone.mid_gain_db)
                    + tone_eq_[chain][slot_index][channel].High() * DbToLinear(tone.high_gain_db);
    }
    return ProcessDrive(equalized, tone, chain, slot_index, channel);
}

float Engine::ProcessDrive(const float input,
                           const ToneDriveParams& tone,
                           const size_t chain,
                           const size_t slot_index,
                           const size_t channel)
{
    switch(tone.drive_mode)
    {
        case DriveMode::Clean:
            return input;
        case DriveMode::Overdrive:
            return overdrive_[chain][slot_index][channel].Process(input);
        case DriveMode::Distortion:
            return std::tanh(input * (1.0f + tone.drive * 20.0f));
        case DriveMode::Fuzz:
            return std::tanh(std::tanh(input * (2.0f + tone.drive * 30.0f)) * 3.0f);
        case DriveMode::BitCrush:
            return bitcrush_[chain][slot_index][channel].Process(input);
    }
    return input;
}

float Engine::ProcessLoopProtection(const float input) const
{
    const float amount = std::max(1.0f, params_.loop_saturation);
    return std::tanh(input * amount) / amount;
}

float Engine::DbToLinear(const float db) const
{
    return std::pow(10.0f, db * 0.05f);
}
} // namespace yayu
