#include "midiHandler.hpp"
#include <stdio.h>

MidiHandler::MidiHandler()
{
	printf("MidiHandler constructor\n");
	fflush(NULL);
	buffer.bufferedEvents = new MidiEvent[MIDI_BUFFER_SIZE];
	buffer.bufferedMidiThroughEvents = new MidiEvent[MIDI_BUFFER_SIZE];
	buffer.midiOutputBuffer = new MidiEvent[MIDI_BUFFER_SIZE];

	emptyMidiBuffer();
	printf("MidiHandler constructor finished\n");
	fflush(NULL);
}

MidiHandler::~MidiHandler()
{
	delete buffer.bufferedEvents;
	delete buffer.bufferedMidiThroughEvents;
	delete buffer.midiOutputBuffer;
}

void MidiHandler::emptyMidiBuffer()
{
	buffer.numBufferedEvents = 0;
	buffer.numBufferedThroughEvents = 0;
}

void MidiHandler::appendMidiMessage(MidiEvent event)
{
	buffer.bufferedEvents[buffer.numBufferedEvents] = event;
	buffer.numBufferedEvents = (buffer.numBufferedEvents + 1) % buffer.maxBufferSize;
}

void MidiHandler::appendMidiThroughMessage(MidiEvent event)
{
	buffer.bufferedMidiThroughEvents[buffer.numBufferedThroughEvents] = event;
	buffer.numBufferedThroughEvents = (buffer.numBufferedThroughEvents + 1) % buffer.maxBufferSize;
}

void MidiHandler::mergeBuffers()
{
	for (unsigned e = 0; e < buffer.numBufferedThroughEvents; e++) {
		buffer.bufferedEvents[e + buffer.numBufferedEvents] = buffer.bufferedMidiThroughEvents[e];
	}
}

struct MidiBuffer MidiHandler::getMidiBuffer()
{
	mergeBuffers();
	return buffer;
}
