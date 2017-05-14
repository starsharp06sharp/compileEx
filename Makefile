CFLAGS := -Wall -O2

targets = LexAnalyzer

all: $(targets)

LexAnalyzer: ex1/LexAnalyzer.c ex1/LexAnalyzer.h
	$(CC) $< -o $@ $(CFLAGS)

.PONY: clean
clean:
	rm -f $(targets)