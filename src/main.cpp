#include "daisy_patch.h"
#include "daisysp.h"
#include "patch_sampler.h"
#include <string>

using namespace daisy;
using namespace daisysp;

// hardware
static DaisyPatch patch;

// SD card
FatFSInterface DSY_SDRAM_BSS fsi;
SdmmcHandler DSY_SDRAM_BSS   sdcard;

// sample
PatchSampler DSY_SDRAM_BSS sampler;

// parameters
Parameter pitch, cue_start;
double    speed;


// function headers
void SetupSD();
void SetupControls();
void ProcessControls();
void UpdateOLED();


/**
 * @brief Audio callback
 * 
 * @param in audio input
 * @param out audio output
 * @param size block size
 */
static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    ProcessControls();

    for(size_t i = 0; i < size; i++)
    {
        // get interpolated sample
        out[0][i] = sampler.Process();
    }
}

int main(void)
{
    // init patch
    float samplerate;
    patch.Init();
    samplerate = patch.AudioSampleRate();

    // setup
    SetupControls();
    SetupSD();

    // setup sample
    sampler.Init();
    sampler.LoadSample("audio.wav", 0);

    // start controls + audio
    patch.StartAdc();
    patch.StartAudio(AudioCallback);

    // main thread loop
    for(;;)
    {
        UpdateOLED();
    }
}

void SetupControls()
{
    pitch.Init(patch.controls[0], 0.f, 60.f, Parameter::LINEAR);
    cue_start.Init(patch.controls[1], 0., 0.99, Parameter::LINEAR);
}

/**
 * @brief Setup SD card
 * 
 */
void SetupSD()
{
    // configure handler
    SdmmcHandler::Config cfg;
    cfg.Defaults();
    cfg.speed = SdmmcHandler::Speed::STANDARD;

    // initialize handler and filesystem
    sdcard.Init(cfg);
    fsi.Init(FatFSInterface::Config::MEDIA_SD);

    if(f_mount(&fsi.GetSDFileSystem(), fsi.GetSDPath(), 1) != FR_OK)
        return;
}

/**
 * @brief Process ADC controls
 * 
 */
void ProcessControls()
{
    patch.ProcessAnalogControls();
    patch.ProcessDigitalControls();

    // speed
    sampler.SetPitch(pitch.Process());
    // sampler.SetCueStart(cue_start.Process());

    // play state
    if(patch.gate_input[patch.GATE_IN_1].Trig())
        sampler.Retrig();
}

/**
 * @brief Update OLED display
 */
void UpdateOLED()
{
    // clear
    patch.display.Fill(false);

    // header text
    std::string label = "Sampler";
    char       *cstr  = &label[0];
    patch.display.SetCursor(0, 0);
    patch.display.WriteString(cstr, Font_7x10, false);

    // update
    patch.display.Update();
}