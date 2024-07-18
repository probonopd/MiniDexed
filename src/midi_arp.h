/* 
 * Base AudioEffect interface
 * Javier Nonis (https://github.com/jnonis) - 2024
 */
#ifndef _MIDI_ARP_H
#define _MIDI_ARP_H

#include <vector>
#include <arm_math.h>
#include "modarpeggiator/common/commons.h"
#include "modarpeggiator/arpeggiator.hpp"
#include "modarpeggiator/common/clock.hpp"
#include "modarpeggiator/common/pattern.hpp"
#include "dexedadapter.h"

class MidiArp
{
public:
    MidiArp(float32_t samplerate, CDexedAdapter* synth);
    ~MidiArp();

    void keydown(int16_t pitch, uint8_t velocity);
    void keyup(int16_t pitch);

    void process(uint16_t len);
protected:
    bool bypass = false;
    float32_t samplerate;

private:
    static const unsigned MIDI_NOTE_OFF = 0b1000;
    static const unsigned MIDI_NOTE_ON = 0b1001;

    CDexedAdapter* synth;
    Arpeggiator arpeggiator;
	int syncMode;
    std::vector<MidiEvent> events;
};

#endif // _MIDI_ARP_H