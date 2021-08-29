all: 
	$(MAKE) clean
	$(MAKE) build unittest check_convention CONFIGURATION=DEBUG
	$(MAKE) clean
	$(MAKE) build unittest egg

clean:
	rm -fr build dist osmosis.egg-info

.PHONY: build
build: build/cpp-netlib-0.13.0-final/.unpacked
	$(MAKE) -f build.Makefile

build/cpp-netlib-0.13.0-final/.unpacked: cpp-netlib-0.13.0-final.tar.gz cpp-netlib-0.13.0-boost-1.72.patch
	mkdir build > /dev/null 2> /dev/null || true
	tar -xf $< -C build
	patch -p1 -d build < cpp-netlib-0.13.0-boost-1.72.patch
	touch $@

.PHONY: egg
egg: dist/osmosis-1.0.linux-x86_64.tar.gz

unittest: build
	PYTHONPATH=py python tests/main.py $(TESTS)
	build/cpp/testtaskqueue.bin

check_convention:
	pep8 py tests --max-line-length=109

.PHONY: install_binary
install_binary:
	sudo cp -f build/cpp/osmosis.bin /usr/bin/osmosis

install:
	-sudo systemctl stop osmosis
	-sudo service osmosis stop
	make install_binary
	python py/get_system_setting.py systemManager
	make install_service_`python py/get_system_setting.py systemManager`
	if [ -d "/etc/bash_completion.d" ]; then \
		sudo cp bash.completion.sh /etc/bash_completion.d/osmosis.sh; \
	fi

install_service_systemd:
	python py/get_system_setting.py serviceFilesDirPath
	$(eval SERVICE_FILES_DIRPATH := $(shell python py/get_system_setting.py serviceFilesDirPath))
	sudo cp osmosis.service ${SERVICE_FILES_DIRPATH}
	sudo systemctl enable osmosis.service
	if ["$(DONT_START_SERVICE)" == ""]; then sudo systemctl start osmosis; fi

install_service_upstart:
	sudo cp upstart_osmosis.conf /etc/init/osmosis.conf
	if ["$(DONT_START_SERVICE)" == ""]; then sudo service osmosis start; fi

uninstall:
	-sudo systemctl stop osmosis
	-sudo service osmosis stop
	-sudo systemctl disable osmosis.service
	sudo rm -f /usr/bin/osmosis
	sudo rm -f /usr/lib/systemd/system/osmosis.service /etc/init/osmosis.conf
	echo "CONSIDER ERASING /var/lib/osmosis"

dist/osmosis-1.0.linux-x86_64.tar.gz:
	cd py; python ../setup.py build
	cd py; python ../setup.py bdist
	cd py; python ../setup.py bdist_egg
	rm -fr dict osmosis.egg-info build/pybuild
	mv py/dist ./
	-mkdir build
	mv py/build build/pybuild
	mv py/osmosis.egg-info ./

prepareForCleanBuild:
	sudo yum install boost-static gcc-c++ --assumeyes
