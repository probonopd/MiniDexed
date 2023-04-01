#!/bin/bash

# Get voices from
# https://yamahablackboxes.com/collection/yamaha-dx7-synthesizer/patches/

CURL_OPTIONS="-L --connect-timeout 15 --max-time 120 --retry 3 --retry-delay 5 --show-error"
ALLOW_INSECURE_SSL="true"

# Add here the links you wish to download from, at the bottom of the list
LINKS=()
LINKS+=("https://yamahablackboxes.com/patches/dx7/factory/rom1a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/factory/rom1b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/factory/rom2a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/factory/rom2b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc101a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc101b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc102a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc102b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc103a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc103b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc104a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc104b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc105a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc105b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc106a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc106b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc107a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc107b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc108a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc108b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc109a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc109b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc110a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc110b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc111a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc111b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc112a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc112b.syx")
# LINKS+=("https://linkToWebsite.com/something.syx")

mkdir -p sysex/voice/

# Check internet connection, https and if website is up 
curl ${CURL_OPTIONS} -s -I -X POST "https://yamahablackboxes.com" > /dev/null 2>&1
case $? in
    0)
        ;;
    60)
        if [[ "${ALLOW_INSECURE_SSL}" == "true" ]]
        then
            CURL_OPTIONS+=" --insecure"
        else
            echo "Error establishing secure connection"
            exit 2
        fi
        ;;
    *)
        echo "No Internet connection or the website is down"
        exit 1
        ;;
esac

# Download all the files in the list
declare -i count
for i in "${LINKS[@]}"; 
do
    printf -v j "%05d" $count
    FILENAME="$j"_`basename "${i}"`
    echo "Downloading ${i} ..."
    curl -o "sysex/voice/${FILENAME}" ${CURL_OPTIONS} "${i}" > /dev/null 2>&1
    if (( $? > 0 ))
    then
        echo "Download failed"
    else
        count+=1
    fi
done
