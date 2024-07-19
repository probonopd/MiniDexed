/* 
 * MOD Arpeggiator port
 * Ported from https://ithub.com/moddevices/mod-arpeggiator-lv2
 * 
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _MIDI_ARP_H
#define _MIDI_ARP_H

#include "midi_effect_base.h"
#include <vector>
#include <arm_math.h>
#include "modarpeggiator/common/commons.h"
#include "modarpeggiator/arpeggiator.hpp"
#include "modarpeggiator/common/clock.hpp"
#include "modarpeggiator/common/pattern.hpp"

class MidiArp : public MidiEffect
{
public:
    static const unsigned MIDI_EFFECT_ARP = 1;

    enum Param
    {
        BYPASS,
        LATCH,
        SYNC,
        ARP_MODE,
        DIVISION,
        NOTE_LENGTH,
        VELOCITY,
        OCTAVE_SPREAD,
        OCTAVE_MODE,
        PANIC,
        UNKNOWN
    };

    MidiArp(float32_t samplerate, CDexedAdapter* synth);
    virtual ~MidiArp();

    virtual unsigned getId()
    {
        return MIDI_EFFECT_ARP;
    }

    virtual void setTempo(unsigned tempo);
    virtual void setParameter(unsigned param, unsigned value);
    virtual unsigned getParameter(unsigned param);
    
    void keydown(int16_t pitch, uint8_t velocity);
    void keyup(int16_t pitch);

protected:
    virtual size_t getParametersSize()
    {
        return MidiArp::Param::UNKNOWN;
    }

    virtual void doProcess(uint16_t len);

private:
    static const unsigned MIDI_NOTE_OFF = 0b1000;
    static const unsigned MIDI_NOTE_ON = 0b1001;

    CDexedAdapter* synth;
    Arpeggiator arpeggiator;
	int syncMode;
    std::vector<MidiEvent> events;
};

#endif // _MIDI_ARP_H