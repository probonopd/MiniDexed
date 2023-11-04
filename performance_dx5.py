import os

performance_names = ["DOUBLE HORN SECTION (DETUNED)",
"FANFARE TRUMPETS 1-2",
"FULL SYNTH BRASS (DETUNED)",
"TIGHT BRASS SECTION",
"SYNTH BRASS",
"SYNTH BRASS [F/C CHORUS]",
"С580 BRASS [F/C VIBRATO]",
"STRINGS & BRASS ENSEMBLE",
"CELLO QUARTET",
"VIOLIN ENSEMBLE",
"ENSEMBLE [L]/SOLO VIOLIN [R]",
"STRING ORCHESTRA",
"HIGH STRINGS (ANALOG TYPE)",
"CELLOS & VIOLINS",
"STRING ENSEMBLE [F/C VIBRATO]",
"STRINGS & VELOCITY TRUMPETS",
"ACOUSTIC GRAND PIANO 1",
"ACOUSTIC GRAND PIANO 2",
"ELECTRIC GRAND PIANO",
"ELECTRIC PIANO [M/W TREMOLO]",
"ELECTRIC PIANO (BRIGHT TINE}",
"DIRTY ELECTRIC PIANO",
"CLAV.ENSEMBLE",
"GRAND HARPSICHORD",
"PIPE ORGAN",
"JAZZ ORGAN",
"ROCK ORGAN WITH OLD TONE CAB",
"E.PIANO [L]/JAZZ GUITAR [R]",
"ELEC. BASS",
"DOUBLE HARPS",
"AFRICAN MALLETS",
"VIBRAPHONE",
"ELECTRIC PIANO & BRASS [BC1]",
"ELECTRIC GRAND & BRASS [BC1]",
"ELECTRIC PIANO & SAX [ВС1]",
"ELEC. PIANO & CLAV ENSEMBLE",
"ELECTRIC PIANO & STRINGS",
"HARPSICHORD & STRING ENSEMBLE",
"FULL ORCHESTRA",
"RIDE CYMBAL",
"KICK DRUM [L]/SNARE [R]",
"НІ-НАТ CLOSING: [L/CYMBAL [R]",
"НАND CLAPS [L]/TOM TOMS [R]",
"LOG DRUMS [LI/ROTO TOMS [9]",
"TAMBOURINE [L]/TIMBALI [R]",
"COWBELL [L]/WOOD BLOCK [R]",
"FRETLESS BASS [L]/SAX [ВС1] [R]",
"ACOUSTIC PIANO [L]/FLUTE [R]",
"SYNTHESIZER UPRISING",
"SAMPLE 8 HOLD [L]/LEAD LINE [R]",
"POLY SYNTH [L]/LEAD SYNTH [R]",
"PERCUSSIVE SYNTH",
"TOY MUSIC BOX",
"FM ENSEMBLE",
"PLANET OF ICE",
"MALE & FEMALE CHOIR",
"\"BIG BEN\" [L]/TUNED BELLS [R]",
"GLASS WIND CHIMES",
"JUNGLE NOISE (GROWL/BIRDS)",
"SIDE TO SIDE",
"TRAFFIC",
"FLOATING CLOUDS",
"COMBAT (EXPLOSION [L]; GUNS [R])",
"BOMBS AWAY!!"]

filename_a = "/tmp/user/_home_user_Downloads_DX5 Carts.zip/DX5A1.SYX"
with open(filename_a, "rb") as file:
    data_a = file.read()
basename_a = filename_a.split("/")[-1]

filename_b = "/tmp/user/_home_user_Downloads_DX5 Carts.zip/DX5B1.SYX"
with open(filename_b, "rb") as file:
    data_b = file.read()
basename_b = filename_b.split("/")[-1]

# Remove the first 6 bytes (the header) from the data, this is the sysex header
data_a = data_a[6:]
data_b = data_b[6:]

# Put the data into a list of 128-byte ones, but only the first 32 ones
voices_a1 = [data_a[i:i+128] for i in range(0, 128*32, 128)]
voices_b1 = [data_b[i:i+128] for i in range(0, 128*32, 128)]
voicenames_a = []
voicenames_b = []
i = 0
for voice_a in voices_a1:
    i += 1
    voicenames_a.append(voice_a[-10:].decode("ascii"))
for voice_b in voices_b1:
    i += 1
    voicenames_b.append(voice_b[-10:].decode("ascii"))

for i in range(0, 32):
    print("* %i `%s`: `%s` `%s` + `%s` `%s`" % (i+1, performance_names[i], basename_a, voicenames_a[i], basename_b, voicenames_b[i]))

filename_a = "/tmp/user/_home_user_Downloads_DX5 Carts.zip/DX5A2.SYX"
with open(filename_a, "rb") as file:
    data_a = file.read()
basename_a = filename_a.split("/")[-1]

filename_b = "/tmp/user/_home_user_Downloads_DX5 Carts.zip/DX5B2.SYX"
with open(filename_b, "rb") as file:
    data_b = file.read()
basename_b = filename_b.split("/")[-1]

# Remove the first 6 bytes (the header) from the data, this is the sysex header
data_a = data_a[6:]
data_b = data_b[6:]

# Put the data into a list of 128-byte ones, but only the first 32 ones
voices_a2 = [data_a[i:i+128] for i in range(0, 128*32, 128)]
voices_b2 = [data_b[i:i+128] for i in range(0, 128*32, 128)]
voicenames_a = []
voicenames_b = []
i = 0
for voice_a in voices_a2:
    voicenames_a.append(voice_a[-10:].decode("ascii"))
for voice_b in voices_b2:
    voicenames_b.append(voice_b[-10:].decode("ascii"))

for i in range(0, 32):
    print("* %i `%s`: `%s` `%s` + `%s` `%s`" % (i+33, performance_names[i+32], basename_a, voicenames_a[i], basename_b, voicenames_b[i]))


# Create the performances based on the data from the syx

for i in range(0, 63):
    output_lines = []
    if i < 32:
        voice_data_a = voices_a1[i].hex().upper()
    else:
        voice_data_a = voices_a2[i-32].hex().upper()
    # Add a blank space after every 2 characters
    voice_data_a = " ".join(voice_data_a[i:i+2] for i in range(0, len(voice_data_a), 2))
    print("voice_data_a: %s" % voice_data_a)

    if i < 32:
        voice_data_b = voices_b1[i].hex().upper()
    else:
        voice_data_b = voices_b2[i-32].hex().upper()
    # Add a blank space after every 2 characters
    voice_data_b = " ".join(voice_data_b[i:i+2] for i in range(0, len(voice_data_b), 2))
    print("voice_data_b: %s" % voice_data_b)

    # output_lines.append("[Performance]")
    # output_lines.append("Name=" + performance_names[i])

    output_lines.append("BankNumber1=1")
    output_lines.append("VoiceNumber1=1")
    output_lines.append("MIDIChannel1=1")
    output_lines.append("Volume1=49")
    output_lines.append("Pan1=0")
    output_lines.append("Detune1=-3")
    output_lines.append("Cutoff1=99")
    output_lines.append("Resonance1=0")
    output_lines.append("NoteLimitLow1=0")
    output_lines.append("NoteLimitHigh1=127")
    output_lines.append("NoteShift1=0")
    output_lines.append("ReverbSend1=99")
    output_lines.append("PitchBendRange1=2")
    output_lines.append("PitchBendStep1=0")
    output_lines.append("PortamentoMode1=0")
    output_lines.append("PortamentoGlissando1=0")
    output_lines.append("PortamentoTime1=0")
    output_lines.append("VoiceData1=" + voice_data_a)
    output_lines.append("MonoMode1=0")
    output_lines.append("ModulationWheelRange1=99")
    output_lines.append("ModulationWheelTarget1=1")
    output_lines.append("FootControlRange1=99")
    output_lines.append("FootControlTarget1=0")
    output_lines.append("BreathControlRange1=99")
    output_lines.append("BreathControlTarget1=0")
    output_lines.append("AftertouchRange1=99")
    output_lines.append("AftertouchTarget1=0")

    output_lines.append("BankNumber2=2")
    output_lines.append("VoiceNumber2=2")
    output_lines.append("MIDIChannel2=1")
    output_lines.append("Volume2=49")
    output_lines.append("Pan2=127")
    output_lines.append("Detune2=3")
    output_lines.append("Cutoff2=99")
    output_lines.append("Resonance2=0")
    output_lines.append("NoteLimitLow2=0")
    output_lines.append("NoteLimitHigh2=127")
    output_lines.append("NoteShift2=0")
    output_lines.append("ReverbSend2=99")
    output_lines.append("PitchBendRange2=2")
    output_lines.append("PitchBendStep2=0")
    output_lines.append("PortamentoMode2=0")
    output_lines.append("PortamentoGlissando2=0")
    output_lines.append("PortamentoTime2=0")
    output_lines.append("VoiceData2=" + voice_data_b)
    output_lines.append("MonoMode2=0")
    output_lines.append("ModulationWheelRange2=99")
    output_lines.append("ModulationWheelTarget2=1")
    output_lines.append("FootControlRange2=99")
    output_lines.append("FootControlTarget2=0")
    output_lines.append("BreathControlRange2=99")
    output_lines.append("BreathControlTarget2=0")
    output_lines.append("AftertouchRange2=99")
    output_lines.append("AftertouchTarget2=0")

    six_digits_number = "%06d" % (i+1+130)

    performance_name = performance_names[i]
    performance_name = performance_name.replace(" ", "_")
    performance_name = performance_name.replace("(", "")
    performance_name = performance_name.replace(")", "")
    performance_name = performance_name.replace("/", "_")
    performance_name = performance_name.replace("[", "")
    performance_name = performance_name.replace("]", "")
    performance_name = six_digits_number + "_"  + performance_name.replace(" ", "_")[:10] + ".ini"

    print("")
    print("performance_name: %s" % performance_name)

    for lines in output_lines:
        print(lines)

    if not os.path.exists("/tmp/performance"):
        os.makedirs("/tmp/performance")

    # Write the file
    with open("/tmp/performance/" + performance_name, "w") as file:
        file.write("\n".join(output_lines))