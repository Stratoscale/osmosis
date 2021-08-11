#!/bin/bash

allLabels=(mgmt-debs duroslight-rpms __rpms__ discovery-service discovery-client memaslap__dev rootfs_booking
management_server ycsb netperf lbctl__rel __lf__rel lightbox-exporter)

cd /data/labels

echo going to erase labels
for label in "${allLabels[@]}"; do
    echo erasing label: "$label"
    ls | grep "$label" | while read line; do /osmosis.bin eraselabel $line ; done
done
echo finished erasing labels

echo going to purge osmosis, this may take a while...
/osmosis.bin purge --objectStoreRootPath=/data
echo all done! exiting...
