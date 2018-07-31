#include "SyntaxAnalyzer.h"
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

FILE *iFile, *oFile, *varFile, *proFile, *eFile;
struct Word word;
int lineNo = 1;
wchar_t nowFunc[SYM_TABLE_SIZE][MAX_BUF_SIZE] = {L"[global]"};
int level = 0;
int vaddr = 0;

wchar_t symTable[SYM_TABLE_SIZE][MAX_BUF_SIZE];
size_t symTableSize = 0;
size_t scopeStart = 0;

int main(int argc, char *argv[])
{
    parseArgs(argc, argv);
    advance();  //读入第一个符号
    program();  //开始解析
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
        if (i == 0)
            baseNameLen = iNameLen;
        else
            baseNameLen = i + 1;
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
    fwscanf(iFile, L"%hd %ls", &(word.type), &(word.str));
    fwprintf(oFile, L"%hd %ls\n", word.type, word.str);
    if (word.type == $EOLN) {
        lineNo++;
        advance();
    } else if (word.type == $EOF) {
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

bool checkSymExist(const wchar_t *sym, bool global)
{
    int i = scopeStart;
    if (global) i = 0;
    for (; i < symTableSize; i++) {
        if (wcscmp(sym, symTable[i]) == 0) return true;
    }
    return false;
}

void addVar(const wchar_t *vname, int vkind)
{
    if (checkSymExist(vname, false)) {
        raiseError(L"Duplicate symbol definition %ls", vname);
    }
    wcscpy(symTable[symTableSize++], vname);

    fwprintf(varFile, L"vname: %ls\n", vname);
    fwprintf(varFile, L"vpro: %ls\n", nowFunc[level]);
    fwprintf(varFile, L"vkind: %d\n", vkind);
    fwprintf(varFile, L"vtype: ints\n");
    fwprintf(varFile, L"vlev: %d\n", level);
    fwprintf(varFile, L"vaddr: %d\n", vaddr);
    fwprintf(varFile, L"\n");
}

void addFunc(int faddr, int laddr)
{
    fwprintf(proFile, L"pname: %ls\n", nowFunc[level]);
    fwprintf(proFile, L"ptype: ints\n");
    fwprintf(proFile, L"plev: %d\n", level);
    fwprintf(proFile, L"fadr: %d\n", faddr);
    fwprintf(proFile, L"ladr: %d\n", laddr);
    fwprintf(proFile, L"\n");
}

#define exceptWord(w)                                                   \
    if (word.type != w) {                                               \
        raiseError(L"In function '%s': Need " L## #w L"(%d) statement", \
                   __FUNCTION__, w);                                    \
    }                                                                   \
    advance();

void relationOp()
{
    switch (word.type) {
    case $E:
    case $NE:
    case $LE:
    case $L:
    case $GE:
    case $G:
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
    if (word.type == $SYMBOL) {
        addVar(word.str, 1);
        vaddr++;
    }
    var();
}

void var()
{
    if (word.type == $SYMBOL) {
        if (!checkSymExist(word.str, true)) {
            raiseError(L"Unknown symbol %ls", word.str);
        }
    }
    exceptWord($SYMBOL);
}

void subprogram()
{
    exceptWord($BEGIN);
    declarationTable();
    executionTable();
    exceptWord($END);
}

void program() { subprogram(); }

void declaration()
{
    exceptWord($INTEGER);
    declaration2();
}

void declaration2()
{
    int faddr, lastScopeStart = scopeStart;
    if (word.type == $FUNCTION) {
        exceptWord($FUNCTION);
        if (word.type == $SYMBOL) {
            faddr = vaddr;
            level++;
            wcscpy(nowFunc[level], word.str);
            scopeStart = symTableSize;
            if (checkSymExist(nowFunc[level], false)) {
                raiseError(L"Duplicate symbol definition %ls", nowFunc[level]);
            }
            wcscpy(symTable[symTableSize++], nowFunc[level]);
        }
        exceptWord($SYMBOL);
        exceptWord($LPAR);
        argument();
        exceptWord($RPAR);
        exceptWord($SEM);
        funcBody();

        addFunc(faddr, vaddr - 1);
        symTableSize = scopeStart + 1;
        scopeStart = lastScopeStart;
        level--;
    } else {
        if (word.type == $SYMBOL) {
            addVar(word.str, 0);
            vaddr++;
        }
        var();
    }
}

void execution()
{
    if (word.type == $READ) {
        read();
    } else if (word.type == $WRITE) {
        write();
    } else if (word.type == $IF) {
        conditionStatement();
    } else {
        assignment();
    }
}

void factor()
{
    if (word.type == $CONSTANT) {
        exceptWord($CONSTANT);
    } else {
        if (word.type == $SYMBOL) {
            if (!checkSymExist(word.str, true)) {
                raiseError(L"Unknown symbol %ls", word.str);
            }
        }
        exceptWord($SYMBOL);
        if (word.type == $LPAR) {
            // function call
            exceptWord($LPAR);
            arithmeticalExpression();
            exceptWord($RPAR);
        }
    }
}

void declarationTable()
{
    if (word.type == $INTEGER) {
        declaration();
        exceptWord($SEM);
        declarationTable();
    }
}

void executionTable()
{
    execution();
    executionTable2();
}

void executionTable2()
{
    if (word.type == $SEM) {
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
    if (word.type == $SUB) {
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
    if (word.type == $MUL) {
        exceptWord($MUL);
        factor();
        item2();
    }
}
