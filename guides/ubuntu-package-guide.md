# Ubuntu package guide!

This guide will help you to package to right files for the installer

### Files to run osmosis

 - /etc/init/osmosis.conf - upstrat service decleration (ubuntu 15 and
   less) 
  - /lib/systemd/system/osmosis.service - systemd service
   decleration (newer ubuntu versions)
   - /usr/bin/osmosis - the osmosis binary
   - /var/lib/osmosis - osmosis store directory skeleton
   
### How to get the files
* service files (systemd / upstart) can be found in osmosis git main directory.

### Script files
Every file is changed form '/' to '_'
The binary files contains the openssl version comapiled to

_etc_init_osmosis.conf
_lib_systemd_system_osmosis.service
_usr_bin_osmosis_ssl_1.0.1f
_usr_bin_osmosis_ssl_1.1.1
_var_lib_osmosis
