#include "daisy_patch.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

static DaisyPatch patch;
Oscillator        osc;

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
    for(size_t i = 0; i < size; i++)
    {
        float val = osc.Process();
        for(int j = 0; j < 4; j++)
        {
            out[j][i] = val;
        }
    }
}

void UpdateOLED();

int main(void)
{
    // init patch
    float samplerate;
    patch.Init();
    samplerate = patch.AudioSampleRate();

    // setup oscillator
    osc.Init(samplerate);
    osc.SetFreq(1000);
    osc.SetAmp(1);
    osc.SetWaveform(Oscillator::WAVE_SIN);

    // start audio
    patch.StartAudio(AudioCallback);
    patch.seed.SetLed(1);

    // main thread loop
    for(;;)
    {
        UpdateOLED();
    }
}

void UpdateOLED()
{
    // clear
    patch.display.Fill(false);

    // display text
    std::string str  = "Sampler";
    const char* cstr = &str[0];
    patch.display.SetCursor(0, 0);
    patch.display.WriteString(cstr, Font_7x10, true);

    // update
    patch.display.Update();
}