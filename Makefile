CFLAGS := -Wall -O2
LDFLAGS :=

targets = LexAnalyzer

all: $(targets)

LexAnalyzer: LexAnalyzer.c LexAnalyzer.h
	$(CC) $< -o $@ $(CFLAGS)

.PONY: clean
clean:
	rm -f $(targets)