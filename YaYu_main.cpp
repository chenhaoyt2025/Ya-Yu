#include <daisy_seed.h>
#include <daisysp.h>

#include "src/yayu_engine.h"

namespace
{
constexpr auto kSampleRate = daisy::SaiHandle::Config::SampleRate::SAI_48KHZ;
constexpr size_t kBlockSize = 4;

daisy::DaisySeed hw;
yayu::Engine engine;
yayu::Params params;
daisysp::Limiter master_limiter[2];

// The shared reverb can be routed inside or after the feedback loop.
daisysp::ReverbSc DSY_SDRAM_BSS reverb;

void AudioCallback(daisy::AudioHandle::InputBuffer in,
                   daisy::AudioHandle::OutputBuffer out,
                   const size_t size)
{
    for(size_t sample = 0; sample < size; ++sample)
    {
        engine.Process(in[0][sample], in[1][sample], out[0][sample], out[1][sample]);
    }

    // This is output safety only. Feedback stability is handled inside Engine.
    master_limiter[0].ProcessBlock(out[0], size, 0.7f);
    master_limiter[1].ProcessBlock(out[1], size, 0.7f);
}
} // namespace

int main()
{
    hw.Init();
    hw.SetAudioSampleRate(kSampleRate);
    hw.SetAudioBlockSize(kBlockSize);

    engine.Init(hw.AudioSampleRate(), &reverb);
    engine.SetParams(params);

    for(auto& limiter : master_limiter)
    {
        limiter.Init();
    }

    hw.StartAudio(AudioCallback);
    while(true) {}
}
