// -*-c++-*-

/**
 * Tunings.h
 * Copyright Paul Walker, 2019-2020
 * Released under the MIT License. See LICENSE.md
 *
 * Tunings.h contains the public API required to determine full keyboard frequency maps
 * for a scala SCL and KBM file in standalone, tested, open licensed C++ header only library.
 *
 * An example of using the API is
 *
 * ```
 * auto s = Tunings::readSCLFile( "./my-scale.scl" );
 * auto k = Tunings::readKBMFile( "./my-mapping.kbm" );
 *
 * Tunings::Tuning t( s, k );
 *
 * std::cout << "The frequency of C4 and A4 are "
 *           << t.frequencyForMidiNote( 60 ) << " and " 
 *           << t.frequencyForMidiNote( 69 ) << std::endl; 
 * ```
 * 
 * The API provides several other points, such as access to the structure of the SCL and KBM,
 * the ability to create several prototype SCL and KBM files wthout SCL or KBM content, 
 * a frequency measure which is normalized by the frequency of standard tuning midi note 0
 * and the logarithmic frequency scale, with a doubling per frequency doubling.
 * 
 * Documentation is in the class header below; tests are in `tests/all_tests.cpp` and
 * a variety of command line tools accompany the header.
 */

#ifndef __INCLUDE_TUNINGS_H
#define __INCLUDE_TUNINGS_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <array>

namespace Tunings
{
    const double MIDI_0_FREQ=8.17579891564371; // or 440.0 * pow( 2.0, - (69.0/12.0 ) )

    /**
     * A Tone is a single entry in an SCL file. It is expressed either in cents or in
     * a ratio, as described in the SCL documentation. 
     *
     * In most normal use, you will not use this class, and it will be internal to a Scale
     */
    struct Tone
    {
        typedef enum Type
        {
            kToneCents, // An SCL representation like "133.0"
            kToneRatio // An SCL representation like "3/7"
        } Type;
        
        Type type;
        double cents;
        int ratio_d, ratio_n;
        std::string stringRep;
        double floatValue; // cents / 1200 + 1. 
        
        Tone() : type(kToneRatio),
                 cents(0),
                 ratio_d(1),
                 ratio_n(1),
                 stringRep("1/1"),
                 floatValue(1.0)
            {
            }
    };

    /**
     * Given an SCL string like "100.231" or "3/7" set up a Tone
     */
    inline Tone toneFromString(const std::string &t, int lineno=-1);

    /**
     * The Scale is the representation of the SCL file. It contains several key
     * features. Most importantly it has a count and a vector of Tones.
     *
     * In most normal use, you will simply pass around instances of this class
     * to a Tunings::Tuning instance, but in some cases you may want to create
     * or inspect this class yourself. Especially if you are displaying this
     * class to your end users, you may want to use the rawText or count methods.
     */
    struct Scale
    {
        std::string name; // The name in the SCL file. Informational only
        std::string description; // The description in the SCL file. Informational only
        std::string rawText; // The raw text of the SCL file used to create this Scale
        int count; // The number of tones.
        std::vector<Tone> tones; // The tones

        Scale() : name("empty scale"),
                  description(""),
                  rawText(""),
                  count(0)
            {
            }
    };

    /**
     * The KeyboardMapping class represents a KBM file. In most cases, the salient
     * features are the tuningConstantNote and tuningFrequency, which allow you to
     * pick a fixed note in the midi keyboard when retuning. The KBM file can also
     * remap individual keys to individual points in a scale, which kere is done with the
     * keys vector.
     *
     * Just as with Scale, the rawText member contains the text of the KBM file used.
     */
    
    struct KeyboardMapping
    {
        int count;
        int firstMidi, lastMidi;
        int middleNote;
        int tuningConstantNote;
        double tuningFrequency, tuningPitch; // pitch = frequency / MIDI_0_FREQ
        int octaveDegrees;
        std::vector<int> keys; // rather than an 'x' we use a '-1' for skipped keys

        std::string rawText;
        std::string name;
        
        KeyboardMapping() : count(0),
                            firstMidi(0),
                            lastMidi(127),
                            middleNote(60),
                            tuningConstantNote(60),
                            tuningFrequency(MIDI_0_FREQ * 32.0),
                            tuningPitch(32.0),
                            octaveDegrees(12),
                            rawText( "" ),
                            name( "" )
            {
            }

    };

    /**
     * In some failure states, the tuning library will throw an exception of
     * type TuningError with a descriptive message.
     */
    class TuningError : public std::exception {
    public:
        TuningError(std::string m) : whatv(m) { }
        virtual const char* what() const noexcept override { return whatv.c_str(); }
    private:
        std::string whatv;
    };

    /**
     * readSCLFile returns a Scale from the SCL File in fname
     */
    Scale readSCLFile(std::string fname);

    /**
     * parseSCLData returns a scale from the SCL file contents in memory
     */
    Scale parseSCLData(const std::string &sclContents);

    /**
     * evenTemperament12NoteScale provides a utility scale which is
     * the "standard tuning" scale
     */
    Scale evenTemperament12NoteScale();

    /**
     * evenDivisionOfSpanByM provides a scale referd to as "ED2-17" or 
     * "ED3-24" by dividing the Span into M points. eventDivisionOfSpanByM(2,12)
     * should be the evenTemperament12NoteScale
     */
    Scale evenDivisionOfSpanByM( int Span, int M );
    
    /**
     * readKBMFile returns a KeyboardMapping from a KBM file name
     */
    KeyboardMapping readKBMFile(std::string fname);

    /**
     * parseKBMData returns a KeyboardMapping from a KBM data in memory
     */
    KeyboardMapping parseKBMData(const std::string &kbmContents);

    /**
     * tuneA69To creates a KeyboardMapping which keeps the midi note 69 (A4) set
     * to a constant frequency, given
     */
    KeyboardMapping tuneA69To(double freq);

    /**
     * tuneNoteTo creates a KeyboardMapping which keeps the midi note given is set
     * to a constant frequency, given
     */
    KeyboardMapping tuneNoteTo(int midiNote, double freq);

    /**
     * startScaleOnAndTuneNoteTo generates a KBM where scaleStart is the note 0
     * of the scale, where midiNote is the tuned note, and where feq is the frequency
     */
    KeyboardMapping startScaleOnAndTuneNoteTo(int scaleStart, int midiNote, double freq);

    /**
     * The Tuning class is the primary place where you will interact with this library.
     * It is constructed for a scale and mapping and then gives you the ability to 
     * determine frequencies across and beyond the midi keyboard. Since modulation 
     * can force key number well outside the [0,127] range in some of our synths we 
     * support a midi note range from -256 to + 256 spanning more than the entire frequency
     * space reasonable.
     *
     * To use this class, you construct a fresh instance every time you want to use a
     * different Scale and Keyboard. If you want to tune to a different scale or mapping,
     * just construct a new instance. 
     */
    class Tuning {
    public:
        // The number of notes we pre-compute
        constexpr static int N = 512;

        // Construct a tuning with even temperament and standard mapping
        Tuning();

        /**
         * Construct a tuning for a particular scale, mapping, or for both.
         */
        Tuning( const Scale &s );
        Tuning( const KeyboardMapping &k );
        Tuning( const Scale &s, const KeyboardMapping &k );

        /**
         * These three related functions provide you the information you
         * need to use this tuning.
         *
         * frequencyForMidiNote returns the Frequency in HZ for a given midi
         * note. In standard tuning, FrequencyForMidiNote(69) will be 440
         * and frequencyForMidiNote(60) will be 261.62 - the standard frequencies
         * for A and middle C.
         *
         * frequencyForMidiNoteScaledByMidi0 returns the frequency but with the
         * standard frequency of midi note 0 divided out. So in standard tuning
         * frequencyForMidiNoteScaledByMidi0(0) = 1 and frequencyForMidiNoteScaledByMidi0(60) = 32
         *
         * Finally logScaledFrequencyForMidiNote returns the log base 2 of the scaled frequency.
         * So logScaledFrequencyForMidiNote(0) = 0 and logScaledFrequencyForMidiNote(60) = 5.
         *
         * Both the frequency measures have the feature of doubling when frequency doubles 
         * (or when a standard octave is spanned), whereas the log one increase by 1 per frequency double.
         *
         * Depending on your internal pitch model, one of these three methods should allow you
         * to calibrate your oscillators to the appropriate frequency based on the midi note
         * at hand.
         *
         * The scalePositionForMidiNote returns the space in the logical scale. Note 0 is the root.
         * It has a maxiumum value of count-1. Note that SCL files omit the root internally and so
         * this logical scale position is off by 1 from the index in the tones array of the Scale data.
         */
        double frequencyForMidiNote( int mn ) const;
        double frequencyForMidiNoteScaledByMidi0( int mn ) const;
        double logScaledFrequencyForMidiNote( int mn ) const;
        int scalePositionForMidiNote( int mn ) const;

        // For convenience, the scale and mapping used to construct this are kept as public copies
        Scale scale;
        KeyboardMapping keyboardMapping;
    private:
        std::array<double, N> ptable, lptable;
        std::array<int, N> scalepositiontable;
    };
    
} // namespace Tunings

#include "TuningsImpl.h"

#endif
