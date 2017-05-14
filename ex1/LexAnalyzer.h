#ifndef LEXANALYZER_H
#define LEXANALYZER_H

#include <wchar.h>

const char oExtName[] = ".dyd";
const char eExtName[] = ".err";

#define MAX_BUF_SIZE 4096
#define ERR_STR_SIZE 512

const wchar_t *reservedWords[] = {
    [1] = L"begin",
    [2] = L"end",
    [3] = L"integer",
    [4] = L"if",
    [5] = L"then",
    [6] = L"else",
    [7] = L"function",
    [8] = L"read",
    [9] = L"write"
};
#define $SYMBOL 10
#define $CONSTANT 11
#define $E 12
#define $NE 13
#define $LE 14
#define $L 15
#define $GE 16
#define $G 17
#define $SUB 18
#define $MUL 19
#define $ASSIGN 20
#define $LPAR 21
#define $RPAR 22
#define $SEM 23
#define $EOLN 24
#define $EOF 25
short isReservedWord(wchar_t *str);
void outTuple(short code, wchar_t *str);

void parseArgs(int argc, char *argv[]);

int feedSTM(wchar_t ch);
#define STM_NEED_ROLLBACK 1
#define STM_ERROR 2

#endif
