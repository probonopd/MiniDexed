#!/bin/sh

# This script creates a set of ini files from the *.override files
# to provide customized configurations for well-known hardware

# Find all files named *.override, and run the following on each of them
for file in *.override; do
    # Copy the file minidexed.ini to the name of this file but with .ini extension instead
    name_of_ini_file=minidexed_$(echo "$file" | sed 's/\.override$/.ini/')
    cp ../src/minidexed.ini "$name_of_ini_file"

    # Change the values in the ini file, leaving the rest of the file unchanged
    while IFS='=' read -r key value; do
        # Skip empty lines and comments
        if [ -z "$key" ] || [ "${key#\#}" != "$key" ]; then
            continue
        fi
        value=$(echo "$value" | tr -d '\r')
        if [ -n "$value" ]; then
            sed -i "s/^$key=.*/$key=$value/" "$name_of_ini_file"
        fi
    done < "$file"

    # Process the last line of the override file separately, if it doesn't end with a newline
    if [ -n "$key" ]; then
        value=$(echo "$value" | tr -d '\r')
        if [ -n "$value" ]; then
            sed -i "s/^$key=.*/$key=$value/" "$name_of_ini_file"
        fi
    fi

    # Configure genxnoise_desktop_module as USB gadget (as intended by the manufacturer)
    case "$file" in
      *genxnoise_desktop_module*)
        echo "" >> "$name_of_ini_file"
        echo "# CAUTION: To prevent hardware damage, DO NOT use the port labeled 'PWR'" >> "$name_of_ini_file"
        echo "# (the microUSB port near the edge of the device) when USBGadget is set to 1!" >> "$name_of_ini_file"
        echo "# You need to disable USBGadget if you would like to use that port!" >> "$name_of_ini_file"
        echo "# See https://github.com/probonopd/MiniDexed/wiki/Hardware#usb-gadget-mode for more information" >> "$name_of_ini_file"
        echo "USBGadget=1" >> "$name_of_ini_file"
        ;;
    esac

    echo "Created $name_of_ini_file"
done
