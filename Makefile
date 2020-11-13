.PHONY: format
format:
	clang-format -i include/*.h src/*.cpp

.PHONY: tags
tags:
	ctags --extra=+f --languages=+C,+C++ --recurse=yes --links=no
