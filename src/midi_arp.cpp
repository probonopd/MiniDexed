#include "midi_arp.h"
#include <stdio.h>

MidiArp::MidiArp(float32_t samplerate, CDexedAdapter* synth)
{
    this->samplerate = samplerate;
    this->syncMode = 1;
	this->synth = synth;

    arpeggiator.transmitHostInfo(0, 4, 1, 1, 120.0);
	arpeggiator.setSampleRate(samplerate);
	arpeggiator.setDivision(7);

	arpeggiator.getMidiBuffer();
}

MidiArp::~MidiArp()
{
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

void MidiArp::process(uint16_t len)
{
	arpeggiator.emptyMidiBuffer();
	
	// Check if host supports Bar-Beat-Tick position
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
	
	/*
	printf("Before Send Midi\n");
	fflush(NULL);
	struct MidiBuffer buffer = arpeggiator.getMidiBuffer();
	for (unsigned x = 0; x < buffer.numBufferedEvents + buffer.numBufferedThroughEvents; x++) {
		printf("Loop x: %d\n", x);
		fflush(NULL);
		
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
	printf("After Send Midi\n");
	fflush(NULL);
	*/
}