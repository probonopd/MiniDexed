#ifndef _H_PLUGIN_ARPEGGIATOR_
#define _H_PLUGIN_ARPEGGIATOR_

#include "arpeggiator.hpp"
#include "common/commons.h"
#include "common/clock.hpp"
#include "common/pattern.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class PluginArpeggiator : public Plugin {
public:
	enum Parameters {
		paramSyncMode = 0,
		paramBpm,
		paramDivision,
		paramVelocity,
		paramNoteLength,
		paramOctaveSpread,
		paramArpMode,
		paramOctaveMode,
		paramLatch,
		paramPanic,
		paramEnabled,
		paramCount
	};

	PluginArpeggiator();

protected:
	// -------------------------------------------------------------------
	// Information

	const char* getLabel() const noexcept override {
		return "Arpeggiator";
	}

	const char* getDescription() const override {
		return "A MIDI arpeggiator";
	}

	const char* getMaker() const noexcept override {
		return "MOD";
	}

	const char* getHomePage() const override {
		return "";
	}

	const char* getLicense() const noexcept override {
		return "https://spdx.org/licenses/GPL-2.0-or-later";
	}

	uint32_t getVersion() const noexcept override {
		return d_version(1, 1, 2);
	}

	int64_t getUniqueId() const noexcept override {
		return d_cconst('M', 'O', 'A', 'P');
	}

	// -------------------------------------------------------------------
	// Init

	void initParameter(uint32_t index, Parameter& parameter) override;

	// -------------------------------------------------------------------
	// Internal data

	float getParameterValue(uint32_t index) const override;
	void setParameterValue(uint32_t index, float value) override;

	// -------------------------------------------------------------------
	// Optional

	// Optional callback to inform the plugin about a sample rate change.
	void sampleRateChanged(double newSampleRate) override;

	// -------------------------------------------------------------------
	// Process

	void activate() override;

	void run(const float**, float**, uint32_t,
			const MidiEvent* midiEvents, uint32_t midiEventCount) override;


	// -------------------------------------------------------------------

private:
	Arpeggiator arpeggiator;
	float fParams[paramCount];
	int syncMode;

	DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginArpeggiator)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif  //_H_PLUGIN_ARPEGGIATOR_
