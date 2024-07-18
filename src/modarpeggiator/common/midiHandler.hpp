#ifndef _H_MIDI_HANDLER_
#define _H_MIDI_HANDLER_

#include "commons.h"

#include <arm_math.h>
#include <cstdint>

#define MIDI_BUFFER_SIZE 2048
#define EMPTY_SLOT 200

#define MIDI_NOTEOFF 0x80
#define MIDI_NOTEON  0x90
#define MIDI_SYSTEM_EXCLUSIVE 0xF0
#define MIDI_MTC_QUARTER_FRAME 0xF1
#define MIDI_SONG_POSITION_POINTER 0xF2
#define MIDI_SONG_SELECT 0xF3
#define MIDI_UNDEFINED_F4 0xF4
#define MIDI_UNDEFINED_F5 0xF5
#define MIDI_TUNE_REQUEST 0xF6
#define MIDI_END_OF_EXCLUSIVE 0xF7
#define MIDI_TIMING_CLOCK 0xF8
#define MIDI_UNDEFINED_F9 0xF9
#define MIDI_START 0xFA
#define MIDI_CONTINUE 0xFB
#define MIDI_STOP 0xFC
#define MIDI_UNDEFINED_FD 0xFD
#define MIDI_ACTIVE_SENSING 0xFE
#define MIDI_SYSTEM_RESET 0xFF

struct MidiBuffer {
	unsigned maxBufferSize = MIDI_BUFFER_SIZE;

	MidiEvent* bufferedEvents;
	unsigned numBufferedEvents;

	MidiEvent* bufferedMidiThroughEvents;
	unsigned numBufferedThroughEvents;

	MidiEvent* midiOutputBuffer;
	unsigned numOutputEvents;
};

class MidiHandler {
public:
	MidiHandler();
	~MidiHandler();
	void emptyMidiBuffer();
	void appendMidiMessage(MidiEvent event);
	void appendMidiThroughMessage(MidiEvent event);
	void resetBuffer();
	int getNumEvents();
	void mergeBuffers();
	MidiEvent getMidiEvent(int index);
	struct MidiBuffer getMidiBuffer();
private:
	MidiBuffer buffer;
};

#endif //_H_MIDI_HANDLER_
