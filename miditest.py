#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Send MIDI data to a MIDI device using the `python-rtmidi` library.

try:
    import rtmidi
except ImportError:
    print("Please install the python-rtmidi library: pip install python-rtmidi")
    exit(1)
import time
import sys
import argparse
import os
import signal
import atexit

def signal_handler(sig, frame):
    print("\nExiting...")
    sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)

def cleanup_all_notes_off():
    try:
        midiout = rtmidi.MidiOut()
        ports = midiout.get_ports()
        if ports:
            midiout.open_port(0)
            all_notes_off(midiout, verbose=True)
            time.sleep(0.5)
            midiout.close_port()
    except Exception as e:
        print(f"Cleanup error: {e}")

atexit.register(cleanup_all_notes_off)

# Ask the user which MIDI port to use
def get_midi_port(midiout):
    print("Available MIDI output ports:")
    ports = midiout.get_ports()
    for i, port in enumerate(ports):
        print(f"{i}: {port}")
    while True:
        try:
            choice = int(input("Select a port by number: "))
            if 0 <= choice < len(ports):
                return choice
            else:
                print("Invalid choice. Please select a valid port number.")
        except ValueError:
            print("Invalid input. Please enter a number.")

def play_chord(midiout, notes, velocity=100, channel=0, delay=0.25, verbose=True):
    print("Playing chord:", notes)
    for note in notes:
        midiout.send_message([0x90 | channel, note, velocity])
        time.sleep(delay)
    time.sleep(0.5)
    for note in notes:
        midiout.send_message([0x80 | channel, note, 0])

def send_sysex(midiout, msg, verbose=True, label=None):
    midiout.send_message(msg)
    if verbose:
        if label:
            print(f"Sent: {label} {msg}")
        else:
            print(f"Sent: {msg}")

def all_notes_off(midiout, verbose=True):
    # Send All Notes Off (CC 123) and individual Note Off for all notes 0-127 on all 16 MIDI channels
    for channel in range(16):
        midiout.send_message([0xB0 | channel, 120, 0])
        if verbose:
            print(f"Sent: All Sound Off (CC120) on channel {channel+1}")

def send_modwheel(midiout, value, channel=0, verbose=True):
    # Modulation wheel is CC 1
    midiout.send_message([0xB0 | channel, 1, value])
    if verbose:
        print(f"Sent: ModWheel CC1 value {value} on channel {channel+1}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Send MIDI messages to a device.")
    parser.add_argument("-p", "--port", type=str, help="MIDI output port name or number")
    # Always verbose, so remove the verbose switch and set verbose to True
    args = parser.parse_args()
    verbose = True

    midiout = rtmidi.MidiOut()
    ports = midiout.get_ports()

    if args.port is not None:
        # Allow port to be specified by number or name
        try:
            port_index = int(args.port)
        except ValueError:
            # Try to find by name
            port_index = next((i for i, p in enumerate(ports) if args.port in p), None)
            if port_index is None:
                print(f"Port '{args.port}' not found.")
                sys.exit(1)
    else:
        port_index = get_midi_port(midiout)

    if verbose:
        print(f"Using MIDI output port: {ports[port_index]}")

    try:
        midiout.open_port(port_index)
        chord_notes = [60, 64, 67]
        # Send all notes off before starting the test program
        all_notes_off(midiout, verbose=verbose)
        while True:
            send_sysex(midiout, [0xF0, 0x43, 0x10, 0x04, 0x05, 0x00, 0xF7], verbose, "Portamento Time 0")
            play_chord(midiout, chord_notes, verbose=verbose)
            send_sysex(midiout, [0xF0, 0x43, 0x10, 0x04, 0x05, 0x02, 0xF7], verbose, "Portamento Time 2")
            time.sleep(1)

            # Test Detune
            detune_msgs = [
                ([0xF0, 0x43, 0x10, 0x04, 0x40, 0x2F, 0xF7], "Detune TG1"),
                ([0xF0, 0x43, 0x11, 0x04, 0x40, 0x37, 0xF7], "Detune TG2"),
                ([0xF0, 0x43, 0x12, 0x04, 0x40, 0x40, 0xF7], "Detune TG3"),
                ([0xF0, 0x43, 0x13, 0x04, 0x40, 0x48, 0xF7], "Detune TG4"),
            ]
            for msg, label in detune_msgs:
                send_sysex(midiout, msg, verbose, label)
            play_chord(midiout, chord_notes, verbose=verbose)
            for ch in range(0x10, 0x14):
                send_sysex(midiout, [0xF0, 0x43, ch, 0x04, 0x40, 0x40, 0xF7], verbose, f"Detune TG{ch-0x0F} reset")
            time.sleep(1)

            # Test Poly and Mono modes
            send_sysex(midiout, [0xF0, 0x43, 0x13, 0x04, 0x02, 0x00, 0xF7], verbose, "TG4 Poly")
            play_chord(midiout, chord_notes, verbose=verbose)
            send_sysex(midiout, [0xF0, 0x43, 0x13, 0x04, 0x02, 0x01, 0xF7], verbose, "TG4 Mono")
            time.sleep(1)

            # Test ModWheel sensitivity
            send_sysex(midiout, [0xF0, 0x43, 0x13, 0x04, 0x09, 0x01, 0xF7], verbose, "TG4 ModWheel Sensitivity ON")
            send_modwheel(midiout, 127, channel=0, verbose=verbose)
            play_chord(midiout, chord_notes, verbose=verbose)
            send_modwheel(midiout, 0, channel=0, verbose=verbose)
            send_sysex(midiout, [0xF0, 0x43, 0x13, 0x04, 0x09, 0x00, 0xF7], verbose, "TG4 ModWheel Sensitivity OFF")
            send_modwheel(midiout, 127, channel=0, verbose=verbose)
            play_chord(midiout, chord_notes, verbose=verbose)
            send_modwheel(midiout, 0, channel=0, verbose=verbose)
            time.sleep(1)

            # Disable all Tone Generators except TG1
            send_sysex(midiout, [0xF0, 0x43, 0x13, 0x04, 0x08, 0x00, 0xF7], verbose, "Disable TG2, TG3, TG4")
            play_chord(midiout, chord_notes, verbose=verbose)
            # Enable all Tone Generators
            send_sysex(midiout, [0xF0, 0x43, 0x13, 0x04, 0x08, 0x01, 0xF7], verbose, "Enable TG2, TG3, TG4")
            play_chord(midiout, chord_notes, verbose=verbose)
            time.sleep(1)

            # Send all notes off after each test program loop
            all_notes_off(midiout, verbose=verbose)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)