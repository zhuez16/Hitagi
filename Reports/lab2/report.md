# lab2 实验报告
PB18111704 朱恩佐
## 实验要求

本次实验需要lab1的词法部分复制到/src/parser目录的lexical_analyzer.l并合理修改相应部分，然后根据 cminus-f 的语法补全 syntax_analyer.y文件，完成语法分析器，要求最终能够输出解析树。

## 实验难点

##### 1. Bison的语法

**Bison**包含两个部分：一个是终结符和非终结符的声明；另一个是非终结符的展开。第一部分由于终结符和非终结符都对应了语法树上的节点，所以所有的终结符和非终结符都是node*类型。第二部分展开需要注意所有的终结符的命名问题：由于符号的名称可能产生冲突所以要注意不能写错。

##### 2.我真的不知道这实验还有什么难点了。。。

## 实验设计

##### 1. 修改lexer的代码

lexer在本实验中负责识别并过滤传入的代码中的token并传入parser。而lab1中识别token之后会多出额外的很多操作：诸如保存到词法分析文件中、报错等。所以需要将这一部分删掉。修改后代码如下：

```c
%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int files_count;
int lines;
int pos_start;
int pos_end;

void pass_node(char *text){
     yylval.node = new_syntax_tree_node(text);
}
%}

//正则表达式

COMMENT (\/\*([^\*]|[\*]*[^\/\*])*[\*]*\*\/)
ELSE (else)
IF (if)
INT (int)
FLOAT (float)
RETURN (return)
VOID (void)
WHILE (while)
ARRAY \[\]
EQ [=][=]
NEQ [!][=]
GTE [>][=]
LTE [<][=]
ADD [+]
SUB [-]
MUL [*]
DIV [/]
LT [<]
GT [>]
ASSIN [=]
SEMICOLON [;]
COMMA [,]
LPARENTHESE \(
RPARENTHESE \)
LBRACKET \[
RBRACKET \]
LBRACE \{
RBRACK \}
ID [a-zA-Z]+
INTEGER [0-9]+
FLOATPOINT [0-9]+[.]|[0-9]*[.][0-9]+
EOL [\n] 
BLANK [ ]|[\t]

//FLEX 动作
{ELSE} 		{pos_start=pos_end;pos_end=pos_start+4;pass_node(yytext);return ELSE;}
{IF} 		{pos_start=pos_end;pos_end=pos_start+2;pass_node(yytext);return IF;}
{INT} 		{pos_start=pos_end;pos_end=pos_start+3;pass_node(yytext);return INT;}
{FLOAT} 	{pos_start=pos_end;pos_end=pos_start+5;pass_node(yytext);return FLOAT;}
{RETURN} 	{pos_start=pos_end;pos_end=pos_start+6;pass_node(yytext);return RETURN;}
{VOID} 		{pos_start=pos_end;pos_end=pos_start+4;pass_node(yytext);return VOID;}
{WHILE} 	{pos_start=pos_end;pos_end=pos_start+5;pass_node(yytext);return WHILE;}
{ARRAY} 	{pos_start=pos_end;pos_end=pos_start+2;pass_node(yytext);return ARRAY;}
{EQ} 		{pos_start=pos_end;pos_end=pos_start+2;pass_node(yytext);return EQ;}
{NEQ} 		{pos_start=pos_end;pos_end=pos_start+2;pass_node(yytext);return NEQ;}
{GTE} 		{pos_start=pos_end;pos_end=pos_start+2;pass_node(yytext);return GTE;}
{LTE} 		{pos_start=pos_end;pos_end=pos_start+2;pass_node(yytext);return LTE;}
{ADD} 		{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return ADD;}
{SUB} 		{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return SUB;}
{MUL} 		{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return MUL;}
{DIV}	 	{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return DIV;}
{LT} 		{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return LT;}
{GT} 		{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return GT;}
{ASSIN} 	{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return ASSIN;}
{SEMICOLON} 	{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return SEMICOLON;}
{COMMA} 	{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return COMMA;}
{LPARENTHESE} 	{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return LPARENTHESE;}
{RPARENTHESE} 	{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return RPARENTHESE;}
{LBRACKET} 	{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return LBRACKET;}
{RBRACKET} 	{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return RBRACKET;}
{LBRACE} 	{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return LBRACE;}
{RBRACK} 	{pos_start=pos_end;pos_end=pos_start+1;pass_node(yytext);return RBRACE;}
{ID} 	{pos_start=pos_end;pos_end=pos_start+yyleng;pass_node(yytext);return ID;}
{INTEGER} 	{pos_start=pos_end;pos_end=pos_start+yyleng;pass_node(yytext);return INTEGER;}
{FLOATPOINT} 	{pos_start=pos_end;pos_end=pos_start+yyleng;pass_node(yytext);return FLOATPOINT;}
{EOL} 		{pos_start=pos_end;pos_end=pos_start+1;analyzer(EOL);}
{BLANK} 	{pos_start=pos_end;pos_end=pos_start+1;analyzer(BLANK);}
{COMMENT} {pos_start=pos_end;analyzer(COMMENT);}
//词法分析中EOL、BLANK、COMMENT三种token是不需要传入parser的
void analyzer(int token){
    int i;
        switch(token){
            case COMMENT:
                //STUDENT TO DO
		for (i = 0;i < yyleng; i++){
			pos_end++;
			if (yytext[i]=='\n'){
				lines++;
				pos_end=1;
			}
		}
		break;
            case BLANK:
                //STUDENT TO DO
                break;
            case EOL:
                //STUDENT TO DO
		lines++;
		pos_start=1;
		pos_end=1;
                break;
       }
    return;
}

```

##### 2.Parser设计

将Cminus-f包含的所有语法规则转换为Bison的语法并写入即可。

```C
program : declaration-list { $$ = node("program", 1, $1); gt->root = $$; }

declaration-list : declaration-list declaration { $$=node("declaration-list",2,$1,$2); }
                 | declaration { $$=node("declaration-list",1,$1); }

declaration : var-declaration { $$=node("declaration",1,$1); } | 
              fun-declaration { $$=node("declaration",1,$1); }

var-declaration : type-specifier ID SEMICOLON { $$=node("var-declaration",3,$1,$2,$3); } |
                  type-specifier ID LBRACKET INTEGER RBRACKET SEMICOLON { $$=node("var-declaration",6,$1,$2,$3,$4,$5,$6); }

type-specifier : INT {$$=node("type-specifier",1,$1);} | FLOAT{$$=node("type-specifier",1,$1);} | VOID{$$=node("type-specifier",1,$1);}

fun-declaration : type-specifier ID LPARENTHESE params RPARENTHESE compound-stmt {$$=node("fun-declaration",6,$1,$2,$3,$4,$5,$6);}

params : param-list {$$=node("params",1,$1);} | VOID {$$=node("params",1,$1);}

param-list : param-list COMMA param {$$=node("param-list",3,$1,$2,$3);} | param {$$=node("param-list",1,$1);}

param : type-specifier ID {$$=node("param",2,$1,$2);} | type-specifier ID ARRAY {$$=node("param",3,$1,$2,$3);}

compound-stmt : LBRACE local-declarations statement-list RBRACE {$$=node("compound-stmt",4,$1,$2,$3,$4);}

local-declarations : local-declarations var-declaration {$$=node("local-declarations",2,$1,$2);} | {$$=node("local-declarations",0);}

statement-list : statement-list statement {$$=node("statement-list",2,$1,$2);}| {$$=node("statement-list",0);}

statement : expression-stmt {$$=node("statement",1,$1);}
          | compound-stmt   {$$=node("statement",1,$1);}
          | selection-stmt  {$$=node("statement",1,$1);}
          | iteration-stmt  {$$=node("statement",1,$1);}
          | return-stmt     {$$=node("statement",1,$1);}

expression-stmt : expression SEMICOLON {$$=node("expression-stmt",2,$1,$2);}
                | SEMICOLON {$$=node("expression-stmt",1,$1);}

selection-stmt : IF LPARENTHESE expression RPARENTHESE statement {$$=node("selection-stmt",5,$1,$2,$3,$4,$5);}
               | IF LPARENTHESE expression RPARENTHESE statement ELSE statement {$$=node("selection-stmt",7,$1,$2,$3,$4,$5,$6,$7);}

iteration-stmt : WHILE LPARENTHESE expression RPARENTHESE statement {$$=node("iteration-stmt",5,$1,$2,$3,$4,$5);}

return-stmt : RETURN SEMICOLON {$$=node("return-stmt",2,$1,$2);}
            | RETURN expression SEMICOLON {$$=node("return-stmt",3,$1,$2,$3);}

expression : var ASSIN expression {$$=node("expression",3,$1,$2,$3);}
           | simple-expression {$$=node("expression",1,$1);}

var : ID {$$=node("var",1,$1);}
    | ID LBRACKET expression RBRACKET {$$=node("var",4,$1,$2,$3,$4);}

simple-expression : additive-expression relop additive-expression {$$=node("simple-expression",3,$1,$2,$3);}
                  | additive-expression {$$=node("simple-expression",1,$1);}

relop : EQ {$$=node("relop",1,$1);}
      | NEQ {$$=node("relop",1,$1);}
      | GTE {$$=node("relop",1,$1);}
      | LTE {$$=node("relop",1,$1);}
      | LT {$$=node("relop",1,$1);}
      | GT {$$=node("relop",1,$1);}

additive-expression : additive-expression addop term {$$=node("additive-expression",3,$1,$2,$3);}
                    | term {$$=node("additive-expression",1,$1);}

addop : ADD {$$=node("addop",1,$1);}| SUB {$$=node("addop",1,$1);}

term : term mulop factor {$$=node("term",3,$1,$2,$3);}
     | factor {$$=node("term",1,$1);}

mulop : MUL {$$=node("mulop",1,$1);} | DIV {$$=node("mulop",1,$1);}

factor : LPARENTHESE expression RPARENTHESE {$$=node("factor",3,$1,$2,$3);}
       | var {$$=node("factor",1,$1);}
       | call {$$=node("factor",1,$1);}
       | integer {$$=node("factor",1,$1);}
       | float {$$=node("factor",1,$1);}

integer : INTEGER {$$=node("integer",1,$1);}

float : FLOATPOINT {$$=node("float",1,$1);}

call : ID LPARENTHESE args RPARENTHESE {$$=node("call",4,$1,$2,$3,$4);}

args : arg-list {$$=node("args",1,$1);}
     | {$$=node("args",0);}

arg-list : arg-list COMMA expression {$$=node("arg-list",3,$1,$2,$3);}
         | expression {$$=node("arg-list",1,$1);}
%%

```

注意识别空串不需要加入任何非终结符。用爱写出来的节点生成函数会帮你处理一切。

## 实验结果验证

```c
float GVAR;
void NeverEverDeclareLikeThis;
int GARRAY[2333];
void MyFuncA(int floatNum, float intNum, void voidNums[]){
    int IKnowYouAreVoid;
    return MyFuncB(IKnowYouAreVoid);
}
float MyFuncB(void){
    int IAmVoid[0];
    return MyFuncA(.0, 0, IAmVoid);
}
int main(void){
    int a; int b; int c;
    a = b = c = (85 == 84 + 0.4);
    if(a = b){
        GARRAY[ ( MyFuncB() ) ] = GARRAY[c = 1.*.1 == 1.1];
    }else if (MyFuncC(NotDeclared)){
    }else;
    return 0.;
}
```

output:

```
>--+ program
|  >--+ declaration-list
|  |  >--+ declaration-list
|  |  |  >--+ declaration-list
|  |  |  |  >--+ declaration-list
|  |  |  |  |  >--+ declaration-list
|  |  |  |  |  |  >--+ declaration-list
|  |  |  |  |  |  |  >--+ declaration
|  |  |  |  |  |  |  |  >--+ var-declaration
|  |  |  |  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  |  |  |  >--* float
|  |  |  |  |  |  |  |  |  >--* GVAR
|  |  |  |  |  |  |  |  |  >--* ;
|  |  |  |  |  |  >--+ declaration
|  |  |  |  |  |  |  >--+ var-declaration
|  |  |  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  |  |  >--* void
|  |  |  |  |  |  |  |  >--* NeverEverDeclareLikeThis
|  |  |  |  |  |  |  |  >--* ;
|  |  |  |  |  >--+ declaration
|  |  |  |  |  |  >--+ var-declaration
|  |  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  |  >--* int
|  |  |  |  |  |  |  >--* GARRAY
|  |  |  |  |  |  |  >--* [
|  |  |  |  |  |  |  >--* 2333
|  |  |  |  |  |  |  >--* ]
|  |  |  |  |  |  |  >--* ;
|  |  |  |  >--+ declaration
|  |  |  |  |  >--+ fun-declaration
|  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  >--* void
|  |  |  |  |  |  >--* MyFuncA
|  |  |  |  |  |  >--* (
|  |  |  |  |  |  >--+ params
|  |  |  |  |  |  |  >--+ param-list
|  |  |  |  |  |  |  |  >--+ param-list
|  |  |  |  |  |  |  |  |  >--+ param-list
|  |  |  |  |  |  |  |  |  |  >--+ param
|  |  |  |  |  |  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  |  |  |  |  |  >--* int
|  |  |  |  |  |  |  |  |  |  |  >--* floatNum
|  |  |  |  |  |  |  |  |  >--* ,
|  |  |  |  |  |  |  |  |  >--+ param
|  |  |  |  |  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  |  |  |  |  >--* float
|  |  |  |  |  |  |  |  |  |  >--* intNum
|  |  |  |  |  |  |  |  >--* ,
|  |  |  |  |  |  |  |  >--+ param
|  |  |  |  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  |  |  |  >--* void
|  |  |  |  |  |  |  |  |  >--* voidNums
|  |  |  |  |  |  |  |  |  >--* []
|  |  |  |  |  |  >--* )
|  |  |  |  |  |  >--+ compound-stmt
|  |  |  |  |  |  |  >--* {
|  |  |  |  |  |  |  >--+ local-declarations
|  |  |  |  |  |  |  |  >--+ local-declarations
|  |  |  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  |  |  >--+ var-declaration
|  |  |  |  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  |  |  |  >--* int
|  |  |  |  |  |  |  |  |  >--* IKnowYouAreVoid
|  |  |  |  |  |  |  |  |  >--* ;
|  |  |  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  |  |  >--+ statement
|  |  |  |  |  |  |  |  |  >--+ return-stmt
|  |  |  |  |  |  |  |  |  |  >--* return
|  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ call
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* MyFuncB
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* (
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ args
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ arg-list
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ var
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* IKnowYouAreVoid
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* )
|  |  |  |  |  |  |  |  |  |  >--* ;
|  |  |  |  |  |  |  >--* }
|  |  |  >--+ declaration
|  |  |  |  >--+ fun-declaration
|  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  >--* float
|  |  |  |  |  >--* MyFuncB
|  |  |  |  |  >--* (
|  |  |  |  |  >--+ params
|  |  |  |  |  |  >--* void
|  |  |  |  |  >--* )
|  |  |  |  |  >--+ compound-stmt
|  |  |  |  |  |  >--* {
|  |  |  |  |  |  >--+ local-declarations
|  |  |  |  |  |  |  >--+ local-declarations
|  |  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  |  >--+ var-declaration
|  |  |  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  |  |  >--* int
|  |  |  |  |  |  |  |  >--* IAmVoid
|  |  |  |  |  |  |  |  >--* [
|  |  |  |  |  |  |  |  >--* 0
|  |  |  |  |  |  |  |  >--* ]
|  |  |  |  |  |  |  |  >--* ;
|  |  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  |  >--+ statement
|  |  |  |  |  |  |  |  >--+ return-stmt
|  |  |  |  |  |  |  |  |  >--* return
|  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ call
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* MyFuncA
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* (
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ args
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ arg-list
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ arg-list
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ arg-list
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ float
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* .0
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* ,
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ integer
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* 0
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* ,
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ var
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* IAmVoid
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* )
|  |  |  |  |  |  |  |  |  >--* ;
|  |  |  |  |  |  >--* }
|  |  >--+ declaration
|  |  |  >--+ fun-declaration
|  |  |  |  >--+ type-specifier
|  |  |  |  |  >--* int
|  |  |  |  >--* main
|  |  |  |  >--* (
|  |  |  |  >--+ params
|  |  |  |  |  >--* void
|  |  |  |  >--* )
|  |  |  |  >--+ compound-stmt
|  |  |  |  |  >--* {
|  |  |  |  |  >--+ local-declarations
|  |  |  |  |  |  >--+ local-declarations
|  |  |  |  |  |  |  >--+ local-declarations
|  |  |  |  |  |  |  |  >--+ local-declarations
|  |  |  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  |  |  >--+ var-declaration
|  |  |  |  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  |  |  |  >--* int
|  |  |  |  |  |  |  |  |  >--* a
|  |  |  |  |  |  |  |  |  >--* ;
|  |  |  |  |  |  |  >--+ var-declaration
|  |  |  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  |  |  >--* int
|  |  |  |  |  |  |  |  >--* b
|  |  |  |  |  |  |  |  >--* ;
|  |  |  |  |  |  >--+ var-declaration
|  |  |  |  |  |  |  >--+ type-specifier
|  |  |  |  |  |  |  |  >--* int
|  |  |  |  |  |  |  >--* c
|  |  |  |  |  |  |  >--* ;
|  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  |  |  >--+ statement
|  |  |  |  |  |  |  |  |  >--+ expression-stmt
|  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  >--+ var
|  |  |  |  |  |  |  |  |  |  |  |  >--* a
|  |  |  |  |  |  |  |  |  |  |  >--* =
|  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  >--+ var
|  |  |  |  |  |  |  |  |  |  |  |  |  >--* b
|  |  |  |  |  |  |  |  |  |  |  |  >--* =
|  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  >--+ var
|  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* c
|  |  |  |  |  |  |  |  |  |  |  |  |  >--* =
|  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* (
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ integer
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* 85
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ relop
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* ==
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ integer
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* 84
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ addop
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* +
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ float
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* 0.4
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* )
|  |  |  |  |  |  |  |  |  |  >--* ;
|  |  |  |  |  |  |  >--+ statement
|  |  |  |  |  |  |  |  >--+ selection-stmt
|  |  |  |  |  |  |  |  |  >--* if
|  |  |  |  |  |  |  |  |  >--* (
|  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  >--+ var
|  |  |  |  |  |  |  |  |  |  |  >--* a
|  |  |  |  |  |  |  |  |  |  >--* =
|  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ var
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* b
|  |  |  |  |  |  |  |  |  >--* )
|  |  |  |  |  |  |  |  |  >--+ statement
|  |  |  |  |  |  |  |  |  |  >--+ compound-stmt
|  |  |  |  |  |  |  |  |  |  |  >--* {
|  |  |  |  |  |  |  |  |  |  |  >--+ local-declarations
|  |  |  |  |  |  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  |  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  |  |  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  |  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  |  |  |  |  |  |  >--+ statement
|  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression-stmt
|  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ var
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* GARRAY
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* [
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* (
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ call
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* MyFuncB
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* (
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ args
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* )
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* )
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* ]
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* =
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ var
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* GARRAY
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* [
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ var
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* c
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* =
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ float
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* 1.
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ mulop
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* *
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ float
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* .1
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ relop
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* ==
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ float
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* 1.1
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* ]
|  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* ;
|  |  |  |  |  |  |  |  |  |  |  >--* }
|  |  |  |  |  |  |  |  |  >--* else
|  |  |  |  |  |  |  |  |  >--+ statement
|  |  |  |  |  |  |  |  |  |  >--+ selection-stmt
|  |  |  |  |  |  |  |  |  |  |  >--* if
|  |  |  |  |  |  |  |  |  |  |  >--* (
|  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ call
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* MyFuncC
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* (
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ args
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ arg-list
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--+ var
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* NotDeclared
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* )
|  |  |  |  |  |  |  |  |  |  |  >--* )
|  |  |  |  |  |  |  |  |  |  |  >--+ statement
|  |  |  |  |  |  |  |  |  |  |  |  >--+ compound-stmt
|  |  |  |  |  |  |  |  |  |  |  |  |  >--* {
|  |  |  |  |  |  |  |  |  |  |  |  |  >--+ local-declarations
|  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  |  |  |  |  |  |  |  >--+ statement-list
|  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* epsilon
|  |  |  |  |  |  |  |  |  |  |  |  |  >--* }
|  |  |  |  |  |  |  |  |  |  |  >--* else
|  |  |  |  |  |  |  |  |  |  |  >--+ statement
|  |  |  |  |  |  |  |  |  |  |  |  >--+ expression-stmt
|  |  |  |  |  |  |  |  |  |  |  |  |  >--* ;
|  |  |  |  |  |  >--+ statement
|  |  |  |  |  |  |  >--+ return-stmt
|  |  |  |  |  |  |  |  >--* return
|  |  |  |  |  |  |  |  >--+ expression
|  |  |  |  |  |  |  |  |  >--+ simple-expression
|  |  |  |  |  |  |  |  |  |  >--+ additive-expression
|  |  |  |  |  |  |  |  |  |  |  >--+ term
|  |  |  |  |  |  |  |  |  |  |  |  >--+ factor
|  |  |  |  |  |  |  |  |  |  |  |  |  >--+ float
|  |  |  |  |  |  |  |  |  |  |  |  |  |  >--* 0.
|  |  |  |  |  |  |  |  >--* ;
|  |  |  |  |  >--* }
```



## 实验反馈

