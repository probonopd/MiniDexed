#include "midiHandler.hpp"
#include <stdio.h>

MidiHandler::MidiHandler()
{
	printf("MidiHandler constructor\n");
	printf("MidiEvent size: %d\n", sizeof(MidiEvent));
	fflush(NULL);
	/*
	memset(buffer.bufferedEvents, 0, MIDI_BUFFER_SIZE * sizeof(MidiEvent));
	memset(buffer.bufferedMidiThroughEvents, 0, MIDI_BUFFER_SIZE * sizeof(MidiEvent));
	memset(buffer.midiOutputBuffer, 0, MIDI_BUFFER_SIZE * sizeof(MidiEvent));
	*/
	for (unsigned i = 0; i < MIDI_BUFFER_SIZE; i++) {
		printf("i: %d\n", i);
		fflush(NULL);
		for (unsigned x = 0; x < buffer.bufferedEvents[i].kDataSize; i++) {
			printf("x: %d\n", x);
			fflush(NULL);
			/*
			buffer.bufferedEvents[i].data[x] = 0;
			buffer.bufferedMidiThroughEvents[i].data[x] = 0;
			buffer.midiOutputBuffer[i].data[x] = 0;
			*/
		}
	}
	emptyMidiBuffer();
	//printf("buffer.bufferedEvents: %d\n", buffer.bufferedEvents[0].data);
	//fflush(NULL);
}

MidiHandler::~MidiHandler()
{
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
