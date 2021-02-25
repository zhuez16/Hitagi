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

/***************
 TO STUDENTS: Copy your Lab1 here. Make adjustments if necessary.

 Note: dont modify the prologue unless you know what you are doing.
***************/

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

%%

 /******************TODO*********************/
 /****请在此补全所有flex的模式与动作  start******/
 /* STUDENT TO DO */
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




 /****请在此补全所有flex的模式与动作  end******/
%%
/****************C代码 start*************/

/// \brief analysize a *.cminus file
///
/// \param input_file, 需要分析的文件路径
/// \param token stream, Token_Node结构体数组，用于存储分析结果，具体定义参考lexical_analyer.h

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



/****************C代码 end*************/

/****************C代码 end*************/



/* Example for you :-) */
//\+  { pos_start = pos_end; pos_end += 1; pass_node(yytext); return ADD; }
