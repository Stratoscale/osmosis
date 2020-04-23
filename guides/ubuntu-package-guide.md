This guide will help you to package to right files for the installer.
It's a little handy, but works should work fine.

### Files to run osmosis

 - /etc/init/osmosis.conf - upstrat service decleration (ubuntu 15 and
   less) 
  - /lib/systemd/system/osmosis.service - systemd service
   decleration (newer ubuntu versions)
   - /usr/bin/osmosis - the osmosis binary
   - /var/lib/osmosis - osmosis store directory skeleton
   

### Script files
The script assume he has the files under the running directory.
Every file path changed form '/' to '_'.
The binary files contains the openssl version comapiled to at the end.

_etc_init_osmosis.conf
_lib_systemd_system_osmosis.service
_usr_bin_osmosis_ssl_1.0.1f
_usr_bin_osmosis_ssl_1.1.1
_var_lib_osmosis

### How to get the package
1. _etc_init_osmosis.conf - osmosis git file name - upstart_osmosis.conf
2. _lib_systemd_system_osmosis.service - osmosis git file name - osmosis.servce
3. install osomsis on ubuntu 1404 with openssl1.0.1f and get the _usr_bin_osmosis_ssl_1.0.1f. from /var/lib/osmosis.
4. install osmosis on ubuntu 1804 with openssl1.1.1 and get the _usr_bin_osmosis_ssl_1.1.1 from /var/lib/osmosis.
5. copy from one of them the wmpty directory or create one by yourself.
6. add the install file.
7. tar it

There is script to help you gather the files - ubuntu-osmosis-gather-files.sh.

### How to install osmosis
1. git clone the repo
2. make clean
3. make prepareForCleanBuild
4. make build
5. make install

you will need to install some libs.

### directory content before taring

----| _etc_init_osmosis.conf
----| _etc_init_osmosis.conf
----| _usr_bin_osmosis_ssl_1.0.1f
----| _usr_bin_osmosis_ssl_1.1.1
----| _var_lib_osmosis/
----| install_osmosis_ubuntu.sh


### Install guide
1. create new directory
2. copy the tar to it
3. extract the files
4. run the script ./install_osmosis_ubuntu.sh
5. check osmosis ( osmosis listlabels and service status)
6. if works - delete the tar and the folder
