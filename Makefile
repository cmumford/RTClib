PLATFORMIO=${HOME}/.platformio/penv/bin/platformio
PORT=/dev/cu.SLAB_USBtoUART

AUTOPEP8=autopep8

.PHONY: format
format:
	clang-format -i include/rtclib/*.h src/*.cc src/*.h test/test_embedded/*.cc
	${AUTOPEP8} --in-place --aggressive --aggressive decoders/rtcds3231/pd.py

docs: doxygen.conf Makefile
	doxygen doxygen.conf

.PHONY: clean
clean:
	${PLATFORMIO} --caller vim run --target clean
	rm -rf docs

.PHONY: tags
tags:
	ctags --extra=+f --languages=+C,+C++ --recurse=yes --links=no

.PHONY: test
test:
	${PLATFORMIO} test --test-port=${PORT}
