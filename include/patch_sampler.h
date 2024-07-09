#pragma once

#include "daisy.h"
#include "patch_sample.h"

namespace daisy
{
class PatchSampler
{
  public:
    enum PlayMode
    {
        FORWARDS = 0,
        BACKWARDS,
        ALTERNATE,
        LAST
    };
    enum Result
    {
        OK = 0,
        ERR
    };

    void   Init();
    Result LoadSample(const char* path, size_t index);
    void   Retrig();
    float  Process();

    // -= list of parameters =-
    // TODO: enable multisample mode
    // pitch (1V/oct) 	  [CV]
    // start point (%) 	  [CV]
    // length (%) 		  [CV]
    // FM amplitude 	  [CV]
    // base pitch		  [GLOBAL]	<-- per sample?
    // mode 			  [GLOBAL]	<-- per sample?
    // FM waveform 		  [GLOBAL]	<-- per sample?
    // RM input (2) 	  [AUDIO]

    void SetTranspose(float transpose, size_t idx);
    void SetPitch(float pitch);

  private:
    static constexpr size_t kMaxSamples = 4;
    PatchSample             samples_[kMaxSamples];

    // parameters
    float    pitch_;
    bool     looping_;
    float    cue_start_;
    PlayMode mode_;
    bool     forward_;
};
} // namespace daisy