lex-sqlite:	lex-sqlite.cpp lex_sqlite.h lex_sqlite.c
	$(CXX) -Wall -Wextra -pedantic -O2 -g -o $@ lex_sqlite.c lex-sqlite.cpp

.PHONY: format
format:
	clang-format-14 -i *.h *.c *.cpp

.PHONY: clean
clean:
	rm -f lex_sqlite.o lex-sqlite a.out
