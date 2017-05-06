#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <errno.h>
#include <wchar.h>
#include <locale.h>

#include "LexAnalyzer.h"

int state = 0, lineNo = 1;
FILE *iFile, *oFile, *eFile;
wchar_t myErrStr[ERR_STR_SIZE];

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "en_US.UTF-8");
    wchar_t ch;
    int res;
    parseArgs(argc, argv);
    while(!feof(iFile)) {
        ch = fgetwc(iFile);
        res = feedSTM(ch);
        if (res & STM_NEED_ROLLBACK) {
            ungetwc(ch, iFile);
        }
        if (res & STM_ERROR) {
            fprintf(stderr, "Error!\n");
            fwprintf(eFile, L"***LINE:%d  %ls\n", lineNo, myErrStr);
            return 1;
        }
    }
    outTuple($EOF, L"EOF");
    return 0;
}

void closeFile()
{
    fclose(iFile);
    fclose(oFile);
    fclose(eFile);
}

char *getOFileName(char *iName)
{
    char *oName;
    size_t iNameLen = strlen(iName), oNameLen;
    size_t i = iNameLen - 1;
    while (i > 0) {
        if (iName[i--] == '.') break;
    }
    if (i == 0) i = iNameLen;
    else i++;
    oNameLen = i + sizeof(oExtName) / sizeof(char) + 1;
    oName = malloc(oNameLen * sizeof(char));
    memcpy(oName, iName, i * sizeof(char));
    strcpy(oName + i, oExtName);
    return oName;
}

char *getEFileName(char *iName)
{
    char *eName;
    size_t iNameLen = strlen(iName), eNameLen;
    size_t i = iNameLen - 1;
    while (i > 0) {
        if (iName[i--] == '.') break;
    }
    if (i == 0) i = iNameLen;
    else i++;
    eNameLen = i + sizeof(eExtName) / sizeof(char) + 1;
    eName = malloc(eNameLen * sizeof(char));
    memcpy(eName, iName, i * sizeof(char));
    strcpy(eName + i, eExtName);
    return eName;
}

void parseArgs(int argc, char *argv[])
{
    char *iFileName, *oFileName;
    switch (argc) {
        case 2:
            iFileName = argv[1];
            oFileName = getOFileName(iFileName);
            break;
        case 3:
            iFileName = argv[1];
            oFileName = argv[2];
            break;
        default:
            fprintf(stderr, "usage: LexAnalyzer inputFileName[ outputFileName]\n");
            exit(1);
    }
    iFile = fopen(iFileName, "r");
    if (iFile == NULL) {
        perror("fopen");
        exit(2);
    }
    oFile = fopen(oFileName, "w");
    if (oFile == NULL) {
        perror("fopen");
        exit(2);
    }
    eFile = fopen(getEFileName(iFileName), "w");
    if (eFile == NULL) {
        perror("fopen");
        exit(2);
    }
    atexit(closeFile);
}

int feedSTM(wchar_t ch)
{
    static wchar_t buffer[MAX_BUF_SIZE];
    static size_t index = 0;
    short code;
    switch (state) {
        case 0:
            if (iswalpha(ch)) {
                state = 1;
                buffer[index++] = ch;
            } else if (iswdigit(ch)) {
                state = 3;
                buffer[index++] = ch;
            } else if (iswblank(ch)) {
                break;
            } else if (ch == L'\n') {
                outTuple($EOLN, L"EOLN");
                lineNo++;
            } else {
                switch (ch) {
                    case L'=':
                        outTuple($E, L"-");
                    break;
                    case L'-':
                        outTuple($SUB, L"-");
                    break;
                    case L'*':
                        outTuple($MUL, L"-");
                    break;
                    case L'(':
                        outTuple($LPAR, L"-");
                    break;
                    case L')':
                        outTuple($RPAR, L"-");
                    break;
                    case L'<':
                        state = 10;
                    break;
                    case L'>':
                        state = 14;
                    break;
                    case L':':
                        state = 17;
                    break;
                    case L';':
                        outTuple($SEM, L"-");
                    break;
                    default:
                        swprintf(myErrStr, ERR_STR_SIZE, L"Unknown character %lc", ch);
                        return STM_ERROR;
                }
            }
        break;
        case 1:
            if (iswalpha(ch) || iswdigit(ch)) {
                buffer[index++] = ch;
            } else {
                buffer[index] = L'\0';
                code = isReservedWord(buffer);
                if (code != 0) {
                    outTuple(code, L"-");
                } else {
                    outTuple($SYMBOL, buffer);
                }
                state = 0;
                index = 0;
                return STM_NEED_ROLLBACK;
            }
        break;
        case 3:
            if (iswdigit(ch)) {
                buffer[index++] = ch;
            } else {
                outTuple($CONSTANT, buffer);
                state = 0;
                index = 0;
                return STM_NEED_ROLLBACK;
            }
        break;
        case 10:
            if (ch == L'=') {
                outTuple($LE, L"-");
                state = 0;
            } else if (ch == L'>') {
                outTuple($NE, L"-");
                state = 0;
            } else {
                outTuple($L, L"-");
                state = 0;
                return STM_NEED_ROLLBACK;
            }
        break;
        case 14:
            if (ch == L'=') {
                outTuple($GE, L"-");
                state = 0;
            } else {
                outTuple($G, L"-");
                state = 0;
                return STM_NEED_ROLLBACK;
            }
        break;
        case 17:
            if (ch == L'=') {
                outTuple($ASSIGN, L"-");
                state = 0;
            } else {
                return STM_ERROR;
            }
        break;
    }
    return 0;
}

void outTuple(short code, wchar_t *str)
{
    if (fwprintf(oFile, L"%hd %ls\n", code, str) < 0) {
        perror("fwprintf");
    }
}

short isReservedWord(wchar_t *str) {
    size_t i, j;
    const wchar_t *rStr;
    for (i = 1; i < sizeof(reservedWords) / sizeof(reservedWords[0]); i++) {
        rStr = reservedWords[i];
        j = 0;
        while (str[j] == rStr[j]) {
            if (str[j] == L'\0') return i;
            j++;
        }
    }
    return 0;
}
