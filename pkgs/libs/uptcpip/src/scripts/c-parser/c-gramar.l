D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*
ws			[ \t\r\v\f]
wsnl		[ \t\r\v\f\n]
identifier	[a-zA-Z_$][a-zA-Z_0-9$]*
number		\.?[0-9][.0-9a-fA-FlLuUxX]*

%{
#include <stdio.h>
#include "y.tab.h"

static int token;
void count();
%}

%%
"/*"			{ comment1(); }
"//"			{ comment2(); }

struct{wsnl}+{identifier}{wsnl}+.*\{		{ 
					token = STRUCT; 
					count(); 
					return (token); 
				}

typedef{wsnl}+{identifier}{wsnl}+.*;	{ 
					token = TYPEDEF; 
					count(); 
					return (token); 
				}

union{wsnl}+{identifier}{wsnl}+.*\{		{ 
					token = UNION; 
					count(); 
					return (token); 
				}

\#{ws}*define{ws}+{identifier}	{
					token = UNION; 
					count(); 
					return (token); 
				}

{identifier}([*]*{identifier}*{wsnl}*)+\(({wsnl}|[a-zA-Z0-9_*,()])*\)({wsnl}*{identifier}{wsnl}*[()0-9]*)*[;|{]	{
					token = FUNCDEF; 
					count();
					return (token);
				}

{wsnl}+			{	}
.				{ /* ignore bad characters */ }

%%

int word_count = 0;


yywrap()
{
//	printf("Word count = %d, Bye\n", word_count);
	return(1);
}


comment1()
{
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0)
		;

	if ((c1 = input()) != '/' && c != 0)
	{
		unput(c1);
		goto loop;
	}

}

comment2()
{
	char c;

	while ((c = input()) != '\n' && c != 0)
		;
}



void count()
{
	int i;

	word_count ++;
/*
	switch(token){
	
	case TYPEDEF:
		 break;

	case STRUCT:
	case UNION:
		 break;

	case ENUM: 
		 break;

	}
*/
	if (token == FUNCDEF){
		for (i = 0; yytext[i] != '\0'; i++){
			if (yytext[i] != '\n')
				 putchar(yytext[i]);
			else putchar(' ');
		}

		printf("\n");
	}
}


