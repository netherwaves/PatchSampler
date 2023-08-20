#include "daisy_patch.h"
#include "daisysp.h"
#include <string>

using namespace daisy;
using namespace daisysp;

// hardware
static DaisyPatch patch;

// SD card
FatFSInterface DSY_SDRAM_BSS fsi;
SdmmcHandler DSY_SDRAM_BSS   sdcard;
FIL DSY_SDRAM_BSS            fil;
DIR DSY_SDRAM_BSS            dir;
FILINFO DSY_SDRAM_BSS        fno;

// parameters
Parameter pitch;

// sampler
int16_t DSY_SDRAM_BSS sample[48000 * 15]; // 16-bit, mono, 48khz --> 2 seconds
uint32_t              sample_len = 0;
double                sample_ptr = 0;
bool                  is_playing = false;
WAV_FormatTypeDef     header;

// function headers
void  SetupSD();
void  SetupControls();
void  ProcessControls();
void  UpdateOLED();
float InterpolateSample();

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
        out[0][i] = InterpolateSample();

        double speed = pow(2., pitch.Process() / 12.f);

        // move pointer (wrap-around)
        sample_ptr = fmod(sample_ptr + speed, sample_len);
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

    // start audio
    patch.StartAdc();
    patch.StartAudio(AudioCallback);

    // do this once

    // main thread loop
    for(;;)
    {
        UpdateOLED();
    }
}

void SetupControls()
{
    pitch.Init(patch.controls[0], -12.f, 48.f, Parameter::LINEAR);
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
    f_mount(&fsi.GetSDFileSystem(), fsi.GetSDPath(), 1);

    // prepare read
    UINT bytesread = 0;
    sample_len     = 0;

    if(f_stat("audio.wav", &fno) != FR_OK)
        asm("bkpt 255");

    // open test file
    if(f_open(&fil, "audio.wav", (FA_OPEN_EXISTING | FA_READ)) != FR_OK)
        asm("bkpt 255");

    // read header
    if(f_read(&fil, (void *)&header, sizeof(WAV_FormatTypeDef), &bytesread)
       != FR_OK)
        asm("bkpt 255");

    // read into buffer
    while(!f_eof(&fil))
    {
        if(f_read(&fil, (void *)(sample + sample_len), 65536, &bytesread)
           != FR_OK)
            asm("bkpt 255");

        sample_len += (bytesread / 2);
    }

    // close file
    f_close(&fil);
}

/**
 * @brief Process ADC controls
 * 
 */
void ProcessControls()
{
    patch.ProcessAnalogControls();
    patch.ProcessDigitalControls();

    // play state
    if(patch.gate_input[patch.GATE_IN_1].Trig())
        sample_ptr = 0;
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

    FixedCapStr<6, char> val;
    val.AppendFloat(pitch.Value());

    // file name
    label = "pitch: ";
    label += val.Cstr();
    patch.display.SetCursor(0, 12);
    patch.display.WriteString(cstr, Font_7x10, true);

    // update
    patch.display.Update();
}

float InterpolateSample()
{
    // extract values
    uint32_t index = (int)sample_ptr;
    double   output;
    double   alpha = sample_ptr - (double)index;

    // set initial output
    double gain = 1.0 / 37268.0;
    output      = sample[index] * gain;

    // stolen formula but im guessing we're adding in the difference depending on the position
    // so kinda linear interpolation...?
    if(alpha > 0.0)
        output += (alpha * (sample[(index + 1) % sample_len] * gain - output));

    // return
    return output;
}