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
