%{
/*
 * Motif
 *
 * Copyright (c) 1987-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these librararies and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */
/*
 * HISTORY
 */

#include <string.h>
#include <stdlib.h>

#ifndef XmConst
#if defined(__STDC__) || !defined( NO_CONST )
#define XmConst const
#else
#define XmConst
#endif /* __STDC__ */
#endif /* XmConst */

	/*
	 * lex program to construct token data for WML to generate token
	 * table data.
 	 *
	 * stdin is the file Uil.y
	 *
	 * it is searched for each occurance of a line starting with '%token'
	 * when found various fields are extracted.
	 *
	 * when Uil.y has been scanned the collected data is written
	 * to stdout.
	 */

%}

%a 9999						/* transitions */

%option main

%{

#include <stdio.h>

#define TRUE  1
#define FALSE 0

#define MAX_SYN 10

#ifdef yywrap
#undef yywrap
#endif

typedef struct _token
{
    char * name;
    char * id;
    char * class;
    int    num_syn;
    char * syn[MAX_SYN];
}
    TokenRec, *Token;

TokenRec token[1000];
TokenRec temp;

void add_token ();
int yywrap ();

int phase		= 0;
int keyword_count;
int line_num		= 0;
int in_comment		= 0;
int in_include		= 0;
int in_token		= 0;
int state		= 0;
int used		= 0;

%}



%%


[\n]		{
		    if (in_token == TRUE)
		    {
			add_token (&temp);
		    }

		    in_token = FALSE;
		    line_num++;
		}

[ \t]		{				/* swallow */
		} 


"/\*"		{				/* swallow */
		}


^%token		{
		    in_token = TRUE;
		    state = 0;
		}

[a-zA-Z0-9_]+	{
		    if (in_token == TRUE)
		    {
			switch (state)
			{
			    case 0:
				temp.name = (char *)
				    strcpy ((char *)
					malloc (strlen ((XmConst char *)yytext) + 1), (XmConst char *)yytext);

				temp.num_syn = 0; 
				break;

			    case 1:
				temp.id = (char *)
				    strcpy ((char *)
					malloc (strlen ((XmConst char *)yytext) + 1), (XmConst char *)yytext);
				break;

			    case 2:
				temp.class = (char *)
				    strcpy ((char *)
					malloc (strlen ((XmConst char *)yytext) + 1), (XmConst char *)yytext);
				break;

			    case 3:
				temp.syn[temp.num_syn] = (char *)
				    strcpy ((char *)
					malloc (strlen ((XmConst char *)yytext) + 1), (XmConst char *)yytext);
				temp.num_syn++;

				if (temp.num_syn > MAX_SYN)
				    printf (
					"ERROR, too many synonyms, line %d\n",
					line_num);
				break;
			}

			state++;
		    }
		}

.		{				/* swallow */
		}

%%


void
add_token (t)					/* keep sorted by name */
    Token t;
{
    int i, j;

    for (j=0; j<used; j++)			/* for each token */
    {
	if (strcmp (token[j].name, t->name) > 0)	/* goes here */
  	{
	    for (i=used++; i>j; i--)		/* make hole */
		token[i] = token[i-1];

	    token[j] = *t;			/* insert it */

	    return;
	}
    }

    /*
     * if we get there then it goes at the end of the list 
     */

    token[used++] = *t;
}



int yywrap ()
{
    int i;

    for (i=0; i<used; i++)
	 { 
	 printf ("%s %s %s\n", token[i].name, token[i].id, token[i].class);
	 }
    return (1);
}
