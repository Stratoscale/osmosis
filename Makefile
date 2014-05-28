all: build test

clean:
	rm -fr build

.PHONY: build
build:
	$(MAKE) -f build.Makefile

test: testscenarios whiteboxtest check_convention

testscenarios: build
	python tests/main.py

whiteboxtest: build
	build/cpp/testtaskqueue.bin

check_convention:
	pep8 . --max-line-length=109

install:
	sudo cp build/cpp/osmosis.bin /usr/bin/osmosis
	sudo cp osmosis.service /usr/lib/systemd/system/osmosis.service
	sudo systemctl enable osmosis
	-sudo systemctl stop osmosis
	sudo systemctl start osmosis

uninstall:
	sudo systemctl stop osmosis
	sudo systemctl disable osmosis
	sudo rm -f /usr/bin/osmosis
	sudo rm -f /usr/lib/systemd/system/osmosis.service
	echo "CONSIDER ERASING /var/lib/osmosis"
