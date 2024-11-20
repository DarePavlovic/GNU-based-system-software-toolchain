%{ 

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include<stdio.h>
using namespace std;

#include "../inc/assembler.hpp"

FILE* filePointer;
extern  FILE* yyin;
int yylex();
extern int yyparse();
extern void yyerror(char const*msg);

Assembler* a;
vector<string> sym_list;
vector<symLitNum> sln_list;

%}

%union {
    int ival;
    char* sval;
}

%token <ival> NUMBER
%token <ival> LITERAL
%token <sval> SYMBOL

%start input;

%%



%token EOL DOLAR COMMA PERCENTAGE PLUS COLON LBRACKET RBRACKET;
%token GLOBAL EXTERN SECTION WORD SKIP ASCII EQU END ;
%token QUOTATION MINUS MULTIPLY;
%token HALT INT IRET CALL RET JMP BEQ BNE BGT PUSH POP XCHG ADD SUB MUL DIV;
%token AND OR XOR SHL SHR NOT LD ST CSRRD CSRWR;
%token R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11 R12 R13 R14 R15;
%token STATUS HANDLER CAUSE;

input:
|
 line input
;

line:
  EOL
|
 label EOL
| label_dir EOL
| instruction EOL
;


label: 
  SYMBOL COLON instruction {a->addLabel($1);}
|
  SYMBOL COLON  {a->addLabel($1);}
; 

label_dir: 
  GLOBAL listOfSymbols{a->setGlobal(sym_list);}
|
  EXTERN listOfSymbols{a->setExtern(sym_list);}
|
  SECTION SYMBOL {a->addSection($2);}
|
  WORD listOfSymbolsOrLiterals
|
  SKIP exprNL
|
  ASCII asciiString 
|
  EQU SYMBOL COMMA exp
|
  END
;

listOfSymbols: 
  listOfSymbols COMMA SYMBOL {sym_list.push_back($3);}
  |
  listOfSymbols COMMA csr 
  |
  csr
  |
  SYMBOL  {sym_list.push_back($1);}
;

listOfSymbolsOrLiterals:
  listOfSymbolsOrLiterals COMMA SYMBOL{symLitNum s;s.sym=$3;s.num=0; s.lit="";sln_list.push_back(s);}
|
  listOfSymbolsOrLiterals COMMA exprNL
|
  SYMBOL {symLitNum s;s.sym=$1;s.num=0; s.lit="";sln_list.push_back(s);}
|
  exprNL
;

asciiString: QUOTATION SYMBOL QUOTATION
;

exp:
  exprNL MULTIPLY exprNL
|
  exprNL PLUS exprNL
|
  exprNL MINUS exprNL;

exprNL:
  NUMBER {symLitNum s;s.sym="";s.num=$1; s.lit="";sln_list.push_back(s);}
|
LITERAL {symLitNum s;s.sym="";s.num=0; s.lit=$1;sln_list.push_back(s);}
;


instruction:
  HALT 
|
  INT 
|
  IRET 
|
  CALL jmpOperand
|
  RET
|
  JMP jmpOperand
|
  BEQ PERCENTAGE reg COMMA PERCENTAGE reg COMMA jmpOperand
|
  BNE PERCENTAGE reg COMMA PERCENTAGE reg COMMA jmpOperand
|
  BGT PERCENTAGE reg COMMA PERCENTAGE reg COMMA jmpOperand
| 
  PUSH PERCENTAGE reg
|
  POP PERCENTAGE reg
|
  XCHG PERCENTAGE reg COMMA PERCENTAGE reg
|
  ADD PERCENTAGE reg COMMA PERCENTAGE reg
|
  SUB PERCENTAGE reg COMMA PERCENTAGE reg
|
  MUL PERCENTAGE reg COMMA PERCENTAGE reg
|
  DIV PERCENTAGE reg COMMA PERCENTAGE reg
|
  NOT PERCENTAGE reg
|
  AND PERCENTAGE reg COMMA PERCENTAGE reg
|
  OR PERCENTAGE reg COMMA PERCENTAGE reg
|
  XOR PERCENTAGE reg COMMA PERCENTAGE reg
|
  SHL PERCENTAGE reg COMMA PERCENTAGE reg
|
  SHR PERCENTAGE reg COMMA PERCENTAGE reg
|
  LD operand COMMA PERCENTAGE reg
|
  ST PERCENTAGE reg COMMA operand
|
  CSRRD PERCENTAGE csr COMMA PERCENTAGE reg
|
  CSRWR PERCENTAGE reg COMMA PERCENTAGE csr
;

reg:  
  R0 
| R1
| R2
| R3
| R4
| R5
| R6
| R7
| R8
| R9
| R10
| R11
| R12
| R13
| R14
| R15
;

csr: 
  STATUS
| HANDLER
| CAUSE;

operand:
  DOLAR exprNL
|
  DOLAR csr
|
  DOLAR SYMBOL
|
  LITERAL
|
  SYMBOL
|
  PERCENTAGE reg
|
  LBRACKET PERCENTAGE reg RBRACKET
|
  LBRACKET PERCENTAGE reg PLUS LITERAL RBRACKET
|
  LBRACKET PERCENTAGE reg PLUS SYMBOL RBRACKET
;

jmpOperand:
  exprNL
|
  SYMBOL;

  %%

  int parser_main(char* fileName){
    filePointer = fopen(fileName, "r");
    if(filePointer==0){
      return 1;
    }
    yyin = filePointer;
    printf("Otvorio sam fajl %s\n", fileName);
    while(yyparse()!=1){

    }


    fclose(filePointer);
    return 0;


  }
  void yyerror(char const* msg){
    printf("%s\n",msg);
    
  }