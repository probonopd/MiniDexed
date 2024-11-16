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

    echo "Created $name_of_ini_file"
done
