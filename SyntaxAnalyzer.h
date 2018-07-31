#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H

#include <stdbool.h>
#include <wchar.h>
#include "word.h"

const char oExtName[] = ".dys";
const char varExtName[] = ".var";
const char proExtName[] = ".pro";
const char eExtName[] = ".err";

#define MAX_BUF_SIZE 1024
#define SYM_TABLE_SIZE 1024

struct Word {
    short type;
    wchar_t str[MAX_BUF_SIZE];
};

void program();                  //程序
void subprogram();               //分程序
void declarationTable();         //说明语句表
void declaration();              //说明语句
void declaration2();             //说明语句2
void var();                      //变量
void argument();                 //参数
void funcBody();                 //函数体
void executionTable();           //执行语句表
void executionTable2();          //执行语句表2
void execution();                //执行语句
void read();                     //读语句
void write();                    //写语句
void assignment();               //赋值语句
void arithmeticalExpression();   //算术表达式
void arithmeticalExpression2();  //算术表达式2
void item();                     //项
void item2();                    //项2
void factor();                   //因子
void conditionStatement();       //条件语句
void conditionExpression();      //条件表达式
void relationOp();               //关系运算符

void advance();
void raiseError(const wchar_t *format, ...);
void addVar(const wchar_t *vname, int vkind);
void addFunc(int faddr, int laddr);
bool checkSymExist(const wchar_t *sym, bool global);

void parseArgs(int argc, char *argv[]);

#endif