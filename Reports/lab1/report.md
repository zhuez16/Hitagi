# lab1实验报告
PB18111704 朱恩佐
## 实验要求

本次实验需要各位同学根据`cminux-f`的词法补全[lexical_analyer.l]()文件，完成词法分析器，能够输出识别出的`token`，`type` ,`line(刚出现的行数)`，`pos_start(该行开始位置)`，`pos_end(结束的位置,不包含)`。如：

## 实验难点

###### 1、注释的识别

​	注释由“/*”字符开始，由“\*/”字符结束。且识别注释的时候要找到最接近的一对开始符和结束符号。因此正则表达式应当表达出来在一段连续的注释中不可以出现\*/符号。因此采用(/\*([\^\*]|[\*]\*[\^\*/])\*[\*]\*\*/)来进行识别。其中先识别注释的开头/\*，然后中间段识别时要求不可以出现\*/符号。所以如果我们想要在中间段出现一个\*号，那么我们必须保证\*号后面不得出现/。因此我们在中间设定[\*]\*[\^\*/]，描述注释中一段连续的\*号后不存在/号的事情。

##### 2、多行注释维护行号和位置

​	在出现多行注释的时候由于注释内部可能出现换行，因此不能简单粗暴的像其他token一样简单的让end=start+yyleng来维护下一个字符的位置。因此我们需要扫描整个token来维护下一个token的开始位置。

​	我们在返回了“本token是注释”之后进行维护行列的操作。对于yytext中的每一个字符，如果该字符不是换行符，则我们没有换行，进行pos_end++。如果该字符是换行符，则我们需要将行树+1，并将pos_end=0。这样我们就可以保证在出现了多段注释，我们的行数和位置是正确的。

3、优先级问题

​	对于一些token比如EQ(==)和ASSIN(=)，GTE(>=)和GT(>)，还有可能出现的一些特殊命名变量比如voidint等，我们需要保证每一次识别串识别尽量长的一段来确保不会将前者错误的识别成后者。不过Lex本身的识别规则就是“贪婪识别”，即每个正则表达式都尽量匹配更长的串。故不需要在设计中刻意考虑。

## 实验设计

##### Step1 设计模式识别

对于不同的可能的token，我们要分别设计其正则表达式。代码如下：

``` C++
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
IDENTIFIER [a-zA-Z]+
INTEGER [0-9]+
FLOATPOINT [0-9]+[.][0-9]* 
EOL [\n] 
BLANK [ ]|[\t]
```

注意对于各种符号比如'['，']'，在插入正则表达式的时候由于其本身有其他表达意思，所以需要增加'\\'转译。

##### Step2 匹配模式后维护位置

对于每一种token，我们需要让光标向后推一定长度。考虑到有一些token是定长的，有一些token是不定长的，所以对于定长的字符我们采用直接加固定长度，不定长的让位置位移yyleng位，即可保证每识别一个token，我们的位置都是正确的。

```C++
{COMMENT} 	{pos_start=pos_end;                    return COMMENT;}
{ELSE} 		{pos_start=pos_end;pos_end=pos_start+4;return ELSE;}
{IF} 		{pos_start=pos_end;pos_end=pos_start+2;return IF;}
{INT} 		{pos_start=pos_end;pos_end=pos_start+3;return INT;}
{FLOAT} 	{pos_start=pos_end;pos_end=pos_start+5;return FLOAT;}
{RETURN} 	{pos_start=pos_end;pos_end=pos_start+6;return RETURN;}
{VOID} 		{pos_start=pos_end;pos_end=pos_start+4;return VOID;}
{WHILE} 	{pos_start=pos_end;pos_end=pos_start+5;return WHILE;}
{ARRAY} 	{pos_start=pos_end;pos_end=pos_start+2;return ARRAY;}
{EQ} 		{pos_start=pos_end;pos_end=pos_start+2;return EQ;}
{NEQ} 		{pos_start=pos_end;pos_end=pos_start+2;return NEQ;}
{GTE} 		{pos_start=pos_end;pos_end=pos_start+2;return GTE;}
{LTE} 		{pos_start=pos_end;pos_end=pos_start+2;return LTE;}
{ADD} 		{pos_start=pos_end;pos_end=pos_start+1;return ADD;}
{SUB} 		{pos_start=pos_end;pos_end=pos_start+1;return SUB;}
{MUL} 		{pos_start=pos_end;pos_end=pos_start+1;return MUL;}
{DIV}	 	{pos_start=pos_end;pos_end=pos_start+1;return DIV;}
{LT} 		{pos_start=pos_end;pos_end=pos_start+1;return LT;}
{GT} 		{pos_start=pos_end;pos_end=pos_start+1;return GT;}
{ASSIN} 	{pos_start=pos_end;pos_end=pos_start+1;return ASSIN;}
{SEMICOLON} 	{pos_start=pos_end;pos_end=pos_start+1;return SEMICOLON;}
{COMMA} 	{pos_start=pos_end;pos_end=pos_start+1;return COMMA;}
{LPARENTHESE} 	{pos_start=pos_end;pos_end=pos_start+1;return LPARENTHESE;}
{RPARENTHESE} 	{pos_start=pos_end;pos_end=pos_start+1;return RPARENTHESE;}
{LBRACKET} 	{pos_start=pos_end;pos_end=pos_start+1;return LBRACKET;}
{RBRACKET} 	{pos_start=pos_end;pos_end=pos_start+1;return RBRACKET;}
{LBRACE} 	{pos_start=pos_end;pos_end=pos_start+1;return LBRACE;}
{RBRACK} 	{pos_start=pos_end;pos_end=pos_start+1;return RBRACE;}
{IDENTIFIER} 	{pos_start=pos_end;pos_end=pos_start+yyleng;return IDENTIFIER;}
{INTEGER} 	{pos_start=pos_end;pos_end=pos_start+yyleng;return INTEGER;}
{FLOATPOINT} 	{pos_start=pos_end;pos_end=pos_start+yyleng;return FLOATPOINT;}
{EOL} 		{pos_start=pos_end;pos_end=pos_start+1;return EOL;}
{BLANK} 	{pos_start=pos_end;pos_end=pos_start+1;return BLANK;}
. {return ERROR;}
```

##### Step3 忽视token处理

对于BLANK、EOL、COMMENT三类字符，由于其与程序的运行无关，所以词法分析的时候就需要将其剥离。其中EOL字符我们需要重置pos和line，COMMENT需要找到新的pos。代码如下：

```C++
case COMMENT:
		for (i = 0;i <= yyleng; i++){
			pos_end++;
			if (yytext[i]=='\n'){
				lines++;
				pos_end=0;
			}
		}
		break;
case BLANK:
    break;
case EOL:
		lines++;
		pos_start=1;
		pos_end=1;
    break;
```

## 实验结果验证

输入代码如下：

```C
/* 88** void main int **else* /* 8.7275-15*/
void main/**/(int a, int b){
	/*
	*/
	int ifelse, voidmain;
	if (a==b){
		ifelse = /*pendulum*/    	0;
		voidmain = b;
	}
	/*{*/else/**/{
		ifelse = 12.;
		voidmain = a;
	}
	return /* int main(); pel;
	/*****
	*/ b;
}
```

测试数据考虑到多行注释、特殊的（一年级的学生特别喜欢的）变量命名、不正规的空格、中间穿插的注释和注释中有关键字这几种情况。

输出结果如下：

```c
void    283     2       1       5
main    285     2       6       10
(       272     2       15      16
int     280     2       16      19
a       285     2       20      21
,       271     2       21      22
int     280     2       23      26
b       285     2       27      28
)       273     2       28      29
{       276     2       29      30
int     280     5       2       5
ifelse  285     5       6       12
,       271     5       12      13
voidmain        285     5       14      22
;       270     5       22      23
if      279     6       2       4
(       272     6       5       6
a       285     6       6       7
==      267     6       7       9
b       285     6       9       10
)       273     6       10      11
{       276     6       11      12
ifelse  285     7       3       9
=       269     7       10      11
0       286     7       30      31
;       270     7       31      32
voidmain        285     8       3       11
=       269     8       12      13
b       285     8       14      15
;       270     8       15      16
}       277     9       2       3
else    278     10      8       12
{       276     10      17      18
ifelse  285     11      3       9
=       269     11      10      11
12.     287     11      12      15
;       270     11      15      16
voidmain        285     12      3       11
=       269     12      12      13
a       285     12      14      15
;       270     12      15      16
}       277     13      2       3
return  282     14      2       8
b       285     16      5       6
;       270     16      6       7
}       277     17      1       2
```

可以大概感觉代码是正确的。

## 实验反馈

（希望助教能够提供加密了的最终测试数据和相应的测试脚本，让我们做实验稍微能安心一点。）

希望万一有错误后面还有改正的空间。。。