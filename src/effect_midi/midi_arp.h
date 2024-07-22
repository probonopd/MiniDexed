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
    // ID must be unique for each MidiEffect
    static const unsigned ID = 1;
    static constexpr const char* NAME = "Arp";

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

    enum Mode
    {
        UP,
        DOWN,
        UP_DOWN,
        UP_DOWN_ALT,
        PLAYED,
        RANDOM,
        MODE_UNKNOWN
    };

    enum Division
    {
        D_1_1,
        D_1_2,
        D_1_3,
        D_1_4,
        D_1_4D,
        D_1_4T,
        D_1_8,
        D_1_8D,
        D_1_8T,
        D_1_16,
        D_1_16D,
        D_1_16T,
        D_1_32,
        D_UNKNOWN
    };

    enum OctMode
    {
        OM_UP,
        OM_DOWN,
        OM_UP_DOWN,
        OM_DOWN_UP,
        OM_UP_CYCLE,
        OM_UNKNOWN
    };


    MidiArp(float32_t samplerate, CDexedAdapter* synth);
    virtual ~MidiArp();

    virtual unsigned getId()
    {
        return MidiArp::ID;
    }

    virtual std::string getName()
    {
        return MidiArp::NAME;
    }

    virtual void setTempo(unsigned tempo);
    virtual void setParameter(unsigned param, unsigned value);
    virtual unsigned getParameter(unsigned param);
    
    virtual void keydown(int16_t pitch, uint8_t velocity);
    virtual void keyup(int16_t pitch);

protected:
    virtual size_t getParametersSize()
    {
        return MidiArp::Param::UNKNOWN;
    }

    virtual void doProcess(uint16_t len);

private:
    static const unsigned MIDI_NOTE_OFF = 0b1000;
    static const unsigned MIDI_NOTE_ON = 0b1001;

    Arpeggiator arpeggiator;
    std::vector<MidiEvent> events;
};

#endif // _MIDI_ARP_H