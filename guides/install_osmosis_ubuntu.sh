#!/bin/bash

function verify_reqs_and_set_vars(){

    CMD_ERROR=0

    # verify commands
    which lsb_release  || CMD_ERROR=1 &> /dev/null
    which openssl &> /dev/null || CMD_ERROR=1

    if [ "$CMD_ERROR" -eq "1" ]
    then
       echo "ERROR - lsb_release or openssl command not found"
       exit 1
    fi

    # set vars
    OS_RELEASE=`lsb_release -i | awk '{print $3}'`
    UBUNTU_VERSION=`lsb_release -r | awk '{print $2}' | head -c 2`
    SSL_VERSION=`openssl version | awk '{print $2}'`

    if [ "$OS_RELEASE" != "Ubuntu" ]
    then
        echo "ERROR - not ubunto dist"
        exit 2
    fi

    if [ "$SSL_VERSION" != "1.1.1" ] && [ "$SSL_VERSION" != "1.0.1f" ]
    then
        echo "ERROR - not openssl 1.1.1 or 1.0.1f"
        exit 3
    fi

}

function stop_service(){
    if [ "$UBUNTU_VERSION" -gt "15" ]
    then
        sudo systemctl stop osmosis.service || true
    else
        sudo service osmosis stop || true
    fi
}

function copy_var_lib(){
    echo "copy _var_lib_osmosis to /var/lib/osmosis"
    sudo cp -r _var_lib_osmosis/ /var/lib/osmosis
}

function copy_usr_bin(){

    if [ "$SSL_VERSION" = "1.1.1" ]
    then
        echo "copy /usr/bin/osmosis match openssl 1.1.1"
        sudo cp -r _usr_bin_osmosis_ssl_1.1.1 /usr/bin/osmosis
    elif [ "$SSL_VERSION" = "1.0.1f" ]
    then
        echo "copy /usr/bin/osmosis match openssl 1.0.1f"
        sudo cp -r _usr_bin_osmosis_ssl_1.0.1f /usr/bin/osmosis
    else
        echo "ERROR - not found match ssl version"
        exit 6
    fi
}

function ubuntu_update_service(){

    if [ "$UBUNTU_VERSION" -gt "15" ]
    then
        echo "systemd install"
        sudo cp _lib_systemd_system_osmosis.service /lib/systemd/system/osmosis.service
        sudo systemctl daemon-reload
        sudo systemctl disable osmosis.service
        sudo systemctl enable osmosis.service
        sudo systemctl start osmosis.service
	sudo systemctl start osmosis
    else
	echo "upstart install"
        sudo cp _etc_init_osmosis.conf /etc/init/osmosis.conf
	sudo service osmosis start
    fi
}
function disable_network_protection(){
    if [ "$UBUNTU_VERSION" -gt "15" ]
    then
        echo "disabling NetworkManager and firewalld"
        sudo systemctl stop NetworkManager || true
        sudo systemctl disable NetworkManager || true
        sudo systemctl stop firewalld || true
        sudo systemctl disable firewalld || true
    else
        sudo sudo ufw disable
    fi
}
function check_osmosis(){
    osmosis listlabels --objectStores=127.0.0.1:1010
  
    OSMO_RET_CODE=$?

    if [ "$OSMO_RET_CODE" -ne "0" ]
    then
        echo "osmosis is not working"
        exit 3
    else
        echo "osmosis is working locally"
        exit 0
    fi
}

verify_reqs_and_set_vars
stop_service
copy_var_lib
copy_usr_bin
ubuntu_update_service
disable_network_protection
check_osmosis
