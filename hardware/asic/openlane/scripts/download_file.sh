#!/usr/bin/env bash
# Source: https://www.24x7serversupport.com/blog/download-google-drive-files-using-wget/
# 1. Go to Google Drive
# 2. Right click file > Share > General Access > Anyone with the link
# 3. Copy share link
#   links have the following structure:
# 		https://drive.google.com/file/d/<FILEID>/view?usp=share_link
# 	For example:
# 		https://drive.google.com/file/d/ABCDEabcde1234/?usp=share_link
# 		Has FILEID = ABCDEabcde1234

ALGORITHM=$1
FILENAME=$2

# Google Drive file IDs
FILEID_SHA256_SYNTH=sha256_synth
FILEID_SHA256_LAYOUT=sha256_layout

FILEID_AES256_SYNTH=aes256_synth
FILEID_AES256_LAYOUT=aes256_layout

FILEID_MCELIECE_SYNTH=mceliece_synth
FILEID_MCELIECE_LAYOUT=mceliece_layout

case $ALGORITHM in
    SHA256)
        if [[ $FILENAME = *synth* ]]
        then
            FILEID=$FILEID_SHA256_SYNTH
        else
            FILEID=$FILEID_SHA256_LAYOUT
        fi
        ;;
    AES256)
        if [[ $FILENAME = *synth* ]]
        then
            FILEID=$FILEID_AES256_SYNTH
        else
            FILEID=$FILEID_AES256_LAYOUT
        fi
        ;;
    *)
        if [[ $FILENAME = *synth* ]]
        then
            FILEID=$FILEID_MCELIECE_SYNTH
        else
            FILEID=$FILEID_MCELIECE_LAYOUT
        fi
        ;;
esac

wget --load-cookies /tmp/cookies.txt "https://docs.google.com/uc?export=download&confirm=$(wget --quiet --save-cookies /tmp/cookies.txt --keep-session-cookies --no-check-certificate 'https://docs.google.com/uc?export=download&id=$FILEID' -O- | sed -rn 's/.*confirm=([0-9A-Za-z_]+).*/\1\n/p')&id=$FILEID" -O $FILENAME && rm -rf /tmp/cookies.txt
