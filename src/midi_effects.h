#ifndef _MIDI_EFFECTS_H
#define _MIDI_EFFECTS_H

#include "effect_midi/midi_effect_base.h"
#include "effect_midi/midi_arp.h"

class MidiEffects
{
public:
	enum Types
	{
		NONE = MidiEffect::ID,
		ARP = MidiArp::ID,
		UNKNOWN
	};
};


inline MidiEffect* newMidiEffect(unsigned type, float32_t samplerate, CDexedAdapter* synth)
{
    switch (type)
	{
	case MidiEffects::Types::ARP:
		return new MidiArp(samplerate, synth);
	default:
		return new MidiEffect(samplerate, synth);
	}
}

inline std::string getMidiFXTypeName(int type)
{
	switch (type)
	{
	case MidiEffects::Types::ARP:
        return MidiArp::NAME;
	case MidiEffects::Types::NONE:
	default:
        return MidiEffect::NAME;
	}
}

inline std::string ToMidiFXType (int value)
{
	return getMidiFXTypeName(value);
}

inline std::string ToArpDivision (int value)
{
	switch (value)
	{
	case MidiArp::Division::D_1_1:
		return "1/1";
	case MidiArp::Division::D_1_2:
		return "1/2";
	case MidiArp::Division::D_1_3:
		return "1/3";
	case MidiArp::Division::D_1_4:
		return "1/4";
	case MidiArp::Division::D_1_4D:
		return "1/4.";
	case MidiArp::Division::D_1_4T:
		return "1/4T";
	case MidiArp::Division::D_1_8:
		return "1/8";
	case MidiArp::Division::D_1_8D:
		return "1/8.";
	case MidiArp::Division::D_1_8T:
		return "1/8T";
	case MidiArp::Division::D_1_16:
		return "1/16";
	case MidiArp::Division::D_1_16D:
		return "1/16.";
	case MidiArp::Division::D_1_16T:
		return "1/16T";
	case MidiArp::Division::D_1_32:
	default:
		return "1/32";
	}
}

inline std::string ToArpMode (int value)
{
	switch (value)
	{
	case MidiArp::Mode::UP:
		return "Up";
	case MidiArp::Mode::DOWN:
		return "Down";
	case MidiArp::Mode::UP_DOWN:
		return "Up-Down";
	case MidiArp::Mode::UP_DOWN_ALT:
		return "Up-Down Alt";
	case MidiArp::Mode::PLAYED:
		return "Played";
	case MidiArp::Mode::RANDOM:
	default:
    	return "Random";
	}
}

inline std::string ToArpOctMode (int value)
{
	switch (value)
	{
	case MidiArp::OctMode::OM_UP:
		return "Up";
	case MidiArp::OctMode::OM_DOWN:
		return "Down";
	case MidiArp::OctMode::OM_UP_DOWN:
		return "Up-Down";
	case MidiArp::OctMode::OM_DOWN_UP:
		return "Down-Up";
	case MidiArp::OctMode::OM_UP_CYCLE:
	default:
    	return "Up-Cycle";
	}
}

#endif // _MIDI_EFFECTS_H