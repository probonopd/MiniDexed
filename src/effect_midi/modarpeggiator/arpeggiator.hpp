#ifndef _H_ARPEGGIATOR_
#define _H_ARPEGGIATOR_

#include <cstdint>

#include "common/commons.h"
#include "common/clock.hpp"
#include "common/pattern.hpp"
#include "common/midiHandler.hpp"
#include "utils.hpp"

#define NUM_VOICES 32
#define NUM_NOTE_OFF_SLOTS 32
#define PLUGIN_URI "http://moddevices.com/plugins/mod-devel/arpeggiator"

#define MIDI_NOTEOFF 0x80
#define MIDI_NOTEON  0x90

#define MIDI_NOTE 0
#define MIDI_CHANNEL 1
#define TIMER 2

#define NUM_ARP_MODES 6
#define NUM_OCTAVE_MODES 5

#define NUM_MIDI_CHANNELS 16

#define ONE_OCT_UP_PER_CYCLE 4

class Arpeggiator {
public:
	enum ArpModes {
		ARP_UP = 0,
		ARP_DOWN,
		ARP_UP_DOWN,
		ARP_UP_DOWN_ALT,
		ARP_PLAYED,
		ARP_RANDOM
	};
	Arpeggiator();
	~Arpeggiator();
	void setArpEnabled(bool arpEnabled);
	void setLatchMode(bool latchMode);
	void setSampleRate(float sampleRate);
	void setSyncMode(int mode);
	void setBpm(double bpm);
	void setDivision(int division);
	void setVelocity(uint8_t velocity);
	void setNoteLength(float noteLength);
	void setOctaveSpread(int octaveSpread);
	void setArpMode(int arpMode);
	void setOctaveMode(int octaveMode);
	void setPanic(bool panic);
	bool getArpEnabled() const;
	bool getLatchMode() const;
	float getSampleRate() const;
	int getSyncMode() const;
	float getBpm() const;
	int getDivision() const;
	uint8_t getVelocity() const;
	float getNoteLength() const;
	int getOctaveSpread() const;
	int getArpMode() const;
	int getOctaveMode() const;
	bool getPanic() const;
	void transmitHostInfo(const bool playing, const float beatsPerBar,
	const int beat, const float barBeat, const double bpm);
	void reset();
	void emptyMidiBuffer();
	void allNotesOff();
	struct MidiBuffer getMidiBuffer();
	void process(const MidiEvent* event, uint32_t eventCount, uint32_t n_frames);
private:
	uint8_t midiNotes[NUM_VOICES][2];
	uint8_t midiNotesBypassed[NUM_VOICES];
	uint32_t noteOffBuffer[NUM_NOTE_OFF_SLOTS][3];

	int notesPressed;
	int activeNotes;
	int notePlayed;

	int octaveMode;
	int octaveSpread;
	int arpMode;

	float noteLength;

	uint8_t pitch;
	uint8_t previousMidiNote;
	uint8_t velocity;
	int previousSyncMode;
	int activeNotesIndex;
	int activeNotesBypassed;
	int timeOutTime;
	int firstNoteTimer;
	float barBeat;

	bool pluginEnabled;
	bool first;
	bool arpEnabled;
	bool latchMode;
	bool previousLatch;
	bool latchPlaying;
	bool trigger;
	bool firstNote;
	bool quantizedStart;
	bool resetPattern;
	bool midiNotesCopied;
	bool panic;

	int division;
	float sampleRate;
	double bpm;

	ArpUtils utils;
	Pattern **arpPattern;
	Pattern **octavePattern;
	MidiHandler midiHandler;
	PluginClock clock;
};

#endif //_H_ARPEGGIATOR_
