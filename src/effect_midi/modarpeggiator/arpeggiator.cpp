#include "arpeggiator.hpp"

Arpeggiator::Arpeggiator() :
	notesPressed(0),
	activeNotes(0),
	notePlayed(0),
	octaveMode(0),
	octaveSpread(1),
	arpMode(0),
	noteLength(0.8),
	pitch(0),
	previousMidiNote(0),
	velocity(80),
	previousSyncMode(0),
	activeNotesIndex(0),
	activeNotesBypassed(0),
	timeOutTime(1000),
	firstNoteTimer(0),
	barBeat(0.0),
	pluginEnabled(true),
	first(true),
	arpEnabled(true),
	latchMode(false),
	previousLatch(false),
	latchPlaying(false),
	trigger(false),
	firstNote(false),
	quantizedStart(false),
	resetPattern(false),
	midiNotesCopied(false),
	panic(false),
	division(0),
	sampleRate(48000),
	bpm(0)
{
	clock.transmitHostInfo(0, 4, 1, 1, 120.0);
	clock.setSampleRate(static_cast<float>(48000.0));
	clock.setDivision(7);

	arpPattern = new Pattern*[6];

	arpPattern[0] = new PatternUp();
	arpPattern[1] = new PatternDown();
	arpPattern[2] = new PatternUpDown();
	arpPattern[3] = new PatternUpDownAlt();
	arpPattern[4] = new PatternUp();
	arpPattern[5] = new PatternRandom();


	octavePattern = new Pattern*[5];

	octavePattern[0] = new PatternUp();
	octavePattern[1] = new PatternDown();
	octavePattern[2] = new PatternUpDown();
	octavePattern[3] = new PatternUpDownAlt();
	octavePattern[4] = new PatternCycle();

	for (unsigned i = 0; i < NUM_VOICES; i++) {
		midiNotes[i][0] = EMPTY_SLOT;
		midiNotes[i][1] = 0;
		midiNotesBypassed[i] = EMPTY_SLOT;
	}
	for (unsigned i = 0; i < NUM_VOICES; i++) {
		noteOffBuffer[i][MIDI_NOTE] = EMPTY_SLOT;
		noteOffBuffer[i][MIDI_CHANNEL] = 0;
		noteOffBuffer[i][TIMER] = 0;
	}
}

Arpeggiator::~Arpeggiator()
{

	delete arpPattern[0];
	delete arpPattern[1];
	delete arpPattern[2];
	delete arpPattern[3];
	delete arpPattern[4];
	delete arpPattern[5];
	delete octavePattern[0];
	delete octavePattern[1];
	delete octavePattern[2];
	delete octavePattern[3];
	delete octavePattern[4];

	delete[] arpPattern;
	arpPattern = nullptr;
	delete[] octavePattern;
	octavePattern = nullptr;
}

void Arpeggiator::setArpEnabled(bool arpEnabled)
{
	this->arpEnabled = arpEnabled;
}

void Arpeggiator::setLatchMode(bool latchMode)
{
	this->latchMode = latchMode;
}

void Arpeggiator::setSampleRate(float newSampleRate)
{
	if (newSampleRate != sampleRate) {
		clock.setSampleRate(newSampleRate);
		sampleRate = newSampleRate;
	}
}

void Arpeggiator::setSyncMode(int mode)
{

	switch (mode)
	{
		case FREE_RUNNING:
			clock.setSyncMode(FREE_RUNNING);
			quantizedStart = false;
			break;
		case HOST_BPM_SYNC:
			clock.setSyncMode(HOST_BPM_SYNC);
			quantizedStart = false;
			break;
		case HOST_QUANTIZED_SYNC:
			clock.setSyncMode(HOST_QUANTIZED_SYNC);
			quantizedStart = true;
			break;
	}
}

void Arpeggiator::setBpm(double newBpm)
{
	if (newBpm != bpm) {
		clock.setInternalBpmValue(static_cast<float>(newBpm));
		bpm = newBpm;
	}
}

void Arpeggiator::setDivision(int newDivision)
{
	if (newDivision != division) {
		clock.setDivision(newDivision);
		division = newDivision;
	}
}

void Arpeggiator::setVelocity(uint8_t velocity)
{
	this->velocity = velocity;
}

void Arpeggiator::setNoteLength(float noteLength)
{
	this->noteLength = noteLength;
}

void Arpeggiator::setOctaveSpread(int octaveSpread)
{
	this->octaveSpread = octaveSpread;
}

void Arpeggiator::setArpMode(int arpMode)
{
	arpPattern[arpMode]->setStep(arpPattern[this->arpMode]->getStep());
	arpPattern[arpMode]->setDirection(arpPattern[this->arpMode]->getDirection());

	this->arpMode = arpMode;
}

void Arpeggiator::setOctaveMode(int octaveMode)
{
	octavePattern[octaveMode]->setStep(octavePattern[this->octaveMode]->getStep());
	octavePattern[octaveMode]->setDirection(octavePattern[this->octaveMode]->getDirection());

	this->octaveMode = octaveMode;
}

void Arpeggiator::setPanic(bool panic)
{
	this->panic = panic;
}

bool Arpeggiator::getArpEnabled() const
{
	return arpEnabled;
}

bool Arpeggiator::getLatchMode() const
{
	return latchMode;
}

float Arpeggiator::getSampleRate() const
{
	return clock.getSampleRate();
}

int Arpeggiator::getSyncMode() const
{
	return clock.getSyncMode();
}

float Arpeggiator::getBpm() const
{
	return clock.getInternalBpmValue();
}

int Arpeggiator::getDivision() const
{
	return clock.getDivision();
}

uint8_t Arpeggiator::getVelocity() const
{
	return velocity;
}

float Arpeggiator::getNoteLength() const
{
	return noteLength;
}

int Arpeggiator::getOctaveSpread() const
{
	return octaveSpread;
}

int Arpeggiator::getArpMode() const
{
	return arpMode;
}

int Arpeggiator::getOctaveMode() const
{
	return octaveMode;
}

bool Arpeggiator::getPanic() const
{
	return panic;
}

void Arpeggiator::transmitHostInfo(const bool playing, const float beatsPerBar,
		const int beat, const float barBeat, const double bpm)
{
	clock.transmitHostInfo(playing, beatsPerBar, beat, barBeat, bpm);
	this->barBeat = barBeat;
}

void Arpeggiator::reset()
{
	clock.reset();
	clock.setNumBarsElapsed(0);

	for (unsigned a = 0; a < NUM_ARP_MODES; a++) {
		arpPattern[arpMode]->reset();
	}
	for (unsigned o = 0; o < NUM_OCTAVE_MODES; o++) {
		octavePattern[o]->reset();
	}

	activeNotesIndex = 0;
	firstNoteTimer  = 0;
	notePlayed = 0;
	activeNotes = 0;
	notesPressed = 0;
	activeNotesBypassed = 0;
	latchPlaying = false;
	firstNote = false;
	first = true;

	for (unsigned i = 0; i < NUM_VOICES; i++) {
		midiNotes[i][MIDI_NOTE] = EMPTY_SLOT;
		midiNotes[i][MIDI_CHANNEL] = 0;
	}
}

void Arpeggiator::emptyMidiBuffer()
{
	midiHandler.emptyMidiBuffer();
}

void Arpeggiator::allNotesOff()
{
	for (unsigned i = 0; i < NUM_VOICES; i++) {
		midiNotesBypassed[i] = EMPTY_SLOT;
	}
	notesPressed = 0;
	activeNotes = 0;
	reset();
}

struct MidiBuffer Arpeggiator::getMidiBuffer()
{
	return midiHandler.getMidiBuffer();
}

void Arpeggiator::process(const MidiEvent* events, uint32_t eventCount, uint32_t n_frames)
{
	struct MidiEvent midiEvent;
	struct MidiEvent midiThroughEvent;

	if (!arpEnabled && !latchMode) {

		reset();

		for (unsigned clear_notes = 0; clear_notes < NUM_VOICES; clear_notes++) {
			midiNotes[clear_notes][MIDI_NOTE] = EMPTY_SLOT;
			midiNotes[clear_notes][MIDI_CHANNEL] = 0;
		}
	}

	if (!latchMode && previousLatch && notesPressed <= 0) {
		reset();
	}
	if (latchMode != previousLatch) {
		previousLatch = latchMode;
	}

	if (panic) {
		reset();
		panic = false;
	}

	for (uint32_t i=0; i<eventCount; ++i) {

		uint8_t status = events[i].data[0] & 0xF0;

		uint8_t midiNote = events[i].data[1];
		uint8_t noteToFind;
		uint8_t foundNote;
		size_t searchNote;

		if (arpEnabled) {

			if (!latchPlaying && (midiNote == 0x7b && events[i].size == 3)) {
				allNotesOff();
			}

			midiNotesCopied = false;

			bool voiceFound;
			bool pitchFound;
			bool noteOffFoundInBuffer;
			size_t findFreeVoice;
			size_t findActivePitch;

			uint8_t channel = events[i].data[0] & 0x0F;

			switch(status) {
				case MIDI_NOTEON:
					if (activeNotes > NUM_VOICES - 1) {
						reset();
					} else {
						if (first) {
							firstNote = true;
						}
						if (notesPressed == 0) {
							if (!latchPlaying) { //TODO check if there needs to be an exception when using sync
								octavePattern[octaveMode]->reset();
								clock.reset();
								notePlayed = 0;
							}
							if (latchMode) {
								latchPlaying = true;
								activeNotes = 0;
								for (unsigned i = 0; i < NUM_VOICES; i++) {
									midiNotes[i][MIDI_NOTE] = EMPTY_SLOT;
									midiNotes[i][MIDI_CHANNEL] = 0;
								}
							}
							resetPattern = true;
						}

						findFreeVoice = 0;
						findActivePitch = 0;
						voiceFound = false;
						pitchFound = false;

						while (findActivePitch < NUM_VOICES && !pitchFound)
						{
							if (midiNotes[findActivePitch][MIDI_NOTE] == (uint32_t)midiNote) {
								pitchFound = true;
							}
							findActivePitch++;
						}

						if (!pitchFound) {
							while (findFreeVoice < NUM_VOICES && !voiceFound)
							{
								if (midiNotes[findFreeVoice][MIDI_NOTE] == EMPTY_SLOT) {
									midiNotes[findFreeVoice][MIDI_NOTE] = midiNote;
									midiNotes[findFreeVoice][MIDI_CHANNEL] = channel;
									voiceFound = true;
								}
								findFreeVoice++;
							}
							notesPressed++;
							activeNotes++;
						}

						if (arpMode != ARP_PLAYED)
							utils.quicksort(midiNotes, 0, NUM_VOICES - 1);
						if (midiNote < midiNotes[notePlayed - 1][MIDI_NOTE] && notePlayed > 0) {
							notePlayed++;
						}
					}
					break;
				case MIDI_NOTEOFF:
					searchNote = 0;
					foundNote = 0;
					noteOffFoundInBuffer = false;
					noteToFind = midiNote;

					if (!latchMode) {
						latchPlaying = false;
					} else {
						latchPlaying = true;
					}

					while (searchNote < NUM_VOICES)
					{
						if (midiNotes[searchNote][MIDI_NOTE] == noteToFind)
						{
							foundNote = searchNote;
							noteOffFoundInBuffer = true;
							searchNote = NUM_VOICES;
						}
						searchNote++;
					}

					if (noteOffFoundInBuffer) {

						notesPressed = (notesPressed > 0) ? notesPressed - 1 : 0;

						if (!latchPlaying) {
							activeNotes = notesPressed;
						}

						if (!latchMode) {
							midiNotes[foundNote][MIDI_NOTE] = EMPTY_SLOT;
							midiNotes[foundNote][MIDI_CHANNEL] = 0;
							if (arpMode != ARP_PLAYED)
								utils.quicksort(midiNotes, 0, NUM_VOICES - 1);
						}
					} else {
						midiThroughEvent.frame = events[i].frame;
						midiThroughEvent.size = events[i].size;
						for (unsigned d = 0; d < midiThroughEvent.size; d++) {
							midiThroughEvent.data[d] = events[i].data[d];
						}
						midiHandler.appendMidiThroughMessage(midiThroughEvent);
					}
					if (activeNotes == 0 && !latchPlaying && !latchMode) {
						reset();
					}
					break;
				default:
					midiThroughEvent.frame = events[i].frame;
					midiThroughEvent.size = events[i].size;
					for (unsigned d = 0; d < midiThroughEvent.size; d++) {
						midiThroughEvent.data[d] = events[i].data[d];
					}
					midiHandler.appendMidiThroughMessage(midiThroughEvent);
					break;
			}
		} else { //if arpeggiator is off

			if (!midiNotesCopied) {
				for (unsigned b = 0; b < NUM_VOICES; b++) {
					midiNotesBypassed[b] = midiNotes[b][MIDI_NOTE];
				}
				midiNotesCopied = true;
			}

			if (latchMode) {

				uint8_t noteToFind = midiNote;
				size_t searchNote = 0;

				switch (status)
				{
					case MIDI_NOTEOFF:
						while (searchNote < NUM_VOICES)
						{
							if (midiNotesBypassed[searchNote] == noteToFind) {
								midiNotesBypassed[searchNote] = EMPTY_SLOT;
								searchNote = NUM_VOICES;
								notesPressed = (notesPressed > 0) ? notesPressed - 1 : 0;
							}
							searchNote++;
						}
						break;
				}
			}

			if (midiNote == 0x7b && events[i].size == 3) {
				allNotesOff();
			}
			//send MIDI message through
			midiHandler.appendMidiThroughMessage(events[i]);
			first = true;
		}
	}

	arpPattern[arpMode]->setPatternSize(activeNotes);

	int patternSize;

	switch (arpMode)
	{
		case ARP_UP_DOWN:
			patternSize = (activeNotes >= 3) ? activeNotes + (activeNotes - 2) : activeNotes;
			break;
		case ARP_UP_DOWN_ALT:
			patternSize = (activeNotes >= 3) ? activeNotes * 2 : activeNotes;
			break;
		default:
			patternSize = activeNotes;
			break;
	}

	switch (octaveMode)
	{
		case ONE_OCT_UP_PER_CYCLE:
			octavePattern[octaveMode]->setPatternSize(patternSize);
			octavePattern[octaveMode]->setCycleRange(octaveSpread);
			break;
		default:
			octavePattern[octaveMode]->setPatternSize(octaveSpread);
			break;
	}

	for (unsigned s = 0; s < n_frames; s++) {

		bool timeOut = (firstNoteTimer > (int)timeOutTime) ? false : true;

		if (firstNote) {
			clock.closeGate(); //close gate to prevent opening before timeOut
			firstNoteTimer++;
		}

		if (clock.getSyncMode() <= 1 && first) {
			clock.setPos(0);
			clock.reset();
		}

		clock.tick();

		if ((clock.getGate() && !timeOut)) {

			if (arpEnabled) {

				if (resetPattern) {
					octavePattern[octaveMode]->reset();
					if (octaveMode == ARP_DOWN) {
						octavePattern[octaveMode]->setStep(activeNotes - 1); //TODO maybe put this in reset()
					}

					arpPattern[arpMode]->reset();
					if (arpMode == ARP_DOWN) {
						arpPattern[arpMode]->setStep(activeNotes - 1);
					}

					resetPattern = false;

					notePlayed = arpPattern[arpMode]->getStep();
				}

				if (first) {
					//send all notes off, on current active MIDI channel
					midiEvent.size = 3;
					midiEvent.data[2] = 0;

					midiEvent.frame = s;
					midiEvent.data[0] = 0xb0 | midiNotes[notePlayed][MIDI_CHANNEL];
					midiEvent.data[1] = 0x40; // sustain pedal
					midiHandler.appendMidiMessage(midiEvent);
					midiEvent.data[1] = 0x7b; // all notes off
					midiHandler.appendMidiMessage(midiEvent);

					first = false;
				}
			}

			size_t searchedVoices = 0;
			bool   noteFound = false;

			while (!noteFound && searchedVoices < NUM_VOICES && activeNotes > 0 && arpEnabled)
			{
				notePlayed = (notePlayed < 0) ? 0 : notePlayed;

				if (midiNotes[notePlayed][MIDI_NOTE] > 0
						&& midiNotes[notePlayed][MIDI_NOTE] < 128)
				{
					//create MIDI note on message
					uint8_t midiNote = midiNotes[notePlayed][MIDI_NOTE];
					uint8_t channel = midiNotes[notePlayed][MIDI_CHANNEL];

					if (arpEnabled) {

						uint8_t octave = octavePattern[octaveMode]->getStep() * 12;
						octavePattern[octaveMode]->goToNextStep();

						midiNote = midiNote + octave;

						midiEvent.frame = s;
						midiEvent.size = 3;
						midiEvent.data[0] = MIDI_NOTEON | channel;
						midiEvent.data[1] = midiNote;
						midiEvent.data[2] = velocity;

						midiHandler.appendMidiMessage(midiEvent);

						noteOffBuffer[activeNotesIndex][MIDI_NOTE] = (uint32_t)midiNote;
						noteOffBuffer[activeNotesIndex][MIDI_CHANNEL] = (uint32_t)channel;
						activeNotesIndex = (activeNotesIndex + 1) % NUM_NOTE_OFF_SLOTS;
						noteFound = true;
						firstNote = false;
					}
				}
				arpPattern[arpMode]->goToNextStep();
				notePlayed = arpPattern[arpMode]->getStep();
				searchedVoices++;
			}
			clock.closeGate();
		}

		for (size_t i = 0; i < NUM_NOTE_OFF_SLOTS; i++) {
			if (noteOffBuffer[i][MIDI_NOTE] != EMPTY_SLOT) {
				noteOffBuffer[i][TIMER] += 1;
				if (noteOffBuffer[i][TIMER] > static_cast<uint32_t>(clock.getPeriod() * noteLength)) {
					midiEvent.frame = s;
					midiEvent.size = 3;
					midiEvent.data[0] = MIDI_NOTEOFF | noteOffBuffer[i][MIDI_CHANNEL];
					midiEvent.data[1] = static_cast<uint8_t>(noteOffBuffer[i][MIDI_NOTE]);
					midiEvent.data[2] = 0;

					midiHandler.appendMidiMessage(midiEvent);

					noteOffBuffer[i][MIDI_NOTE] = EMPTY_SLOT;
					noteOffBuffer[i][MIDI_CHANNEL] = 0;
					noteOffBuffer[i][TIMER] = 0;

				}
			}
		}
	}
	midiHandler.mergeBuffers();
}
