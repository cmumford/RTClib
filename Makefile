.PHONY: format
format:
	clang-format -i include/*.h src/*.{cpp,h}

.PHONY: tags
tags:
	ctags --extra=+f --languages=+C,+C++ --recurse=yes --links=no
