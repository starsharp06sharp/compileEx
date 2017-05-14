#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H

#include <wchar.h>
#include "word.h"

const char oExtName[] = ".dys";
const char varExtName[] = ".var";
const char proExtName[] = ".pro";
const char eExtName[] = ".err";

#define MAX_BUF_SIZE 4096

struct Word {
    short code;
    wchar_t str[MAX_BUF_SIZE];
};

void program();                 //程序
void subprogram();              //分程序
void declarationTable();        //说明语句表 FIXME: 消除左递归
void declaration();             //说明语句 FIXME: 化简产生式
void varDeclaration();          //变量说明
void var();                     //变量
void funcDeclaration();         //函数说明
void argument();                //参数
void funcBody();                //函数体
void executionTable();          //执行语句表 FIXME: 消除左递归
void execution();               //执行语句 FIXME: 化简产生式
void read();                    //读语句
void write();                   //写语句
void assignment();              //赋值语句
void arithmeticalExpression();  //算术表达式 FIXME: 消除左递归
void item();                    //项 FIXME: 消除左递归
void factor();                  //因子 FIXME: 化简产生式
void conditionStatement();      //条件语句
void conditionExpression();     //条件表达式
void relationOp();              //关系运算符

void advance();
void raiseError(const wchar_t *format, ...);

void parseArgs(int argc, char *argv[]);

#endif