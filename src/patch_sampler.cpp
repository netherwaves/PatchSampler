#include "patch_sampler.h"

namespace daisy
{
void PatchSampler::Init() {}

PatchSampler::Result PatchSampler::LoadSample(const char* path, size_t idx)
{
    if(idx >= kMaxSamples)
        return Result::ERR;

    return (Result)samples_[idx].LoadSample(path);
}

void PatchSampler::Retrig()
{
    for(size_t i = 0; i < kMaxSamples; i++)
        samples_[i].Retrig();
}

float PatchSampler::Process()
{
    float output = 0.;

    // combine all samples
    for(size_t i = 0; i < kMaxSamples; i++)
        output += samples_[i].Process();

    // return
    return output;
}

void PatchSampler::SetTranspose(float transpose, size_t idx)
{
    if(idx >= kMaxSamples)
        return;

    samples_[idx].SetTranspose(transpose);
}

void PatchSampler::SetPitch(float pitch)
{
    pitch_ = pitch;

    for(size_t i = 0; i < kMaxSamples; i++)
        samples_[i].SetPitch(pitch);
}
} // namespace daisy