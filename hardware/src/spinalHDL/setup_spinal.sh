#!/bin/bash
# setup_spinal.sh
# 
# Script to install tools for SpinalHDL
#
# See also:
#   Spinal HDL Example project: https://github.com/SpinalHDL/SpinalTemplateSbt
#   SBT Install Page: https://www.scala-sbt.org/download.html

# Check for sudo permissions
if ! [ $(id -u) = 0 ];
then
    echo "The script needs to run as root to allow program installation."
    echo "Usage: sudo ./setup_spinal.sh"
    exit 1
fi

echo "Updating repository cache"
sudo apt update

echo "Installing JDK"
sudo apt install -y openjdk-8-jdk

echo "Installing Scala distribution"
sudo apt install -y scala

echo "Installing Scala Build Tool"
echo "deb https://repo.scala-sbt.org/scalasbt/debian all main" | sudo tee /etc/apt/sources.list.d/sbt.list
echo "deb https://repo.scala-sbt.org/scalasbt/debian /" | sudo tee /etc/apt/sources.list.d/sbt_old.list
curl -sL "https://keyserver.ubuntu.com/pks/lookup?op=get&search=0x2EE0EA64E40A89B84B2DF73499E82A75642AC823" | sudo apt-key add
sudo apt-get update
sudo apt-get install -y sbt

echo "SpinalHDL Tools installed"
exit 0
