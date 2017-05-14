CFLAGS := -Wall -O2

targets = LexAnalyzer SyntaxAnalyzer

all: $(targets)

LexAnalyzer: LexAnalyzer.c LexAnalyzer.h
	$(CC) $< -o $@ $(CFLAGS)

SyntaxAnalyzer: SyntaxAnalyzer.c SyntaxAnalyzer.h
	$(CC) $< -o $@ $(CFLAGS)

.PONY: clean
clean:
	rm -f $(targets)