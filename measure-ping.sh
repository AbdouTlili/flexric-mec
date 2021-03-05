#!/bin/bash

wait_time=1100
folder=random-size

set -x
killall e2sm_hw_fb_ric
killall e2sm_hw_fb_agent
killall e2sm_hw_ric
killall e2sm_hw_agent

cd build
if [ ! -d "$folder" ]; then
  echo "folder $folder does not exist in build/, aborting"
  exit 1
fi
cmake -DCMAKE_BUILD_TYPE=Release -DE2AP_ENCODING=FLATBUFFERS ..
make -j > /dev/null
if [ $? -ne 0 ]; then
  echo "error compiling, Polen offen"
  exit 1
fi
./test/sm_asn1_hw/e2sm_hw_ric | tee $folder/fb.asn1.log &
sleep 1
./test/sm_asn1_hw/e2sm_hw_agent > /dev/null &
sleep $wait_time
kill %2
kill %1
./test/sm_fb_hw/e2sm_hw_fb_ric | tee $folder/fb.fb.log &
sleep 1
./test/sm_fb_hw/e2sm_hw_fb_agent > /dev/null &
sleep $wait_time
kill %4
kill %3

cmake -DCMAKE_BUILD_TYPE=Release -DE2AP_ENCODING=ASN ..
make -j > /dev/null
if [ $? -ne 0 ]; then
  echo "error compiling, Polen offen"
  exit 1
fi
./test/sm_fb_hw/e2sm_hw_fb_ric | tee $folder/asn1.fb.log &
sleep 1
./test/sm_fb_hw/e2sm_hw_fb_agent > /dev/null &
sleep $wait_time
kill %2
kill %1
./test/sm_asn1_hw/e2sm_hw_ric | tee $folder/asn1.asn1.log &
sleep 1
./test/sm_asn1_hw/e2sm_hw_agent > /dev/null &
sleep $wait_time
kill %4
kill %3
