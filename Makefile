PLATFORMIO=${HOME}/.platformio/penv/bin/platformio
PORT=/dev/cu.SLAB_USBtoUART

.PHONY: format
format:
	clang-format -i include/*.h src/*.{cpp,h} test/test_embedded/*.cc

.PHONY: clean
clean:
	${PLATFORMIO} --caller vim run --target clean

.PHONY: tags
tags:
	ctags --extra=+f --languages=+C,+C++ --recurse=yes --links=no

.PHONY: test
test:
	${PLATFORMIO} test --test-port=${PORT}
