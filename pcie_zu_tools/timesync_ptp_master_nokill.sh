#!/bin/bash
# Last modify date: 2023-1-15

SCRIPT_VERSION=v1.4-20230115

func_clockdiff_exist() {
    which "clockdiff" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo -e "clockdiff not exist, install"
        apt-get install iputils-clockdiff
    fi
}

func_ptp_exist() {
    which "ptpd" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo -e "ptpd not exist, install"
        apt-get install ptpd
    fi
}

echo -e "$0 VER: ${SCRIPT_VERSION}"

#0, check root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

#1, check param
if [ $# -lt 1 ]; then
  echo "Usage: $0 <net-dev>"
  echo "net-dev: net connect with board, check with ifconfig"
  echo " eg1: $0 enp0s31f6"
  exit 1;
fi

#2, check ptp4l & clockdiff
func_ptp_exist
func_clockdiff_exist

#3, clear time app in system
killall ntpd
killall ptpd
#killall ptp4l

#4, run ptp4l
ptp4l -i ${1} -E -S &
#ptpd -C -E -M -i ${1} &

#6, all done
echo -e "\n> 6, all done"
