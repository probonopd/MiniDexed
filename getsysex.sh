#!/bin/sh

# Get voices from
# https://yamahablackboxes.com/collection/yamaha-dx7-synthesizer/patches/

CURL_OPTIONS="-L --connect-timeout 15 --max-time 120 --retry 3 --retry-delay 5 --show-error"
ALLOW_INSECURE_SSL="true"

TARGET_DIR="sysex/voice/"

# Add here the links you wish to download from, at the bottom of the list
# First put the link to the SysEx file you wish to download, followed by a space and then the filename on Minidexed
# Please note that files on Minidexed need to start with a number, in this case the next link should be 000028_something.syx
LINKS=()
LINKS+=("https://yamahablackboxes.com/patches/dx7/factory/rom1a.syx 00001_rom1a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/factory/rom1b.syx 00002_rom1b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/factory/rom2a.syx 00003_rom2a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/factory/rom2b.syx 00004_rom2b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc101a.syx 00005_vrc101a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc101b.syx 00006_vrc101b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc102a.syx 00007_vrc102a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc102b.syx 00008_vrc102b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc103a.syx 00009_vrc103a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc103b.syx 00010_vrc103b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc104a.syx 00011_vrc104a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc104b.syx 00012_vrc104b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc105a.syx 00013_vrc105a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc105b.syx 00014_vrc105b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc106a.syx 00015_vrc106a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc106b.syx 00016_vrc106b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc107a.syx 00017_vrc107a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc107b.syx 00018_vrc107b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc108a.syx 00019_vrc108a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc108b.syx 00020_vrc108b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc109a.syx 00021_vrc109a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc109b.syx 00022_vrc109b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc110a.syx 00023_vrc110a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc110b.syx 00024_vrc110b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc111a.syx 00025_vrc111a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc111b.syx 00026_vrc111b.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc112a.syx 00027_vrc112a.syx")
LINKS+=("https://yamahablackboxes.com/patches/dx7/vrc/vrc112b.syx 00028_vrc112b.syx")
# LINKS+=("https://linkToWebsite.com/something.syx 00029_something.syx")

mkdir -p "$TARGET_DIR"

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

declare -i count
count=1
# Download all the files in the list
for i in "${LINKS[@]}"; 
do
    LINK=`echo "${i}" | awk '{print $1}'`
    FILE=`echo "${i}" | awk '{print $2}'`
    echo "Downloading ${LINK} ..."
    curl -o "${TARGET_DIR}${FILE}" ${CURL_OPTIONS} "${LINK}" > /dev/null 2>&1
    if (( $? > 0 ))
    then
        echo "Download failed"
    else
        count+=1
    fi
done

# Download the user bank
echo "Downloading https://github.com/donluca/MiniDexed/raw/getsysex.sh/userBank.syx ..."
printf -v j "%05d" $count
curl -o "${TARGET_DIR}${j}_userBank.syx" ${CURL_OPTIONS} https://github.com/donluca/MiniDexed/raw/getsysex.sh/userBank.syx > /dev/null 2>&1
if (( $? > 0 ))
then
    echo "Download failed"
fi
