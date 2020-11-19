PLATFORMIO=${HOME}/.platformio/penv/bin/platformio
PORT=/dev/cu.SLAB_USBtoUART

AUTOPEP8=autopep8

.PHONY: format
format:
	clang-format -i include/*.h src/*.{cpp,h} test/test_embedded/*.cc
	${AUTOPEP8} --in-place --aggressive --aggressive decoders/rtcds3231/pd.py

.PHONY: clean
clean:
	${PLATFORMIO} --caller vim run --target clean

.PHONY: tags
tags:
	ctags --extra=+f --languages=+C,+C++ --recurse=yes --links=no

.PHONY: test
test:
	${PLATFORMIO} test --test-port=${PORT}
