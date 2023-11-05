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

# The following was determined by looking at the sysex data
# from https://github.com/bladeSk/DX7II-Librarian/issues/3
# in https://bladesk.github.io/DX7II-Librarian/
modes = ["dual",
"dual",
"dual",
"dual",
"dual",
"dual",
"dual",
"split",
"split",
"split",
"split",
"split",
"split",
"split",
"split",
"split",
"dual",
"split",
"split",
"dual",
"dual",
"dual",
"dual",
"dual",
"split",
"dual",
"split",
"dual",
"split",
"dual",
"split",
"dual",
"dual",
"dual",
"dual",
"dual",
"dual",
"dual",
"dual",
"split",
"split",
"split",
"split",
"split",
"split",
"split",
"split",
"split",
"dual",
"split",
"split",
"dual",
"dual",
"dual",
"dual",
"dual",
"split",
"dual",
"split",
"dual",
"split",
"dual",
"split",
"dual"]

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

# Put the data into a list of 128-byte voices, but only the first 32 ones
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

def vmem2vced(vmem):
    # Borrowed from
    # https://github.com/rarepixel/dxconvert/blob/master/DXconvert/dx7.py
    if vmem == [vmem[0]] * 128:
        return initvced()
    vced=[0]*155
    for op in range(6):
        for p in range(11):
            vced[p+21*op] = vmem[p+17*op]&127
        vced[11+21*op] = vmem[11+17*op]&0b11
        vced[12+21*op] = (vmem[11+17*op]&0b1100)>>2
        vced[13+21*op] = vmem[12+17*op]&0b111
        vced[20+21*op] = (vmem[12+17*op]&0b1111000)>>3
        vced[14+21*op] = vmem[13+17*op]&0b11
        vced[15+21*op] = (vmem[13+17*op]&0b11100)>>2
        vced[16+21*op] = vmem[14+17*op]&127
        vced[17+21*op] = vmem[15+17*op]&1
        vced[18+21*op] = (vmem[15+17*op]&0b111110)>>1
        vced[19+21*op] = vmem[16+17*op]&127
    for p in range(102, 110):
        vced[p+24] = vmem[p]&127
    vced[134] = vmem[110]&0b11111
    vced[135] = vmem[111]&0b0111
    vced[136] = (vmem[111]&0b1000)>>3
    for p in range(112, 116):
        vced[p+25] = vmem[p]&127
    vced[141] = vmem[116]&1
    vced[142] = (vmem[116]&0b1110)>>1
    vced[143] = (vmem[116]&0b1110000)>>4
    for p in range(117, 128):
        vced[p+27] = vmem[p]
    for i in range(len(vced)):
        vced[i] = vced[i]&127
    # return vced
    return bytes(vced)

# Create the performances based on the data from the syx

for i in range(0, 64):
    output_lines = []
    if i < 32:
        voice_data_a = vmem2vced(voices_a1[i]).hex().upper()
    else:
        voice_data_a = vmem2vced(voices_a1[i-32]).hex().upper()

    # Add a blank space after every 2 characters
    voice_data_a = " ".join(voice_data_a[i:i+2] for i in range(0, len(voice_data_a), 2))

    if i < 32:
        voice_data_b = vmem2vced(voices_b1[i]).hex().upper()
    else:
        voice_data_b = vmem2vced(voices_b2[i-32]).hex().upper()
    # Add a blank space after every 2 characters
    voice_data_b = " ".join(voice_data_b[i:i+2] for i in range(0, len(voice_data_b), 2))

    # output_lines.append("[Performance]")
    # output_lines.append("Name=" + performance_names[i])
    # output_lines.append("; NOTE: These values are partially guessed and likely wrong")
    # output_lines.append("; If you have DX5 performance data, please let us know")
    output_lines.append("BankNumber1=1")
    output_lines.append("VoiceNumber1=1")
    output_lines.append("MIDIChannel1=1")
    output_lines.append("Volume1=49")
    output_lines.append("Pan1=62")
    output_lines.append("Detune1=-3")
    output_lines.append("Cutoff1=99")
    output_lines.append("Resonance1=0")
    output_lines.append("NoteLimitLow1=0")
    if modes[i] == "dual":
        output_lines.append("NoteLimitHigh1=127")
    elif modes[i] == "split":
        output_lines.append("NoteLimitHigh1=36")
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
    output_lines.append("AftertouchTarget1=1")
    # output_lines.append("")
    output_lines.append("BankNumber2=2")
    output_lines.append("VoiceNumber2=2")
    output_lines.append("MIDIChannel2=1")
    output_lines.append("Volume2=49")
    output_lines.append("Pan2=65")
    output_lines.append("Detune2=3")
    output_lines.append("Cutoff2=99")
    output_lines.append("Resonance2=0")
    if modes[i] == "dual":
        output_lines.append("NoteLimitLow2=0")
    elif modes[i] == "split":
        output_lines.append("NoteLimitLow2=37")
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
    output_lines.append("AftertouchTarget2=1")
    # output_lines.append("")

    # FIXME: Tone generators that are not explicitly configured should be set to 0 in the firmware
    output_lines.append("MIDIChannel3=0")
    output_lines.append("MIDIChannel4=0")
    output_lines.append("MIDIChannel5=0")
    output_lines.append("MIDIChannel6=0")
    output_lines.append("MIDIChannel7=0")
    output_lines.append("MIDIChannel8=0")

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
