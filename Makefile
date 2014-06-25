all: 
	$(MAKE) clean
	$(MAKE) build unittest check_convention CONFIGURATION=DEBUG
	$(MAKE) clean
	$(MAKE) build unittest

clean:
	rm -fr build

.PHONY: build
build:
	$(MAKE) -f build.Makefile

unittest: build
	python tests/main.py
	build/cpp/testtaskqueue.bin

check_convention:
	pep8 . --max-line-length=109

install:
	-sudo systemctl stop osmosis
	sudo cp build/cpp/osmosis.bin /usr/bin/osmosis
	sudo cp osmosis.service /usr/lib/systemd/system/osmosis.service
	sudo systemctl enable osmosis.service
	if ["$(DONT_START_SERVICE)" == ""]; then sudo systemctl start osmosis; fi

uninstall:
	-sudo systemctl stop osmosis
	-sudo systemctl disable osmosis.service
	sudo rm -f /usr/bin/osmosis
	sudo rm -f /usr/lib/systemd/system/osmosis.service
	echo "CONSIDER ERASING /var/lib/osmosis"
