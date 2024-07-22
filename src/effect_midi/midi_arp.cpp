#include "midi_arp.h"
#include <stdio.h>

MidiArp::MidiArp(float32_t samplerate, CDexedAdapter* synth) : MidiEffect(samplerate, synth)
{
    arpeggiator.transmitHostInfo(0, 4, 1, 1, 120.0);
	arpeggiator.setSampleRate(samplerate);
	
	arpeggiator.setBpm(120);
	
	this->setParameter(MidiArp::Param::LATCH, 0);
	this->setParameter(MidiArp::Param::ARP_MODE, 0);
	this->setParameter(MidiArp::Param::DIVISION, 9);
	this->setParameter(MidiArp::Param::NOTE_LENGTH, 70);
	this->setParameter(MidiArp::Param::VELOCITY, 110);
	this->setParameter(MidiArp::Param::OCTAVE_SPREAD, 1);
	this->setParameter(MidiArp::Param::OCTAVE_MODE, 4);
}

MidiArp::~MidiArp()
{
}

void MidiArp::setTempo(unsigned tempo)
{
	arpeggiator.setBpm(tempo);
}

void MidiArp::setParameter(unsigned param, unsigned value)
{
	switch (param)
    {
    case MidiArp::Param::BYPASS:
        this->setBypass(value == 1);
        break;
    case MidiArp::Param::LATCH:
		this->arpeggiator.setLatchMode(value == 1);
        break;
    case MidiArp::Param::SYNC:
		this->arpeggiator.setSyncMode(value);
        break;
    case MidiArp::Param::ARP_MODE:
        this->arpeggiator.setArpMode(value);
        break;
    case MidiArp::Param::DIVISION:
        this->arpeggiator.setDivision(value);
        break;
    case MidiArp::Param::NOTE_LENGTH:
        this->arpeggiator.setNoteLength((float) value / 100.0f);
        break;
    case MidiArp::Param::VELOCITY:
        this->arpeggiator.setVelocity(value);
        break;
	case MidiArp::Param::OCTAVE_SPREAD:
        this->arpeggiator.setOctaveSpread(value);
        break;
	case MidiArp::Param::OCTAVE_MODE:
        this->arpeggiator.setOctaveMode(value);
        break;
	case MidiArp::Param::PANIC:
        this->arpeggiator.setPanic(value == 1);
        break;
    default:
        break;
    }
}

unsigned MidiArp::getParameter(unsigned param)
{
	switch (param)
    {
    case MidiArp::Param::BYPASS:
		return this->getBypass() ? 1 : 0;
    case MidiArp::Param::LATCH:
        return this->arpeggiator.getLatchMode() ? 1 : 0;
    case MidiArp::Param::SYNC:
        return this->arpeggiator.getSyncMode();
    case MidiArp::Param::ARP_MODE:
        return this->arpeggiator.getArpMode();
    case MidiArp::Param::DIVISION:
        return this->arpeggiator.getDivision();
    case MidiArp::Param::NOTE_LENGTH:
        return roundf(this->arpeggiator.getNoteLength() * 100);
    case MidiArp::Param::VELOCITY:
        return this->arpeggiator.getVelocity();
	case MidiArp::Param::OCTAVE_SPREAD:
        return this->arpeggiator.getOctaveSpread();
	case MidiArp::Param::OCTAVE_MODE:
        return this->arpeggiator.getOctaveMode();
	case MidiArp::Param::PANIC:
        return this->arpeggiator.getPanic();
    default:
        return 0;
    }
}

void MidiArp::keydown(int16_t pitch, uint8_t velocity)
{
	MidiEvent event;
	event.data[0] = MIDI_NOTE_ON << 4;
	event.data[1] = pitch;
	event.data[2] = velocity;
	event.size = 3;
	event.frame = 0;
	this->events.push_back(event);
}

void MidiArp::keyup(int16_t pitch)
{
	MidiEvent event;
	event.data[0] = MIDI_NOTE_OFF << 4;
	event.data[1] = pitch;
	event.data[2] = 0;
	event.size = 3;
	event.frame = 0;
	this->events.push_back(event);
}

void MidiArp::doProcess(uint16_t len)
{
	arpeggiator.emptyMidiBuffer();
	
	// Check if host supports Bar-Beat-Tick position
	arpeggiator.setSyncMode(0);
	/*
    const TimePosition& position = getTimePosition();
	if (!position.bbt.valid) {
		// set-arpeggiator in free running mode
		arpeggiator.setSyncMode(0);
	} else {
		arpeggiator.setSyncMode(syncMode);
		arpeggiator.transmitHostInfo(position.playing, position.bbt.beatsPerBar, position.bbt.beat, position.bbt.barBeat, static_cast<float>(position.bbt.beatsPerMinute));
	}
    */

   	arpeggiator.process(events.data(), events.size(), len);
	events.clear();
	events.shrink_to_fit();
	
	struct MidiBuffer buffer = arpeggiator.getMidiBuffer();
	for (unsigned x = 0; x < buffer.numBufferedEvents + buffer.numBufferedThroughEvents; x++) {
		MidiEvent event = buffer.bufferedEvents[x];
		unsigned eventType = event.data[0] >> 4;
		
		switch (eventType)
		{
		case MIDI_NOTE_ON:
			if (event.data[2] > 0)
			{
				if (event.data[2] <= 127)
				{
					this->synth->keydown(event.data[1], event.data[2]);
				}
			}
			else
			{
				this->synth->keyup(event.data[1]);
			}
			break;

		case MIDI_NOTE_OFF:
			this->synth->keyup(event.data[1]);
			break;
		
		default:
			break;
		}
	}
}