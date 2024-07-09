#pragma once

#include "daisy.h"

namespace daisy
{
#define SECTOR_SIZE 65536

class PatchSample
{
  public:
    enum PlayMode
    {
        FORWARDS = 0,
        BACKWARDS,
        ALTERNATE,
        LAST
    };
    enum LoadState
    {
        NOT_LOADED = 0,
        LOADING,
        LOADED
    };
    enum Result
    {
        OK = 0,
        ERR
    };

    PatchSample();
    Result LoadSample(const char* path);
    float  Process();
    void   Retrig();

    // setters
    void SetTranspose(float transpose);
    void SetPitch(float pitch);

    // getters
    double GetSpeed();

  private:
    float InterpolateSample();

    // playback
    const char*       path_;
    double            frame_;
    int16_t           buf_[48000 * 10];
    uint32_t          len_;
    WAV_FormatTypeDef header_;
    bool              forward_;

    // parameters
    PlayMode mode_;
    double   pitch_, transpose_;
    double   amp_;
    double   start_pos_;
    double   loop_len_;

    // FS props
    LoadState load_state_;
    FIL       fil_;
    FILINFO   fno_;
    UINT      br_;
};
} // namespace daisy