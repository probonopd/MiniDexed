// -*-c++-*-
/**
 * TuningsImpl.h
 * Copyright 2019-2020 Paul Walker
 * Released under the MIT License. See LICENSE.md
 * 
 * This contains the nasty nitty gritty implementation of the api in Tunings.h. You probably
 * don't need to read it unless you have found and are fixing a bug, are curious, or want
 * to add a feature to the API. For usages of this library, the documentation in Tunings.h and
 * the usages in tests/all_tests.cpp should provide you more than enough guidance.
 */

#ifndef __INCLUDE_TUNINGS_IMPL_H
#define __INCLUDE_TUNINGS_IMPL_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <sstream>
#include <cctype>

namespace Tunings
{
    inline double locale_atof(const char* s)
    {
        double result = 0;
        std::istringstream istr(s);
        istr.imbue(std::locale("C"));
        istr >> result;
        return result;
    }

    inline Tone toneFromString(const std::string &line, int lineno)
    {
        Tone t;
        t.stringRep = line;
        if (line.find(".") != std::string::npos)
        {
            t.type = Tone::kToneCents;
            t.cents = locale_atof(line.c_str());
        }
        else
        {
            t.type = Tone::kToneRatio;
            auto slashPos = line.find("/");
            if (slashPos == std::string::npos)
            {
                t.ratio_n = atoi(line.c_str());
                t.ratio_d = 1;
            }
            else
            {
                t.ratio_n = atoi(line.substr(0, slashPos).c_str());
                t.ratio_d = atoi(line.substr(slashPos + 1).c_str());
            }
            
            if( t.ratio_n == 0 || t.ratio_d == 0 )
            {
                std::string s = "Invalid Tone in SCL file.";
                if( lineno >= 0 )
                    s += "Line " + std::to_string(lineno) + ".";
                s += " Line is '" + line + "'.";
                throw TuningError(s);
            }
            // 2^(cents/1200) = n/d
            // cents = 1200 * log(n/d) / log(2)
            
            t.cents = 1200 * log(1.0 * t.ratio_n/t.ratio_d) / log(2.0);
        }
        t.floatValue = t.cents / 1200.0 + 1.0;
        return t;
    }
    
    inline Scale scaleFromStream(std::istream &inf)
    {
        std::string line;
        const int read_header = 0, read_count = 1, read_note = 2, trailing = 3;
        int state = read_header;

        Scale res;
        std::ostringstream rawOSS;
        int lineno = 0;
        while (std::getline(inf, line))
        {
            rawOSS << line << "\n";
            lineno ++;

            if (line[0] == '!')
            {
                continue;
            }
            switch (state)
            {
            case read_header:
                res.description = line;
                state = read_count;
                break;
            case read_count:
                res.count = atoi(line.c_str());
                state = read_note;
                break;
            case read_note:
                auto t = toneFromString(line, lineno);
                res.tones.push_back(t);
                if( (int)res.tones.size() == res.count )
                    state = trailing;

                break;
            }
        }

        if( ! ( state == read_note || state == trailing ) )
        {
            throw TuningError( "Incomplete SCL file. Found no notes section in the file" );
        }
        
        if( (int)res.tones.size() != res.count )
        {
            std::string s = "Read fewer notes than count in file. Count=" + std::to_string( res.count )
                + " notes array size=" + std::to_string( res.tones.size() );
            throw TuningError(s);

        }
        res.rawText = rawOSS.str();
        return res;
    }

    inline Scale readSCLFile(std::string fname)
    {
        std::ifstream inf;
        inf.open(fname);
        if (!inf.is_open())
        {
            std::string s = "Unable to open file '" + fname + "'";
            throw TuningError(s);
        }

        auto res = scaleFromStream(inf);
        res.name = fname;
        return res;
    }

    inline Scale parseSCLData(const std::string &d)
    {
        std::istringstream iss(d);
        auto res = scaleFromStream(iss);
        res.name = "Scale from Patch";
        return res;
    }

    inline Scale evenTemperament12NoteScale()
    {
        std::string data = R"SCL(! even.scl
!
12 note even temperament
 12
!
 100.0
 200.0
 300.0
 400.0
 500.0
 600.0
 700.0
 800.0
 900.0
 1000.0
 1100.0
 2/1
)SCL";
        return parseSCLData(data);
    }

    inline Scale evenDivisionOfSpanByM( int Span, int M )
    {
        std::ostringstream oss;
        oss.imbue( std::locale( "C" ) );
        oss << "! Automatically generated ED" << Span << "-" << M << " scale\n";
        oss << "Automatically generated ED" << Span << "-" << M << " scale\n";
        oss << M << "\n";
        oss << "!\n";

                    
        double topCents = 1200.0 * log(1.0 * Span) / log(2.0);
        double dCents = topCents / M;
        for( int i=1; i<M; ++i )
            oss << std::fixed << dCents * i << "\n";
        oss << Span << "/1\n";

        return parseSCLData( oss.str() );
    }
    
    inline KeyboardMapping keyboardMappingFromStream(std::istream &inf)
    {
        std::string line;
        
        KeyboardMapping res;
        std::ostringstream rawOSS;
        res.keys.clear();
        
        enum parsePosition {
            map_size = 0,
            first_midi,
            last_midi,
            middle,
            reference,
            freq,
            degree,
            keys,
            trailing
        };
        parsePosition state = map_size;

        int lineno  = 0;
        while (std::getline(inf, line))
        {
            rawOSS << line << "\n";
            lineno ++;
            if (line[0] == '!')
            {
                continue;
            }
            
            if( line == "x" ) line = "-1";
            else if( state != trailing )
            {
                const char* lc = line.c_str();
                bool validLine = line.length() > 0;
                char badChar = '\0';
                while( validLine && *lc != '\0' )
                {
                    if( ! ( *lc == ' ' || std::isdigit( *lc ) || *lc == '.'  || *lc == (char)13 || *lc == '\n' ) )
                    {
                        validLine = false;
                        badChar = *lc;
                    }
                    lc ++;
                }
                if( ! validLine )
                {
                    throw TuningError( "Invalid line " + std::to_string( lineno ) + ". line='" + line + "'. Bad char is '" +
                                       badChar + "/" + std::to_string( (int)badChar ) + "'" );
                }
            }
            
            int i = std::atoi(line.c_str());
            double v = locale_atof(line.c_str());
            
            switch (state)
            {
            case map_size:
                res.count = i;
                break;
            case first_midi:
                res.firstMidi = i;
                break;
            case last_midi:
                res.lastMidi = i;
                break;
            case middle:
                res.middleNote = i;
                break;
            case reference:
                res.tuningConstantNote = i;
                break;
            case freq:
                res.tuningFrequency = v;
                res.tuningPitch = res.tuningFrequency / 8.17579891564371;
                break;
            case degree:
                res.octaveDegrees = i;
                break;
            case keys:
                res.keys.push_back(i);
                if( (int)res.keys.size() == res.count ) state = trailing;
                break;
            case trailing:
                break;
            }
            if( ! ( state == keys || state == trailing ) ) state = (parsePosition)(state + 1);
            if( state == keys && res.count == 0 ) state = trailing;
            
        }

        if( ! ( state == keys || state == trailing ) )
        {
            throw TuningError( "Incomplete KBM file. Ubable to get to keys section of file" );
        }

        if( (int)res.keys.size() != res.count )
        {
            throw TuningError( "Different number of keys than mapping file indicates. Count is "
                               + std::to_string( res.count ) + " and we parsed " + std::to_string( res.keys.size() ) + " keys." );
        }
        
        res.rawText = rawOSS.str();
        return res;
    }

    inline KeyboardMapping readKBMFile(std::string fname)
    {
        std::ifstream inf;
        inf.open(fname);
        if (!inf.is_open())
        {
            std::string s = "Unable to open file '" + fname + "'";
            throw TuningError(s);
        }
        
        auto res = keyboardMappingFromStream(inf);
        res.name = fname;
        return res;
    }
    
    inline KeyboardMapping parseKBMData(const std::string &d)
    {
        std::istringstream iss(d);
        auto res = keyboardMappingFromStream(iss);
        res.name = "Mapping from Patch";
        return res;
    }
    
    inline Tuning::Tuning() : Tuning( evenTemperament12NoteScale(), KeyboardMapping() ) { }
    inline Tuning::Tuning(const Scale &s ) : Tuning( s, KeyboardMapping() ) {}
    inline Tuning::Tuning(const KeyboardMapping &k ) : Tuning( evenTemperament12NoteScale(), k ) {}
    
    inline Tuning::Tuning(const Scale& s, const KeyboardMapping &k)
    {
        scale = s;
        keyboardMapping = k;

        if( s.count <= 0 )
            throw TuningError( "Unable to tune to a scale with no notes. Your scale provided " + std::to_string( s.count ) + " notes." );
            
        
        double pitches[N];

        int posPitch0 = 256 + k.tuningConstantNote;
        int posScale0 = 256 + k.middleNote;
        
        double pitchMod = log(k.tuningPitch)/log(2) - 1;

        int scalePositionOfTuningNote = k.tuningConstantNote - k.middleNote;
        if( k.count > 0 )
            scalePositionOfTuningNote = k.keys[scalePositionOfTuningNote];

        double tuningCenterPitchOffset;
        if( scalePositionOfTuningNote == 0 )
            tuningCenterPitchOffset = 0;
        else
        {
            double tshift = 0;
            double dt = s.tones[s.count -1].floatValue - 1.0;
            while( scalePositionOfTuningNote < 0 )
            {
                scalePositionOfTuningNote += s.count;
                tshift += dt;;
            }
            while( scalePositionOfTuningNote > s.count )
            {
                scalePositionOfTuningNote -= s.count;
                tshift -= dt;
            }

            if( scalePositionOfTuningNote == 0 )
                tuningCenterPitchOffset = -tshift;
            else
                tuningCenterPitchOffset = s.tones[scalePositionOfTuningNote-1].floatValue - 1.0 - tshift;
        }

        for (int i=0; i<N; ++i)
        {
            // TODO: ScaleCenter and PitchCenter are now two different notes.
            int distanceFromPitch0 = i - posPitch0;
            int distanceFromScale0 = i - posScale0;
            
            if( distanceFromPitch0 == 0 )
            {
                pitches[i] = 1;
                lptable[i] = pitches[i] + pitchMod;
                ptable[i] = pow( 2.0, lptable[i] );
                scalepositiontable[i] = scalePositionOfTuningNote % s.count;
#if DEBUG_SCALES
                std::cout << "PITCH: i=" << i << " n=" << i - 256 
                          << " p=" << pitches[i]
                          << " lp=" << lptable[i] 
                          << " tp=" << ptable[i]
                          << " fr=" << ptable[i] * 8.175798915
                          << std::endl;
#endif           
            }
            else 
            {
                /*
                  We used to have this which assumed 1-12
                  Now we have our note number, our distance from the 
                  center note, and the key remapping
                  int rounds = (distanceFromScale0-1) / s.count;
                  int thisRound = (distanceFromScale0-1) % s.count;
                */
                
                int rounds;
                int thisRound;
                int disable = false;
                if( ( k.count == 0 ) )
                {
                    rounds = (distanceFromScale0-1) / s.count;
                    thisRound = (distanceFromScale0-1) % s.count;
                }
                else
                {
                    /*
                    ** Now we have this situation. We are at note i so we
                    ** are m away from the center note which is distanceFromScale0
                    **
                    ** If we mod that by the mapping size we know which note we are on
                    */
                    int mappingKey = distanceFromScale0 % k.count;
                    if( mappingKey < 0 )
                        mappingKey += k.count;
                    int cm = k.keys[mappingKey];
                    int push = 0;
                    if( cm < 0 )
                    {
                        disable = true;
                    }
                    else
                    {
                        push = mappingKey - cm;
                    }
                    rounds = (distanceFromScale0 - push - 1) / s.count;
                    thisRound = (distanceFromScale0 - push - 1) % s.count;
#ifdef DEBUG_SCALES
                    if( i > 296 && i < 340 )
                        std::cout << "MAPPING n=" << i - 256 << " pushes ds0=" << distanceFromScale0 << " cmc=" << k.count << " tr=" << thisRound << " r=" << rounds << " mk=" << mappingKey << " cm=" << cm << " push=" << push << " dis=" << disable << " mk-p-1=" << mappingKey - push - 1 << std::endl;
#endif
                    
                    
                }
                
                if( thisRound < 0 )
                {
                    thisRound += s.count;
                    rounds -= 1;
                }

                if( disable )
                {
                    pitches[i] = 0;
                    scalepositiontable[i] = -1;
                }
                else
                {
                    pitches[i] = s.tones[thisRound].floatValue + rounds * (s.tones[s.count - 1].floatValue - 1.0) - tuningCenterPitchOffset;
                    scalepositiontable[i] = ( thisRound + 1 ) % s.count;
                }

                lptable[i] = pitches[i] + pitchMod;
                ptable[i] = pow( 2.0, pitches[i] + pitchMod );
                
#if DEBUG_SCALES
                if( i > 296 && i < 340 )
                    std::cout << "PITCH: i=" << i << " n=" << i - 256
                              << " ds0=" << distanceFromScale0 
                              << " dp0=" << distanceFromPitch0
                              << " r=" << rounds << " t=" << thisRound
                              << " p=" << pitches[i]
                              << " t=" << s.tones[thisRound].floatValue << " " << s.tones[thisRound ].cents
                              << " dis=" << disable
                              << " tp=" << ptable[i]
                              << " fr=" << ptable[i] * 8.175798915
                              << " tcpo=" << tuningCenterPitchOffset
                        
                        //<< " l2p=" << log(otp)/log(2.0)
                        //<< " l2p-p=" << log(otp)/log(2.0) - pitches[i] - rounds - 3
                              << std::endl;
#endif           
            }
        }
    }

    inline double Tuning::frequencyForMidiNote( int mn ) const {
        auto mni = std::min( std::max( 0, mn + 256 ), N-1 );
        return ptable[ mni ] * MIDI_0_FREQ;
    }

    inline double Tuning::frequencyForMidiNoteScaledByMidi0( int mn ) const {
        auto mni = std::min( std::max( 0, mn + 256 ), N-1 );
        return ptable[ mni ];
    }

    inline double Tuning::logScaledFrequencyForMidiNote( int mn ) const {
        auto mni = std::min( std::max( 0, mn + 256 ), N-1 );
        return lptable[ mni ];
    }

    inline int Tuning::scalePositionForMidiNote( int mn ) const {
        auto mni = std::min( std::max( 0, mn + 256 ), N-1 );
        return scalepositiontable[ mni ];
    }

    inline KeyboardMapping tuneA69To(double freq)
    {
        return tuneNoteTo( 69, freq );
    }

    inline KeyboardMapping tuneNoteTo( int midiNote, double freq )
    {
        return startScaleOnAndTuneNoteTo( 60, midiNote, freq );
    }

    inline KeyboardMapping startScaleOnAndTuneNoteTo( int scaleStart, int midiNote, double freq )
    {
        std::ostringstream oss;
        oss.imbue( std::locale( "C" ) );
        oss << "! Automatically generated mapping, tuning note " << midiNote << " to " << freq << " hz\n"
            << "!\n"
            << "! Size of Map\n"
            << 0 << "\n"
            << "! First and Last Midi Notes to map - map the entire keyboard\n"
            << 0 << "\n" << 127 << "\n"
            << "! Middle note where the first entry in the scale is mapped.\n"
            << scaleStart << "\n"
            << "! Reference not where frequency is fixed\n"
            << midiNote << "\n"
            << "! Frequency for midi note " << midiNote << "\n"
            << freq << "\n"
            << "! Scale degree for formal octave. This is am empty mapping so:\n"
            << 0 << "\n"
            << "! Mapping. This is an empty mapping so list no keys\n";
        
        return parseKBMData( oss.str() );
    }

}
#endif
