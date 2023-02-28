#!/bin/bash

# Get voices from
# https://yamahablackboxes.com/collection/yamaha-dx7-synthesizer/patches/

CURL_OPTIONS="-L --connect-timeout 15 --max-time 120 --retry 3 --retry-delay 5 --show-error"
ALLOW_INSECURE_SSL="true"

# Add here the links you wish to download from, at the bottom of the list
# First put the link to the SysEx file you wish to download, followed by a space and then the filename on Minidexed
# Please note that files on Minidexed need to start with a number, in this case the next link should be 000028_something.syx
LINKS=()
LINKS+=("https://yamahablackboxes.com/patches/dx7/factory/rom1a.syx 000000_rom1a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/factory/rom1b.syx 000001_rom1b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/factory/rom2a.syx 000002_rom2a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/factory/rom2b.syx 000003_rom2b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc101a.syx 000004_vrc101a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc101b.syx 000005_vrc101b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc102a.syx 000006_vrc102a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc102b.syx 000007_vrc102b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc103a.syx 000008_vrc103a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc103b.syx 000009_vrc103b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc104a.syx 000010_vrc104a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc104b.syx 000011_vrc104b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc105a.syx 000012_vrc105a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc105b.syx 000013_vrc105b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc106a.syx 000014_vrc106a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc106b.syx 000015_vrc106b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc107a.syx 000016_vrc107a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc107b.syx 000017_vrc107b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc108a.syx 000018_vrc108a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc108b.syx 000019_vrc108b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc109a.syx 000020_vrc109a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc109b.syx 000021_vrc109b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc110a.syx 000022_vrc110a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc110b.syx 000023_vrc110b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc111a.syx 000024_vrc111a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc111b.syx 000025_vrc111b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc112a.syx 000026_vrc112a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc112b.syx 000027_vrc112b.syx")
# LINKS+=("https://linkToWebsite.com/something.syx 000028_something.syx")

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
for i in "${LINKS[@]}"; 
do
    LINK=`echo "${i}" | awk '{print $1}'`
    FILE=`echo "${i}" | awk '{print $2}'`
    echo "Downloading ${LINK} ..."
    curl -o "sysex/voice/${FILE}" ${CURL_OPTIONS} "${LINK}" > /dev/null 2>&1
    if (( $? > 0 ))
    then
        echo "Download failed"
    fi
done
