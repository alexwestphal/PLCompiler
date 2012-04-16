/** @file p3.c
 *  @author
 *      Pascal Version: Nickolas Wirth
 *      C Version: Tadao Takaoka
 *      Enhanced C Version: Alex Westphal (98196992)
 *  @version 19-08-2009
 *
 * Implements else statments and arrays
 *
 * sign != given by #
 * sign <= given by $
 * sign >= given by %
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>



/* no. of reserved words */
#define NORW    13

/* length of indentifier table */
#define TXMAX   1000

/* max. no. of digits in numbers */
#define NMAX    14

/* length of identifier */
#define AL      10

/* maximum address */
#define AMAX    2047

/* maximum depth of block nesting */
#define LEVMAX  3

/* size of code array */
#define CXMAX   20000

/* stack size*/
#define SMAX 500

/* Internal Symbols*/
#define NUL         0
#define IDENT       2
#define NUMBER      3
#define PLUS        4
#define MINUS       5
#define MULT        6
#define SLASH       7
#define ODDSYM      8
#define EQL         9
#define NEQ         10
#define LSS         11
#define LEQ         12
#define GTR         13
#define GEQ         14
#define LPAREN      15
#define RPAREN      16
#define COMMA       17
#define SEMICOLON   18
#define PERIOD      19
#define BECOMES     20
#define BEGINSYM    21
#define ENDSYM      22
#define IFSYM       23
#define THENSYM     24
#define ELSESYM     35
#define WHILESYM    26
#define DOSYM       27
#define CALLSYM     28
#define CONSTSYM    29
#define VARSYM      30
#define PROCSYM     31
#define WRITESYM    32
#define AMPER       33
#define LBRACK      34
#define RBRACK      35

#define CONSTANT    1
#define VARIABLE    2
#define PROCEDURE   3
#define ARRAY       4

/*Machine Instructions*/
#define LIT         1 /*Load Literal*/
#define OPR         2 /*Operator*/
#define LOD         3 /*Load Integer*/
#define STO         4 /*Store Intger*/
#define CAL         5 /*Call Procedure*/
#define INC         6 /*Increase Stack Size*/
#define JMP         7 /*Unconditional Jump*/
#define JPC         8 /*Conditional Jump*/
#define WRT         9 /*Write To Screen*/
#define STP         10 /*Create Stack Frame*/
#define LID         11 /*Load Indirect*/
#define LDA         12 /*Load Address*/
#define SID         13 /*Store Indirect*/

/*Operators*/
#define OPR_RET     0 /*Return*/
#define OPR_NEG     1 /*Negate*/
#define OPR_ADD     2 /*Add*/
#define OPR_SUB     3 /*Subtract*/
#define OPR_MUL     4 /*Multiply*/
#define OPR_DIV     5 /*Divide*/
#define OPR_ODD     6 /*Odd*/
#define OPR_MOD     7 /*Modulus*/
#define OPR_EQL     8 /*Equal*/
#define OPR_NEQ     9 /*Not Equal*/
#define OPR_LSS     10 /*Less*/
#define OPR_LEQ     11 /*Less or Equal*/
#define OPR_GTR     12 /*Greater*/
#define OPR_GEQ     13 /*Greater or Equal*/



struct namerecord
{
  int kind;
  char name[10];
  int val;
  int level;
  int adr;
  int indirect;
};

struct instruction
{
  int f; 
  int l;
  int a;
};

char line[81];

char ch; /*last character read */
int  sym ;    /* last symbol read */
char *id;  // last identifier read
int num;   // last number read
int cc;   // character count
int ll;   // line length
int cx;   //code allocation index
char a[20];
int s[SMAX]; //datastore
struct namerecord table[TXMAX];
struct instruction code[CXMAX];
char *word[NORW+1];
int wsym[100];
int ssym[100];
int flag;
char fname[10];
FILE *f;
char *errmsg[100];
int result = EXIT_SUCCESS;



/**
 * Print an error message.
 * @param msgid Error message to print
 */
void error(msgid)
    int msgid;
{
    printf("error %d - %s\n",msgid, errmsg[msgid]);
    result = EXIT_FAILURE;
}



/**
 * Fetch in a line from the file (f)
 * Stores in (line)
 */
int getln()
{
    int i, n=0; char c;

    cc=0; ll=0;
    while ((c=getc(f))!='\n')
    {
        if (c==EOF)
        {
            ch=-1;
            return 0;
        }
        n++; ll++; line[n]=c;
    }
    n++; ll++; line[n]=' ';
    for(i=1; i<=n; i++) printf("%c", line[i]);
    printf("\n");
    return 1;
}



/**
 * Get the next charcter
 * Stores in (ch)
 */
int getch()
{
    if (cc==ll)
    {
        if (ch==-1)
        {
            printf("PROGRAM INCOMPLETE");
            getchar();
            return NUL;
        }
        getln();
    }
    cc=cc+1;
    ch=line[cc];
}



/**
 * Check if reserved word
 * @param index of word, or 0 if not found
 */
int search()
{
    int i, j, len;
    char *s;
    for (i=NORW; i>=1; i--)
    {
        s=word[i];
        len=strlen(s);
        for (j=1; j<=len; j++)
        {
            if(a[j]!=*s) break;
            s++;
        }
        if (j>len) return i;
    }
    return 0;
}



/**
 * Get the next Symbol
 * Stores in (sym)
 * @return 0 if no symbol is available
 */
int getsym()
{
    int k=0, i;

    while (ch<=' ') getch();

    /*Identifier*/
    if(((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z')))
    {
        do
        {
            if (k<AL) { k++; a[k]=ch; }
            getch();
        }
        while(((ch>='0') && (ch<='9')) ||
              ((ch>='A') && (ch<='Z')) ||
              ((ch>='^') && (ch<='z')));

        a[k+1]=0;
        id=a+1;

        if ((i=search())!=0) sym=wsym[i]; else sym=IDENT;
    }


    /*Number*/
    else if((ch>='0') && (ch <= '9'))
    {
        k=0; num=0; sym = NUMBER;

        do
        {
            num=10*num + (ch - '0');
            k++; getch();
        }
        while ((ch >= '0') && (ch <= '9'));

        if (k > NMAX) error(30);
    }


    /*Becomes*/
    else if(ch==':')
    {
        getch();
        if (ch=='=')
        {
            sym = BECOMES;
            getch();
        }
        else sym=0;
    }

    
    else if (ch==-1) return NUL;
    else
    {
        sym = ssym[ch];
        getch();
    }
}



/**
 * Generate an Instruction
 * @param x Opcode
 * @param y Level Difference
 * @param z Argument
 */
void gen(x, y, z)
    int x,y,z;
{
    if (cx>CXMAX) printf("program too long\n");
    else
    {
        code[cx].f=x;
        code[cx].l=y;
        code[cx].a=z;
    }
    cx++;
}



/**
 * Enter a Symbol into the table
 * @param k Kind of variable
 * @param ptx Pointer to table index
 * @param pdx Pointer to stack index
 * @param lev Variable level
 * @param size Variable size
 */
void enter(k, ptx, pdx, lev, size)
    int k; int *ptx; int *pdx;
    int lev; int size;
{
    char *id1;
    int ii,len;

    (*ptx)++;
    id1=id;
    len=strlen(id);

    for (ii=0;ii<=len;ii++)
    {
        table[*ptx].name[ii]=*id1;
        id1++;
    }

    table[*ptx].kind=k;

    if (k==CONSTANT) table[*ptx].val=num;
    else if (k==VARIABLE || k==ARRAY)
    {
        table[*ptx].level=lev;
        table[*ptx].adr=*pdx;
        table[*ptx].indirect=flag;
        (*pdx)+=size;
    }
    else table[*ptx].level=lev;
}



/**
 * Check if the specified strings are not the same
 * @param a First string to compare
 * @param b Second string to compare
 * @return true if the specified strings are not the same
 */
int mismatch(a,b)
    char *a; char *b;
{
    if(strcmp(a, b)==0) return 0; else return 1;
}



/**
 * Find position of an id in table
 * @param id Id to find
 * @param ptx Pointer to table index
 * @return The index of the id or 0 if not found
 */
int position(id, ptx)
    char *id; int *ptx;
{
    int i=*ptx;
    strcpy(table[0].name, id);

    while (mismatch(table[i].name, id))
    {
        i--;
    }
    return i;
}



/**
 * Process a constant declaration
 * @param lev Level
 * @param ptx Pointer to table index
 * @param pdx Pointer to stack index
 */
void constdeclaration(lev, ptx, pdx)
    int lev,*ptx,*pdx;
{
    if (sym==IDENT)
    {
        getsym();
        if ((sym==EQL) || (sym==BECOMES))
        {
            if (sym==BECOMES) error(1);
            getsym();
            if (sym==NUMBER) {
                enter(CONSTANT,ptx,pdx,lev,0);
                getsym();
            }
        }
    }
}



/**
 * Process a variable declaration
 * @param lev Level
 * @param ptx Pointer to table index
 * @param pdx Pointer to stack index
 */
void vardeclaration(lev,ptx,pdx)
    int *ptx,*pdx,lev;
{
    if (sym==IDENT)
    {
        getsym();

        if(sym==LBRACK)
        {
            getsym();
            if(sym==NUMBER)
            {
                enter(ARRAY,ptx,pdx,lev,num);
                getsym();
            }
            else error(25);
            if(sym==RBRACK) getsym(); else error(27);
        }
        else enter(VARIABLE,ptx,pdx,lev,1);

    }
    else error(4);
}



/**
 * Declare expression function
 */
void expression(int, int*);



/**
 * Process a factor
 * @param lev Level
 * @param ptx Pointer to table index
 */
void factor(lev, ptx)
    int lev, *ptx;
{
    int i, level, adr, kind, val;

    while ((sym==IDENT) || (sym==NUMBER) || (sym==LPAREN))
    {
        /*Variable, Constant or Array*/
        if (sym==IDENT)
        {
            i=position(id,ptx);
            if (i==0) error(11);
            else
            {
                kind=table[i].kind;
                level=table[i].level;
                adr=table[i].adr;
                val=table[i].val;

                /*Constant*/
                if (kind==CONSTANT) gen(LIT,0,val);

                /*Variable*/
                else if (kind==VARIABLE)
                {
                    if (table[i].indirect==0) gen(LOD,lev-level,adr);
                    else gen(LID,lev-level,adr);
                }

                /*Array*/
                else if(kind==ARRAY)
                {
                    getsym();
                    if(sym==LBRACK) getsym(); else error(26);
                    gen(LDA, lev-level, adr);
                    expression(lev,ptx);
                    gen(OPR, 0, OPR_ADD);
                    gen(STO, lev, 1);
                    if(sym!=RBRACK) error(27);
                    gen(LID, lev, 1);
                }
                else error(21);
            }
            getsym();
        }

        /*Number*/
        else if(sym==NUMBER)
        {
            if (num>AMAX)
            {
                error(31);
                num=0;
            }
            gen(LIT,0,num);
            getsym();
        }

        /*Nested Expression*/
        else if(sym==LPAREN)
        {
            getsym();
            expression(lev,ptx);
            if (sym==RPAREN) getsym(); else error(22);
        }
    }
}



/**
 * Process a term
 * @param lev Level
 * @param ptx Pointer to table index
 */
void term(lev,ptx)
    int lev, *ptx;
{
    int mulop;

    factor(lev,ptx);

    while((sym==MULT) || (sym==SLASH))
    {
        mulop=sym;
        getsym();
        factor(lev,ptx);
        if (mulop==MULT) gen(OPR, 0, OPR_MUL); else gen(OPR, 0, OPR_DIV);
    }
}



/**
 * Process an expression
 * @param lev Level
 * @param ptx Pointer to table index
 */
void expression(lev, ptx)
    int lev, *ptx;
{
    int addop;

    if ((sym==PLUS) || (sym==MINUS))
    {
        addop = sym;
        getsym();
        term(lev, ptx);
        if (addop==MINUS) gen(OPR, 0, OPR_NEQ);
    }
    else term(lev, ptx);

    while ((sym==PLUS) || (sym==MINUS))
    {
        addop = sym;
        getsym();
        term(lev, ptx);
        if (addop==PLUS) gen(OPR, 0, OPR_ADD); else gen(OPR, 0, OPR_SUB);
    }
}



/**
 * Process a condition
 * @param lev Level
 * @param ptx Pointer to table index
 */
void condition(lev, ptx)
    int lev, *ptx;
{
    int relop;

    if(sym==ODDSYM)
    {
        getsym();
        expression(lev, ptx);
        gen(OPR, 0, OPR_ODD);
    }
    else
    {
        expression(lev, ptx);
        relop = sym;
        getsym();
        expression(lev, ptx);
        switch (relop)
        {
            case EQL: gen(OPR, 0, OPR_EQL); break;
            case NEQ: gen(OPR, 0, OPR_NEQ); break;
            case LSS: gen(OPR, 0, OPR_LSS); break;
            case LEQ: gen(OPR, 0, OPR_LEQ); break;
            case GTR: gen(OPR, 0, OPR_GTR); break;
            case GEQ: gen(OPR, 0, OPR_GEQ); break;
            default: error(20); break;
        }
    }
}



/**
 * Formal parameter
 * @param lev Level
 * @param ptx Pointer to table index
 * @param pdx Pointer to stack index
 */
void fparam(lev, ptx, pdx)
    int lev, *ptx,*pdx;
{
    if (sym==VARSYM)
    {
        flag=1;
        getsym();
    }
    else flag=0;
    
    if(sym==IDENT) vardeclaration(lev,ptx,pdx); else error(81);

    flag=0;
    while (sym==COMMA)
    {
        getsym();
        if (sym==VARSYM)
        {
            flag=1;
            getsym();
        }
        else flag=0;
        vardeclaration(lev,ptx,pdx);
        flag=0;
   }
   if(sym!=RPAREN) error(82);
}



/**
 * Actual parameter
 * @param lev Level
 * @param ptx Pointer to table index
 */
void aparam(lev,ptx)
    int lev, *ptx;
{
    int i, pnum=0, adr, level;

    if(sym==AMPER)
    {
        getsym();
        if(sym!=IDENT) error(70);
        i=position(id,ptx);
        adr=table[i].adr;
        level=table[i].level;
        gen(LDA,lev-level,adr);
        getsym();
    }
    else expression(lev,ptx);

    pnum++;
    while (sym==COMMA)
    {
        getsym();
        if(sym==AMPER)
        {
            getsym();
            if (sym!=IDENT) error(70);
            i=position(id,ptx);
            adr=table[i].adr;
            level=table[i].level;
            gen(LDA,lev-level,adr);
            getsym();
        }
        else expression(lev,ptx);
        pnum++;
    }
    while(pnum>0)
    {
        gen(STP,0,0);
        pnum--;
    }
}



/**
 * Process a statement
 * @param lev Level
 * @param ptx Pointer to table index
 */
void statement(lev, ptx)
    int lev, *ptx;
{
    int i, cx1, cx2;

    /*Identifier Becomes*/
    if (sym==IDENT)
    {
        i=position(id, ptx);

        if(i==0) error(11);

        /*Variable*/
        else if (table[i].kind==VARIABLE)
        {
            getsym();
            if (sym==BECOMES) getsym(); else error(13);
            expression(lev, ptx);

            if (table[i].indirect==0)
                gen(STO,lev-table[i].level,table[i].adr);
            else
                gen(SID, lev-table[i].level, table[i].adr);
        }

        /*Array*/
        else if(table[i].kind==ARRAY)
        {
            getsym();
            if(sym==LBRACK) getsym(); else error(26);
            gen(LDA, lev-table[i].level, table[i].adr);
            expression(lev,ptx);
            gen(OPR,0,OPR_ADD);
            gen(STO,lev,0);
            if(sym==RBRACK) getsym(); else error(27);
            if(sym==BECOMES) getsym(); else error(13);
            expression(lev, ptx);
            gen(SID,lev,0);
        }
        else error(12);
    }

    /*Procedure Call*/
    else if (sym==CALLSYM)
    {
        getsym();
        if (sym!=IDENT) error(14);
        else
        {
            i=position(id, ptx);
            if(i==0) error(11);
            else
            {
                getsym();
                if (sym==LPAREN)
                {
                    getsym();
                    aparam(lev, ptx);
                    if (sym==RPAREN) getsym(); else error(89);
                }
                if (table[i].kind==PROCEDURE)
                    gen(CAL,lev-table[i].level, table[i].adr);
                else error(15);
            }
        }
    }

    /*If Statement*/
    else if (sym==IFSYM)
    {
        getsym();
        condition(lev,ptx);
        if (sym==THENSYM) getsym(); else error(16);
        cx1 = cx;
        gen(JPC, 0, 0);
        statement(lev, ptx);

        /*Else Statement*/
        if(sym==ELSESYM)
        {
            cx2 = cx;
            gen(JMP, 0, 0);
            code[cx1].a = cx;
            getsym();
            statement(lev, ptx);
            code[cx2].a = cx;
        }
        else code[cx1].a = cx;
    }

    /*Begin*/
    else if (sym==BEGINSYM)
    {
        getsym();
        statement(lev,ptx);

        while((sym==SEMICOLON) || (sym==BEGINSYM) || 
                (sym==IFSYM) || (sym==WHILESYM) ||
                (sym==CALLSYM) || (sym==WRITESYM))
        {
            if(sym==SEMICOLON) getsym(); else error(10);
            statement(lev,ptx);
        }
        if (sym==ENDSYM) getsym(); else error(17);
    }

    /*While Loop*/
    else if (sym==WHILESYM)
    {
        cx1 = cx;
        getsym();
        condition(lev, ptx);
        cx2 = cx;
        gen(JPC, 0, 0);
        if (sym==DOSYM) getsym(); else error(18);
        statement(lev, ptx);
        gen(JMP,0, cx1);
        code[cx2].a = cx;
    }

    /*Write*/
    else if (sym==WRITESYM)
    {
        getsym();
        if (sym!=LPAREN) error(98);
        else
        {
            getsym();
            expression(lev, ptx);
            gen(WRT, 0, 0);
            if (sym==RPAREN) getsym(); else error(99);
        }
    }
}



/**
 * Process a block
 * @param lev Level
 * @param ptx Pointer to table index
 */
void block(lev, tx)
    int lev; int tx;
{
    int dx, tx0, cx0;
    dx=3; tx0 = tx;
    table[tx].adr = cx;
    gen(JMP,0,0);
    if (lev>LEVMAX) error(32);
    do
    {
        /*Constant Declarations*/
        if (sym==CONSTSYM)
        {
            getsym();
            do
            {
                constdeclaration(lev, &tx, &dx);
                while(sym==COMMA)
                {
                    getsym();
                    constdeclaration(lev, &tx, &dx);
                }
                if(sym==SEMICOLON) getsym(); else error(5);
            }
            while (sym==IDENT);
        }

        /*Variable Declarations*/
        if (sym==VARSYM)
        {
            getsym();
            do
            {
                vardeclaration(lev,&tx,&dx);
                while (sym==COMMA)
                {
                    getsym();
                    vardeclaration(lev, &tx, &dx);
                }
                if(sym==SEMICOLON) getsym(); else error(5);
            }
            while(sym==IDENT);
        }

        if (sym==LPAREN)
        {
            getsym();
            fparam(lev, &tx, &dx);
            if (sym==RPAREN) getsym(); else error(88);
        }

        /*Procedure Declaration*/
        while(sym==PROCSYM)
        {
            getsym();
            if(sym==IDENT)
            {
                enter(PROCEDURE,&tx, &dx, lev);
                getsym();
            }
            else error(4);

            if (sym==SEMICOLON) getsym();
            else if (sym==LPAREN);
            else error(5);

            block(lev+1, tx);
            if(sym==SEMICOLON) getsym();
            
            else error(5);
        }
  }
  while ((sym==CONSTSYM) || (sym==VARSYM) || (sym==PROCSYM));

  code[table[tx0].adr].a = cx;
  table[tx0].adr = cx;
  cx0 = cx;
  gen(INC, 0, dx);
  statement(lev, &tx);
  gen(OPR, 0, 0);

}



/**
 * Find the base 1 level down
 * @param l Level
 * @param b Current Base
 */
int base(l,b)
    int l,b;
{
    int b1;

    b1=b;
    while (l>0)
    {
        b1 = s[b1];
        l--;
    }
    return b1;
}



/**
 * Interpret the generated object code
 */
void interpret()
{
    int p=0, b=1, t=0; //program-, base-, topstack-registers
    int i; // instruction register

    printf("start PL/0\n");
    s[1]=0; s[2]=0; s[3]=0;
    
    do
    {
        i = code[p].f;
        switch (i)
        {
            case LIT: t++; s[t]=code[p].a; p++; break; //lit
            case OPR:
                switch (code[p].a)
                {
                    case OPR_RET:   t=b-1;  p=s[t+3]; b=s[t+2];        break;

                    case OPR_NEG:       s[t] = (        -s[t]);   p++; break;
                    case OPR_ADD: t--;  s[t] = (s[t] +  s[t+1]);  p++; break;
                    case OPR_SUB: t--;  s[t] = (s[t] -  s[t+1]);  p++; break;
                    case OPR_MUL: t--;  s[t] = (s[t] *  s[t+1]);  p++; break;
                    case OPR_DIV: t--;  s[t] = (s[t] /  s[t+1]);  p++; break;
                    case OPR_ODD:       s[t] = (s[t] %  2);       p++; break;
                    case OPR_MOD: t--;  s[t] = (s[t] %  s[t+1]);  p++; break;
                    case OPR_EQL: t--;  s[t] = (s[t] == s[t+1]);  p++; break;
                    case OPR_NEQ: t--;  s[t]=  (s[t] != s[t+1]);  p++; break;
                    case OPR_LSS: t--;  s[t]=  (s[t] <  s[t+1]);  p++; break;
                    case OPR_LEQ: t--;  s[t]=  (s[t] <= s[t+1]);  p++; break;
                    case OPR_GTR: t--;  s[t]=  (s[t] >  s[t+1]);  p++; break;
                    case OPR_GEQ: t--;  s[t]=  (s[t] >= s[t+1]);  p++; break;
                }
                break;
            case LOD:
                t++;
                s[t] = s[ base(code[p].l, b) + code[p].a ];
                p++;
                break;
            case STO: 
                s[ base(code[p].l, b) + code[p].a ] = s[t];
                p++;
                t--;
                break;
            case CAL: 
                s[t+1] = base(code[p].l, b);
                s[t+2] = b;
                s[t+3] = p + 1;
                b = t + 1;
                p = code[p].a;
                break;
            case INC: 
                t += code[p].a;
                p++;
                break;
            case JMP: 
                p = code[p].a;
                break;
            case JPC: 
                if (s[t]==0) p=code[p].a;
                else p++;
                t--;
                break;
            case WRT: 
                printf("%d\n", s[t]);
                t--;
                p++;
                break;
            case STP: 
                s[t+3] = s[t];
                t--;
                p++;
                break;
            case LID:
                t++;
                s[t] =s [ s[ base(code[p].l, b) + code[p].a ] ];
                p++;
                break;
            case LDA:
                t++;
                s[t] = base(code[p].l, b) + code[p].a;
                p++;
                break;
            case SID: 
                s[ s[ base(code[p].l, b) + code[p].a ] ] = s[t];
                p++;
                t--;
                break;
	}

        printf("%d - ", p);
        for(i=0; i<t; i++) {
            printf("%d ", s[i]);
        }
        printf("\n");
        
    }
    while (p!=0);
    printf("END PL/0\n");
}



/**
 * Run the program
 * @param argc The number of command line arguments
 * @param argv The list of command line arguments
 */
int main(argc, argv)
    int argc;
    char *argv[];
{
    int i;

    // External symbols in dictionary "word"
    word[ 1] =  "begin" ;   word[ 2] =  "call"      ;   word[ 3] =  "const" ;
    word[ 4] =  "do"    ;   word[ 5] =  "end"       ;   word[ 6] =  "if"    ;
    word[ 7] =  "odd"   ;   word[ 8] =  "procedure" ;   word[ 9] =  "then"  ;
    word[10] =  "var"   ;   word[11] =  "while"     ;   word[12] =  "write" ;
    word[13] =  "else"  ;

    wsym[ 1] =  BEGINSYM;   wsym[ 2] =  CALLSYM     ;   wsym[ 3] =  CONSTSYM;
    wsym[ 4] =  DOSYM   ;   wsym[ 5] =  ENDSYM      ;   wsym[ 6] =  IFSYM   ;
    wsym[ 7] =  ODDSYM  ;   wsym[ 8] =  PROCSYM     ;   wsym[ 9] =  THENSYM ;
    wsym[10] =  VARSYM  ;   wsym[11] =  WHILESYM    ;   wsym[12] =  WRITESYM;
    wsym[13] =  ELSESYM ;

    ssym['+'] = PLUS    ;   ssym['-'] = MINUS   ;   ssym['*'] = MULT        ;
    ssym['/'] = SLASH   ;   ssym['('] = LPAREN  ;   ssym[')'] = RPAREN      ;
    ssym['='] = EQL     ;   ssym[','] = COMMA   ;   ssym['.'] = PERIOD      ;
    ssym['#'] = NEQ     ;   ssym['<'] = LSS     ;   ssym['>'] = GTR         ;
    ssym['$'] = LEQ     ;   ssym['%'] = GEQ     ;   ssym[';'] = SEMICOLON   ;
    ssym['&'] = AMPER   ;   ssym['['] = LBRACK  ;   ssym[']'] = RBRACK      ;

    errmsg[ 0] = "No error";
    errmsg[ 1] = "Use = instead of :=";
    errmsg[ 2] = "= must be followed by a number.";
    errmsg[ 3] = "Identifier must be followed by =";
    errmsg[ 4] = "const, var, procedure must be followed by an identifier.";
    errmsg[ 5] = "Semicolon or comma missing.";
    errmsg[ 6] = "Incorrect symbol after procedure declaration.";
    errmsg[ 7] = "Statement Expected.";
    errmsg[ 8] = "Incorrect symbol after procedure declaration.";
    errmsg[ 9] = "Period Expected.";
    errmsg[10] = "Semicolon between statements is missing.";
    errmsg[11] = "Undeclared Identifier.";
    errmsg[12] = "Assignment to constant or procedure not allowed.";
    errmsg[13] = "Assignment operator := expected.";
    errmsg[14] = "call must be followed by and identifier.";
    errmsg[15] = "Call of constant or variable is meaningless.";
    errmsg[16] = "then expected.";
    errmsg[17] = "Semicolon or end expected.";
    errmsg[18] = "do expected.";
    errmsg[19] = "Incorrect symbol following statement.";
    errmsg[20] = "Relational operator expected.";
    errmsg[21] = "Expression must not contain a procedure identifier.";
    errmsg[22] = "Right parenthesis missing";
    errmsg[23] = "The preceding factor cannot be followed by this symbol.";
    errmsg[24] = "An expression cannot begin with this symbol.";
    errmsg[25] = "Array size missing.";
    errmsg[26] = "Left bracket expected.";
    errmsg[27] = "Right bracket expected.";
    errmsg[30] = "This number is too large.";

    printf("input source file name ");
    scanf("%s",fname);

    if(f=fopen(fname,"r"))
    {
         ch=' '; cx=0;

        gen(INC, 0, 2);
        getsym();
        block(0,0);
        getchar();
        getchar();

        // Print object code
        for (i=0; i<=cx-1; i++)
        {
            if (i<10) printf(" ");
            printf("%d ",i);
            switch (code[i].f)
            {
                case LIT: printf("lit "); break;
                case OPR: printf("opr "); break;
                case LOD: printf("lod "); break;
                case STO: printf("sto "); break;
                case CAL: printf("cal "); break;
                case INC: printf("inc "); break;
                case JMP: printf("jmp "); break;
                case JPC: printf("jpc "); break;
                case WRT: printf("wrt "); break;
                case STP: printf("stp "); break;
                case LID: printf("lid "); break;
                case LDA: printf("lda "); break;
                case SID: printf("sid "); break;
            }
            printf("%d ", code[i].l);
            printf("%d ", code[i].a);
            printf("\n");
        }
        if (sym!=PERIOD) error(9);
        getchar();
        interpret();
        getchar();
    }
    else
    {
        printf("Unable to open file '%s'.\n", fname);
        result = EXIT_FAILURE;
    }

    return result;
}
