#include "clock.hpp"

PluginClock::PluginClock() :
	gate(false),
	trigger(false),
	beatSync(true),
	phaseReset(false),
	playing(false),
	previousPlaying(false),
	endOfBar(false),
	init(false),
	period(0),
	halfWavelength(0),
	quarterWaveLength(0),
	pos(0),
	beatsPerBar(1.0),
	bpm(120.0),
	internalBpm(120.0),
	previousBpm(0),
	sampleRate(48000.0),
	division(1),
	hostBarBeat(0.0),
	beatTick(0.0),
	syncMode(1),
	previousSyncMode(0),
	hostTick(0),
	hostBeat(0),
	barLength(4),
	numBarsElapsed(0),
	previousBeat(0),
	arpMode(0)
{
}

PluginClock::~PluginClock()
{
}

void PluginClock::transmitHostInfo(const bool playing, const float beatsPerBar,
		const int hostBeat, const float hostBarBeat, const float hostBpm)
{
	this->beatsPerBar = beatsPerBar;
	this->hostBeat = hostBeat;
	this->hostBarBeat = hostBarBeat;
	this->hostBpm = hostBpm;
	this->playing = playing;

	if (playing && !previousPlaying && beatSync) {
		syncClock();
	}
	if (playing != previousPlaying) {
		previousPlaying = playing;
	}

	if (!init) {
		calcPeriod();
		init = true;
	}
}

void PluginClock::setSyncMode(int mode)
{
	switch (mode)
	{
		case FREE_RUNNING:
			beatSync = false;
			break;
		case HOST_BPM_SYNC:
			beatSync = false;
			break;
		case HOST_QUANTIZED_SYNC:
			beatSync = true;
			break;
	}

	this->syncMode = mode;
}

void PluginClock::setInternalBpmValue(float internalBpm)
{
	this->internalBpm = internalBpm;
}

void PluginClock::setBpm(float bpm)
{
	this->bpm = bpm;
	calcPeriod();
}

void PluginClock::setSampleRate(float sampleRate)
{
	this->sampleRate = sampleRate;
	calcPeriod();
}

void PluginClock::setDivision(int setDivision)
{
	this->division = setDivision;
	this->divisionValue = divisionValues[setDivision];

	calcPeriod();
}

void PluginClock::syncClock()
{
	pos = static_cast<uint32_t>(fmod(sampleRate * (60.0f / bpm) * (hostBarBeat + (numBarsElapsed * beatsPerBar)), sampleRate * (60.0f / (bpm * (divisionValue / 2.0f)))));
}

void PluginClock::setPos(uint32_t pos)
{
	this->pos = pos;
}

void PluginClock::setNumBarsElapsed(uint32_t numBarsElapsed)
{
	this->numBarsElapsed = numBarsElapsed;
}

void PluginClock::calcPeriod()
{
	period = static_cast<uint32_t>(sampleRate * (60.0f / (bpm * (divisionValue / 2.0f))));
	halfWavelength = static_cast<uint32_t>(period / 2.0f);
	quarterWaveLength = static_cast<uint32_t>(halfWavelength / 2.0f);
	period = (period <= 0) ? 1 : period;
}

void PluginClock::closeGate()
{
	gate = false;
}

void PluginClock::reset()
{
	trigger = false;
}

float PluginClock::getSampleRate() const
{
	return sampleRate;
}

bool PluginClock::getGate() const
{
	return gate;
}

int PluginClock::getSyncMode() const
{
	return syncMode;
}

float PluginClock::getInternalBpmValue() const
{
	return internalBpm;
}

int PluginClock::getDivision() const
{
	return division;
}

uint32_t PluginClock::getPeriod() const
{
	return period;
}

uint32_t PluginClock::getPos() const
{
	return pos;
}

void PluginClock::tick()
{
	int beat = static_cast<int>(hostBarBeat);

	if (beatsPerBar <= 1) {
		if (hostBarBeat > 0.99 && !endOfBar) {
			endOfBar = true;
		}
		else if (hostBarBeat < 0.1 && endOfBar) {
			numBarsElapsed++;
			endOfBar = false;
		}
	} else {
		if (beat != previousBeat) {
			numBarsElapsed = (beat == 0) ? numBarsElapsed + 1 : numBarsElapsed;
			previousBeat = beat;
		}
	}

	float threshold = 0.009; //TODO might not be needed

	switch (syncMode)
	{
		case FREE_RUNNING:
			if ((internalBpm != previousBpm) || (syncMode != previousSyncMode)) {
				setBpm(internalBpm);
				previousBpm = internalBpm;
				previousSyncMode = syncMode;
			}
			break;
		case HOST_BPM_SYNC:
			if ((hostBpm != previousBpm && (fabs(previousBpm - hostBpm) > threshold)) || (syncMode != previousSyncMode)) {
				setBpm(hostBpm);
				previousBpm = hostBpm;
				previousSyncMode = syncMode;
			}
			break;
		case HOST_QUANTIZED_SYNC: //TODO fix this duplicate
			if ((hostBpm != previousBpm && (fabs(previousBpm - hostBpm) > threshold)) || (syncMode != previousSyncMode)) {
				setBpm(hostBpm);
				if (playing) {
					syncClock();
				}
				previousBpm = hostBpm;
				previousSyncMode = syncMode;
			}
			break;
	}

	if (pos > period) {
		pos = 0;
	}

	if (pos < quarterWaveLength && !trigger) {
		gate = true;
		trigger = true;
	} else if (pos > halfWavelength && trigger) {
		if (playing && beatSync) {
			syncClock();
		}
		trigger = false;
	}

	if (playing && beatSync) {
		syncClock(); //hard-sync to host position
	}
	else if (!beatSync) {
		pos++;
	}
}
