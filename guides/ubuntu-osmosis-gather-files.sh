#!/bin/sh

# avilable on osmosis git
cp /etc/init/osmosis.conf ./_etc_init_osmosis.conf && echo "bring /etc/init/osmosis" || echo "fail to locate /etc/init/osmosis - maybe not upstart linux? you can bring the file from git"

# avilable on osmosis git
cp /lib/systemd/system/osmosis.service ./_lib_systemd_system_osmosis.service && echo "bring /lib/systemd/system/osmosis.service" || echo "fail to locate /lib/systemd/system/osmosis.service - maybe not systemd machine? "

# from current running machine
cp /usr/bin/osmosis ./_usr_bin_osmosis && echo "bring /usr/bin/osmosis" || echo "fail to locate /usr/bin/osmosis - is osmosis installed?"

# from current running machine - prefer empty
cp -r /var/lib/osmosis ./_var_lib_osmosis && echo "bring /var/lib/osmosis" || echo "fail to locate /var/lib/osmosis - is osmosis installed?"


echo "tar this files with the install_osmosis_ubuntu.sh from git"
echo "don't forget to change to _usr_bin_osmosis to current ssl"
