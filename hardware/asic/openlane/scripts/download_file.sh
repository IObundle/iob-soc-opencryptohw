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

FILEID=$1
FILENAME=$2
wget --load-cookies /tmp/cookies.txt "https://docs.google.com/uc?export=download&confirm=$(wget --quiet --save-cookies /tmp/cookies.txt --keep-session-cookies --no-check-certificate 'https://docs.google.com/uc?export=download&id=$FILEID' -O- | sed -rn 's/.*confirm=([0-9A-Za-z_]+).*/\1\n/p')&id=$FILEID" -O $FILENAME && rm -rf /tmp/cookies.txt
