/* 
 * Stereo Delay
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#include <circle/logger.h>
#include "effect_delay.h"

LOGMODULE ("fx delay");

AudioEffectDelay::AudioEffectDelay(float32_t samplerate) : AudioEffect(samplerate)
{   
    this->bufferSize = (int) samplerate * MAX_DELAY_TIME;
    this->bufferL = new float32_t[this->bufferSize];
    this->bufferR = new float32_t[this->bufferSize];
    this->index = 0;
    this->lpf = new AudioEffectLPF(samplerate);
    this->lpf->setParameter(AudioEffectLPF::Param::CUTOFF, 80);
    this->lpf->setParameter(AudioEffectLPF::Param::RESONANCE, 0);

    // Clean buffers
    memset(this->bufferL, 0, this->bufferSize * sizeof(float32_t));
    memset(this->bufferR, 0, this->bufferSize * sizeof(float32_t));
    
    this->setParameter(AudioEffectDelay::Param::TIME_L, 360);
    this->setParameter(AudioEffectDelay::Param::TIME_R, 360);
    this->setParameter(AudioEffectDelay::Param::FEEDBACK, 60);
    this->pingPongMode = false;
    this->setMix(0.5f);
}

AudioEffectDelay::~AudioEffectDelay()
{
    delete this->bufferL;
    delete this->bufferR;
    delete this->lpf;
}

unsigned AudioEffectDelay::getId()
{
    return EFFECT_DELAY;
}

void AudioEffectDelay::initializeSendFX()
{
    this->setParameter(AudioEffectDelay::Param::MIX, 100);
}

void AudioEffectDelay::setTempo(unsigned tempo)
{
    this->tempo = (float32_t) tempo;
    this->setParameter(AudioEffectDelay::Param::TIME_L, timeLValue);
    this->setParameter(AudioEffectDelay::Param::TIME_R, timeRValue);
}

void AudioEffectDelay::setParameter(unsigned param, unsigned value)
{
    switch (param)
    {
    case AudioEffectDelay::Param::BYPASS:
        this->setBypass(value == 1);
        break;
    case AudioEffectDelay::Param::TIME_L:
        this->timeLValue = value;
        this->timeL = this->calculateTime(value);
        break;
    case AudioEffectDelay::Param::TIME_R:
        this->timeRValue = value;
        this->timeR = this->calculateTime(value);
        break;
    case AudioEffectDelay::Param::FEEDBACK:
        this->feedback = (float32_t) value / 100.0f;
        break;
    case AudioEffectDelay::Param::TONE:
        this->lpf->setParameter(AudioEffectLPF::Param::CUTOFF, value);
        break;
    case AudioEffectDelay::Param::PING_PONG:
        this->pingPongMode = (value == 1);
        break;
    case AudioEffectDelay::Param::MIX:
        this->setMix((float32_t) value / 100.0f);
        break;
    default:
        break;
    }
}

unsigned AudioEffectDelay::getParameter(unsigned param)
{
    switch (param)
    {
    case AudioEffectDelay::Param::BYPASS:
		return this->getBypass() ? 1 : 0;
    case AudioEffectDelay::Param::TIME_L:
        return this->timeLValue;
    case AudioEffectDelay::Param::TIME_R:
        return this->timeRValue;
    case AudioEffectDelay::Param::FEEDBACK:
        return roundf(this->feedback * 100);
    case AudioEffectDelay::Param::TONE:
        return this->lpf->getParameter(AudioEffectLPF::Param::CUTOFF);
    case AudioEffectDelay::Param::PING_PONG:
        return this->pingPongMode ? 1 : 0;
    case AudioEffectDelay::Param::MIX:
        return roundf(this->mix * 100);
    default:
        return 0;
    }
}

float32_t AudioEffectDelay::calculateTime(unsigned value)
{
    if (value < AudioEffectDelay::MAX_DELAY_TIME * 1000)
    {
        return (float32_t) value / 1000.0f;
    }
    float32_t numerator;
    float32_t denominator;
    switch (value - AudioEffectDelay::MAX_DELAY_TIME * 1000)
	{
	case AudioEffectDelay::SyncTime::T_1_32:
        numerator = 1.0f;
        denominator = 32.0f;
        break;
	case AudioEffectDelay::SyncTime::T_1_24:
		numerator = 1.0f;
        denominator = 24.0f;
        break;
	case AudioEffectDelay::SyncTime::T_1_16:
		numerator = 1.0f;
        denominator = 16.0f;
        break;
	case AudioEffectDelay::SyncTime::T_1_12:
		numerator = 1.0f;
        denominator = 12.0f;
        break;
	case AudioEffectDelay::SyncTime::T_3_32:
		numerator = 3.0f;
        denominator = 32.0f;
        break;
	case AudioEffectDelay::SyncTime::T_1_8:
		numerator = 1.0f;
        denominator = 8.0f;
        break;
	case AudioEffectDelay::SyncTime::T_1_6:
		numerator = 1.0f;
        denominator = 6.0f;
        break;
	case AudioEffectDelay::SyncTime::T_3_16:
		numerator = 3.0f;
        denominator = 16.0f;
        break;
	case AudioEffectDelay::SyncTime::T_1_4:
		numerator = 1.0f;
        denominator = 4.0f;
        break;
	case AudioEffectDelay::SyncTime::T_1_3:
		numerator = 1.0f;
        denominator = 3.0f;
        break;
	case AudioEffectDelay::SyncTime::T_3_8:
		numerator = 3.0f;
        denominator = 8.0f;
        break;
	case AudioEffectDelay::SyncTime::T_1_2:
		numerator = 1.0f;
        denominator = 2.0f;
        break;
	case AudioEffectDelay::SyncTime::T_2_3:
		numerator = 2.0f;
        denominator = 3.0f;
        break;
	case AudioEffectDelay::SyncTime::T_3_4:
		numerator = 3.0f;
        denominator = 4.0f;
        break;
	case AudioEffectDelay::SyncTime::T_1_1:
	default:
		numerator = 1.0f;
        denominator = 1.0f;
        break;
	}
    return 60000.0f / this->tempo * numerator / denominator / 1000.0f;
}

void AudioEffectDelay::setMix(float32_t mix)
{
    this->mix = mix;
    if (this->mix <= 0.5f)
    {
        this->dryMix = 1.0f;
        this->wetMix = this->mix * 2;
    }
    else
    {
        this->dryMix = 1.0f - ((this->mix - 0.5f) * 2);
        this->wetMix = 1.0f;
    }
}

void AudioEffectDelay::doProcess(const float32_t* inblockL, const float32_t* inblockR, float32_t* outblockL, float32_t* outblockR, uint16_t len)
{
    for (uint16_t i=0; i < len; i++) 
    {
        float32_t inL = inblockL[i];
        float32_t inR = inblockR[i];
        if (this->pingPongMode)
        {
            // Ping Pong mode only uses timeL
            // Calculate offsets
            int offsetL = this->index - (this->timeL * this->samplerate);
            if (offsetL < 0)
            {
                offsetL = this->bufferSize + offsetL;
            }
            
            // We need to feed the buffers each other
            this->bufferL[index] = this->lpf->processSampleR(this->bufferR[offsetL]) * this->feedback;
            this->bufferR[index] = this->lpf->processSampleL(this->bufferL[offsetL]) * this->feedback;

            outblockL[i] = (inL * this->dryMix) + (this->bufferL[index] * this->wetMix);
            outblockR[i] = (inR * this->dryMix) + (this->bufferR[index] * this->wetMix);

            // Update buffers
            this->bufferL[index] += (inL + inR) * 0.5f;
        }
        else
        {
            // Calculate offsets
            int offsetL = this->index - (this->timeL * this->samplerate);
            if (offsetL < 0)
            {
                offsetL = this->bufferSize + offsetL;
            }
            int offsetR = this->index - (this->timeR * this->samplerate);
            if (offsetR < 0)
            {
                offsetR = this->bufferSize + offsetR;
            }

            this->bufferL[index] = this->lpf->processSampleL(this->bufferL[offsetL]) * this->feedback;
            this->bufferR[index] = this->lpf->processSampleR(this->bufferR[offsetR]) * this->feedback;
            
            outblockL[i] = (inL * this->dryMix) + (this->bufferL[index] * this->wetMix);
            outblockR[i] = (inR * this->dryMix) + (this->bufferR[index] * this->wetMix);

            // Update buffers
            this->bufferL[index] += inL;
            this->bufferR[index] += inR;
        }

        // Update index
        this->index++;
        if (this->index >= this->bufferSize)
        {
            this->index = 0;
        }
    }
}
