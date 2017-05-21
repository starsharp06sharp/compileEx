#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <stdarg.h>
#include <errno.h>
#include <wchar.h>
#include <locale.h>
#include "SyntaxAnalyzer.h"

FILE *iFile, *oFile, *varFile, *proFile, *eFile;
struct Word word;
int lineNo = 1;
wchar_t now_func[MAX_BUF_SIZE];
int level = 0;
int vaddr = 0;

int main(int argc, char *argv[])
{
    parseArgs(argc, argv);
    advance();//读入第一个符号
    program();//开始解析
    return 0;
}

char *getFileName(const char *iName, const char *extName)
{
    static char *baseName = NULL;
    static size_t baseNameLen;
    char *retName;
    if (baseName == NULL) {
        size_t iNameLen = strlen(iName);
        size_t i = iNameLen - 1;
        while (i > 0) {
            if (iName[i--] == '.') break;
        }
        if (i == 0) baseNameLen = iNameLen;
        else baseNameLen = i + 1;
        baseName = malloc((baseNameLen + 1) * sizeof(char));
        strncpy(baseName, iName, baseNameLen);
        baseName[baseNameLen] = '\0';
    }
    retName = malloc((baseNameLen + strlen(extName) + 1) * sizeof(char));
    strcpy(retName, baseName);
    strcpy(retName + baseNameLen, extName);
    return retName;
}

void closeFile()
{
    fclose(iFile);
    fclose(oFile);
    fclose(varFile);
    fclose(proFile);
    fclose(eFile);
}

void parseArgs(int argc, char *argv[])
{
    char *iFileName;
    switch (argc) {
        case 2:
            iFileName = argv[1];
            break;
        default:
            fprintf(stderr, "usage: SyntaxAnalyzer inputFileName\n");
            exit(1);
    }
    setlocale(LC_ALL, "en_US.UTF-8");
    iFile = fopen(iFileName, "r");
    if (iFile == NULL) {
        perror("fopen");
        exit(2);
    }
    oFile = fopen(getFileName(iFileName, oExtName), "w");
    if (oFile == NULL) {
        perror("fopen");
        exit(2);
    }
    varFile = fopen(getFileName(iFileName, varExtName), "w");
    if (varFile == NULL) {
        perror("fopen");
        exit(2);
    }
    proFile = fopen(getFileName(iFileName, proExtName), "w");
    if (proFile == NULL) {
        perror("fopen");
        exit(2);
    }
    eFile = fopen(getFileName(iFileName, eExtName), "w");
    if (eFile == NULL) {
        perror("fopen");
        exit(2);
    }
    atexit(closeFile);
}

void advance()
{
    fwscanf(iFile, L"%hd %ls", &(word.code), &(word.str));
    fwprintf(oFile, L"%hd %ls\n", word.code, word.str);
    if (word.code == $EOLN) {
        lineNo++;
        advance();
    } else if (word.code == $EOF) {
        exit(0);
    }
}

void raiseError(const wchar_t *format, ...)
{
    fwprintf(stderr, L"Error!\n");
    fwprintf(eFile, L"***LINE: %d ", lineNo);
    va_list ap;
    va_start(ap, format);

    vfwprintf(eFile, format, ap);

    va_end(ap);

    fwprintf(eFile, L"\n");
    exit(1);
}

#define exceptWord(w) \
    if (word.code != w) { \
        raiseError(L"In function '%s': Need " L###w L"(%d) statement", __FUNCTION__, w); \
    } \
    advance();

void relationOp()
{
    switch(word.code) {
    case $E:case $NE:case $LE:case $L:case $GE:case $G:
        advance();
    break;
    default:
        raiseError(L"Need relationOp");
    break;
    }
}

void conditionExpression()
{
    arithmeticalExpression();
    relationOp();
    arithmeticalExpression();
}

void conditionStatement()
{
    exceptWord($IF);
    conditionExpression();
    exceptWord($THEN);
    execution();
    exceptWord($ELSE);
    execution();
}

void assignment()
{
    var();
    exceptWord($ASSIGN);
    arithmeticalExpression();
}

void write()
{
    exceptWord($WRITE);
    exceptWord($LPAR);
    var();
    exceptWord($RPAR);
}

void read()
{
    exceptWord($READ);
    exceptWord($LPAR);
    var();
    exceptWord($RPAR);
}

void funcBody()
{
    exceptWord($BEGIN);
    declarationTable();
    executionTable();
    exceptWord($END);
}

void argument()
{
    var();
}

void var()
{
    exceptWord($SYMBOL);
}

void subprogram()
{
    exceptWord($BEGIN);
    declarationTable();
    executionTable();
    exceptWord($END);
}

void program()
{
    subprogram();
}

void declaration()
{
    exceptWord($INTEGER);
    declaration2();
}

void declaration2()
{
    if (word.code == $FUNCTION) {
        exceptWord($FUNCTION);
        exceptWord($SYMBOL);
        exceptWord($LPAR);
        argument();
        exceptWord($RPAR);
        exceptWord($SEM);
        funcBody();
    } else {
        var();
    }
}

void execution()
{
    if (word.code == $READ) {
        read();
    } else if (word.code == $WRITE) {
        write();
    } else if (word.code == $IF) {
        conditionStatement();
    } else {
        assignment();
    }
}

void factor()
{
    if (word.code == $CONSTANT) {
        exceptWord($CONSTANT);
    } else {
        exceptWord($SYMBOL);
        if (word.code == $LPAR) {
            //function call
            exceptWord($LPAR);
            arithmeticalExpression();
            exceptWord($RPAR);
        }
    }
}

void declarationTable()
{
    declaration();
    declarationTable2();
}

void declarationTable2()
{
    if (word.code == $SEM) {
        exceptWord($SEM);
        if (word.code != $INTEGER) return;
        declaration();
        declarationTable2();
    }
}

void executionTable()
{
    execution();
    executionTable2();
}

void executionTable2()
{
    if (word.code == $SEM) {
        exceptWord($SEM);
        execution();
        executionTable2();
    }
}

void arithmeticalExpression()
{
    item();
    arithmeticalExpression2();
}

void arithmeticalExpression2()
{
    if (word.code == $SUB) {
        exceptWord($SUB);
        item();
        arithmeticalExpression2();
    }
}

void item()
{
    factor();
    item2();
}

void item2()
{
    if (word.code == $MUL) {
        exceptWord($MUL);
        factor();
        item2();
    }
}
