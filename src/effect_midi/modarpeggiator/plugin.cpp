#include "plugin.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

PluginArpeggiator::PluginArpeggiator() :
	Plugin(paramCount, 0, 0),  // paramCount params, 0 program(s), 0 states
	syncMode(1)
{
	arpeggiator.transmitHostInfo(0, 4, 1, 1, 120.0);
	arpeggiator.setSampleRate(static_cast<float>(getSampleRate()));
	arpeggiator.setDivision(7);
}

// -----------------------------------------------------------------------
// Init

void PluginArpeggiator::initParameter(uint32_t index, Parameter& parameter)
{
	if (index >= paramCount) return;

	switch (index) {
		case paramSyncMode:
			parameter.hints = kParameterIsAutomable | kParameterIsInteger;
			parameter.name = "Sync";
			parameter.symbol = "sync";
			parameter.ranges.def = 0;
			parameter.ranges.min = 0;
			parameter.ranges.max = 2;
			parameter.enumValues.count = 3;
			parameter.enumValues.restrictedMode = true;
			{
				ParameterEnumerationValue* const channels = new ParameterEnumerationValue[13];
				parameter.enumValues.values = channels;
				channels[0].label = "Free Running";
				channels[0].value = 0;
				channels[1].label = "Host Sync";
				channels[1].value = 1;
				channels[2].label = "Host Sync (Quantized Start)";
				channels[2].value = 2;
			}
			break;
		case paramBpm:
			parameter.hints = kParameterIsAutomable;
			parameter.name = "Bpm";
			parameter.symbol = "Bpm";
			parameter.ranges.def = 120.f;
			parameter.ranges.min = 20.f;
			parameter.ranges.max = 280.f;
			break;
		case paramDivision:
			parameter.hints = kParameterIsAutomable | kParameterIsInteger;
			parameter.name = "Divison";
			parameter.symbol = "Divisons";
			parameter.ranges.def = 9;
			parameter.ranges.min = 0;
			parameter.ranges.max = 12;
			parameter.enumValues.count = 13;
			parameter.enumValues.restrictedMode = true;
			{
				ParameterEnumerationValue* const channels = new ParameterEnumerationValue[13];
				parameter.enumValues.values = channels;
				channels[0].label = "1/1";
				channels[0].value = 0;
				channels[1].label = "1/2";
				channels[1].value = 1;
				channels[2].label = "1/3";
				channels[2].value = 2;
				channels[3].label = "1/4";
				channels[3].value = 3;
				channels[4].label = "1/4.";
				channels[4].value = 4;
				channels[5].label = "1/4T";
				channels[5].value = 5;
				channels[6].label = "1/8";
				channels[6].value = 6;
				channels[7].label = "1/8.";
				channels[7].value = 7;
				channels[8].label = "1/8T";
				channels[8].value = 8;
				channels[9].label = "1/16";
				channels[9].value = 9;
				channels[10].label = "1/16.";
				channels[10].value = 10;
				channels[11].label = "1/16T";
				channels[11].value = 11;
				channels[12].label = "1/32";
				channels[12].value = 12;
			}
			break;
		case paramVelocity:
			parameter.hints = kParameterIsAutomable | kParameterIsInteger;
			parameter.name = "Velocity";
			parameter.symbol = "velocity";
			parameter.ranges.def = 0;
			parameter.ranges.min = 0;
			parameter.ranges.max = 127;
			break;
		case paramNoteLength:
			parameter.hints      = kParameterIsAutomable;
			parameter.name       = "Note Length";
			parameter.symbol     = "noteLength";
			parameter.unit       = "";
			parameter.ranges.def = 0.f;
			parameter.ranges.min = 0.f;
			parameter.ranges.max = 1.f;
			break;
		case paramOctaveSpread:
			parameter.hints = kParameterIsAutomable | kParameterIsInteger;
			parameter.name = "Octave Spread";
			parameter.symbol = "octaveSpread";
			parameter.ranges.def = 0;
			parameter.ranges.min = 0;
			parameter.ranges.max = 3;
			parameter.enumValues.count = 4;
			parameter.enumValues.restrictedMode = true;
			{
				ParameterEnumerationValue* const channels = new ParameterEnumerationValue[4];
				parameter.enumValues.values = channels;
				channels[0].label = "1 oct";
				channels[0].value = 0;
				channels[1].label = "2 oct";
				channels[1].value = 1;
				channels[2].label = "3 oct";
				channels[2].value = 2;
				channels[3].label = "4 oct";
				channels[3].value = 3;
			}
			break;
		case paramArpMode:
			parameter.hints = kParameterIsAutomable | kParameterIsInteger;
			parameter.name = "Arp Mode";
			parameter.symbol = "arpMode";
			parameter.ranges.def = 0;
			parameter.ranges.min = 0;
			parameter.ranges.max = 5;
			parameter.enumValues.count = 6;
			parameter.enumValues.restrictedMode = true;
			{
				ParameterEnumerationValue* const channels = new ParameterEnumerationValue[6];
				parameter.enumValues.values = channels;
				channels[0].label = "Up";
				channels[0].value = 0;
				channels[1].label = "Down";
				channels[1].value = 1;
				channels[2].label = "Up-Down";
				channels[2].value = 2;
				channels[3].label = "Up-Down (alt)";
				channels[3].value = 3;
				channels[4].label = "Played";
				channels[4].value = 4;
				channels[5].label = "Random";
				channels[5].value = 5;
			}
			break;
		case paramOctaveMode:
			parameter.hints = kParameterIsAutomable | kParameterIsInteger;
			parameter.name = "Octave Mode";
			parameter.symbol = "octaveMode";
			parameter.ranges.def = 0;
			parameter.ranges.min = 0;
			parameter.ranges.max = 4;
			parameter.enumValues.count = 5;
			parameter.enumValues.restrictedMode = true;
			{
				ParameterEnumerationValue* const channels = new ParameterEnumerationValue[5];
				parameter.enumValues.values = channels;
				channels[0].label = "Up";
				channels[0].value = 0;
				channels[1].label = "Down";
				channels[1].value = 1;
				channels[2].label = "Up-Down";
				channels[2].value = 2;
				channels[3].label = "Down-up";
				channels[3].value = 3;
				channels[4].label = "1 Up / Cycle";
				channels[4].value = 4;
			}
			break;
		case paramLatch:
			parameter.hints      = kParameterIsAutomable | kParameterIsBoolean;
			parameter.name       = "Latch";
			parameter.symbol     = "latch";
			parameter.unit       = "";
			parameter.ranges.def = 0.f;
			parameter.ranges.min = 0.f;
			parameter.ranges.max = 1.f;
			break;
		case paramPanic:
			parameter.hints      = kParameterIsAutomable | kParameterIsTrigger;
			parameter.name       = "Panic";
			parameter.symbol     = "Panic";
			parameter.unit       = "";
			parameter.ranges.def = 0;
			parameter.ranges.min = 0;
			parameter.ranges.max = 1;
			break;
		case paramEnabled:
			parameter.hints      = kParameterIsBoolean;
			parameter.name       = "Enabled";
			parameter.symbol     = "enabled";
			parameter.unit       = "";
			parameter.ranges.def = 1.f;
			parameter.ranges.min = 0.f;
			parameter.ranges.max = 1.f;
			break;
	}
}

// -----------------------------------------------------------------------
// Internal data

/**
Optional callback to inform the plugin about a sample rate change.
*/
void PluginArpeggiator::sampleRateChanged(double newSampleRate)
{
	(void) newSampleRate;

	arpeggiator.setSampleRate(static_cast<float>(newSampleRate));
}

/**
Get the current value of a parameter.
*/
float PluginArpeggiator::getParameterValue(uint32_t index) const
{
	switch (index)
	{
		case paramSyncMode:
			return arpeggiator.getSyncMode();
		case paramBpm:
			return arpeggiator.getBpm();
		case paramDivision:
			return arpeggiator.getDivision();
		case paramVelocity:
			return arpeggiator.getVelocity();
		case paramNoteLength:
			return arpeggiator.getNoteLength();
		case paramOctaveSpread:
			return arpeggiator.getOctaveSpread();
		case paramArpMode:
			return arpeggiator.getArpMode();
		case paramOctaveMode:
			return arpeggiator.getOctaveMode();
		case paramLatch:
			return arpeggiator.getLatchMode();
		case paramPanic:
			return arpeggiator.getPanic();
		case paramEnabled:
			return arpeggiator.getArpEnabled();
	}
}

/**
Change a parameter value.
*/
void PluginArpeggiator::setParameterValue(uint32_t index, float value)
{
	switch (index)
	{
		case paramSyncMode:
			syncMode = static_cast<int>(value);
			break;
		case paramBpm:
			arpeggiator.setBpm(value);
			break;
		case paramDivision:
			arpeggiator.setDivision(static_cast<int>(value));
			break;
		case paramVelocity:
			arpeggiator.setVelocity(static_cast<int>(value));
			break;
		case paramNoteLength:
			arpeggiator.setNoteLength(value);
			break;
		case paramOctaveSpread:
			arpeggiator.setOctaveSpread(static_cast<int>(value));
			break;
		case paramArpMode:
			arpeggiator.setArpMode(static_cast<int>(value));
			break;
		case paramOctaveMode:
			arpeggiator.setOctaveMode(static_cast<int>(value));
			break;
		case paramLatch:
			arpeggiator.setLatchMode(static_cast<bool>(value));
			break;
		case paramPanic:
			arpeggiator.setPanic(static_cast<bool>(value));
			break;
		case paramEnabled:
			arpeggiator.setArpEnabled(static_cast<bool>(value));
			break;
	}
}

// -----------------------------------------------------------------------
// Process

void PluginArpeggiator::activate()
{
	// plugin is activated
}

void PluginArpeggiator::run(const float**, float**, uint32_t n_frames,
		const MidiEvent* events, uint32_t eventCount)
{
	arpeggiator.emptyMidiBuffer();

	// Check if host supports Bar-Beat-Tick position
	const TimePosition& position = getTimePosition();
	if (!position.bbt.valid) {
		// set-arpeggiator in free running mode
		arpeggiator.setSyncMode(0);
	} else {
		arpeggiator.setSyncMode(syncMode);
		arpeggiator.transmitHostInfo(position.playing, position.bbt.beatsPerBar, position.bbt.beat, position.bbt.barBeat, static_cast<float>(position.bbt.beatsPerMinute));
	}

	arpeggiator.process(events, eventCount, n_frames);

	struct MidiBuffer buffer = arpeggiator.getMidiBuffer();
	for (unsigned x = 0; x < buffer.numBufferedEvents + buffer.numBufferedThroughEvents; x++) {
		writeMidiEvent(buffer.bufferedEvents[x]); //needs to be one struct or array?
	}
}

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
	return new PluginArpeggiator();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
