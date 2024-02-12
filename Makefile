# NETLIB = cpp-netlib-0.12.0-final
NETLIB = cpp-netlib-0.13.0-final
all: 
	$(MAKE) clean
	$(MAKE) build unittest check_convention CONFIGURATION=DEBUG
	$(MAKE) clean
	$(MAKE) build unittest egg

clean:
	rm -fr build dist osmosis.egg-info

.PHONY: build egg install_binary osmosis-cli
build: build/$(NETLIB)/.unpacked
	$(MAKE) -f build.Makefile

build/$(NETLIB)/.unpacked: build/$(NETLIB)

build/$(NETLIB):build/cpp-netlib-$(NETLIB)
	mv $< $@
	touch $@/.unpacked

build/cpp-netlib-$(NETLIB): $(NETLIB).tar.gz
	mkdir -p build
	tar -xf $< -C build

egg: dist/osmosis-1.0.linux-x86_64.tar.gz

unittest: build
	PYTHONPATH=py python tests/main.py $(TESTS)
	build/cpp/testtaskqueue.bin

check_convention:
	python -m pep8 py tests --max-line-length=109

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

dist/osmosis-1.0.linux-x86_64.tar.gz: setup.py py/*/*.py
	python setup.py build
	python setup.py bdist
	python setup.py bdist_egg

prepareForCleanBuild: centos_cpp_deps

centos_cpp_deps: rpm-requirements.txt
	sudo yum install --assumeyes $$(cat $<)

venv:
	virtualenv venv
	source venv/bin/activate \
		&& pip install -r dev-requirements.txt \
		&& deactivate
	@echo "Run 'source venv/bin/activate' to activate the virtual environment"

testo:
	./build/cpp/osmosis.bin listlabels --objectStores osmosis.dc1.strato:1010 base

osmosis-cli:
	skipper make
	skipper build osmosis-cli