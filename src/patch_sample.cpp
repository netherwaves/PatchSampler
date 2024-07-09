#include "patch_sample.h"

namespace daisy
{
PatchSample::PatchSample()
: frame_(0),
  len_(0),
  transpose_(0),
  pitch_(0),
  amp_(1),
  start_pos_(0),
  loop_len_(1),
  load_state_(LoadState::NOT_LOADED),
  mode_(PlayMode::FORWARDS),
  forward_(true)
{
}

PatchSample::Result PatchSample::LoadSample(const char* path)
{
    FRESULT result;
    load_state_ = LoadState::LOADING;

    // get file stats
    result = f_stat(path_, &fno_);
    if(result == FR_OK)
    {
        // open file
        result = f_open(&fil_, path_, (FA_OPEN_EXISTING | FA_READ));
        if(result == FR_OK)
        {
            // read header
            result = f_read(
                &fil_, (void*)&header_, sizeof(WAV_FormatTypeDef), &br_);
            if(result == FR_OK)
            {
                // read sample into buffer in full
                while(!f_eof(&fil_))
                {
                    result = f_read(
                        &fil_, (void*)(buf_ + len_), SECTOR_SIZE, &br_);
                    len_ += br_;
                }
            }

            // when done, close file
            f_close(&fil_);
        }
    }

    // result...
    if(result != FR_OK)
    {
        load_state_ = LoadState::NOT_LOADED;
        return Result::ERR;
    }
    else
    {
        path_       = path;
        load_state_ = LoadState::LOADED;
        return Result::OK;
    }
}

float PatchSample::Process()
{
    // don't process if loading sample
    if(load_state_ != LoadState::LOADED)
        return 0.;

    // get output
    float output = InterpolateSample();

    // move frame forwards or backwards
    if(forward_)
        frame_ += GetSpeed();
    else
        frame_ -= GetSpeed();

    // wrap around or bounce
    if(mode_ == PlayMode::FORWARDS && frame_ > len_)
        frame_ -= len_;
    else if(mode_ == PlayMode::BACKWARDS && frame_ < 0)
        frame_ += len_;

    // return
    return output * amp_;
}

void PatchSample::Retrig()
{
    frame_ = len_ * start_pos_;
}

void PatchSample::SetTranspose(float transpose)
{
    transpose_ = transpose;
}

void PatchSample::SetPitch(float pitch)
{
    pitch_ = pitch;
}

double PatchSample::GetSpeed()
{
    return pow(2., (transpose_ + pitch_) / 12.);
}

float PatchSample::InterpolateSample()
{
    // extract values
    uint32_t index = (int)frame_;
    double   output;
    double   alpha = frame_ - (double)index;

    // set initial output
    output = s162f(buf_[index]);

    // stolen formula but im guessing we're adding in the difference depending on the position
    // so kinda linear interpolation...?
    uint32_t nextSampleIndex = index;
    if(forward_)
        nextSampleIndex++;
    else
        nextSampleIndex--;

    if(alpha > 0.0)
        output += (alpha * (s162f(buf_[(index + 1) % len_]) - output));

    // return
    return output;
}
} // namespace daisy