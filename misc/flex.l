%{
  #include "bison.tab.hpp"
  #include<stdlib.h>
  #include<stdio.h>
  #include<string.h>
%}
%option noyywrap
%option yylineno

%%

"#".* {} /*komentar*/

"$" { return DOLAR;}
","  { return COMMA;}
"%"  { return PERCENTAGE;}
"+" { return PLUS;}
":" { return COLON;}
"[" { return LBRACKET;}
"]" { return RBRACKET;}
"-" { return MINUS;}
"*" { return MULTIPLY;}


"\.global" { return GLOBAL;}
\.extern { return EXTERN;}
\.section { return SECTION;}
\.word { return WORD;}
\.skip { return SKIP;}
\.ascii { return ASCII;}
\.equ { return EQU;}
\.end { return END;}
halt { return HALT;}
int { return INT;}
iret { return IRET;}
call { return CALL;}
ret { return RET;}
jmp { return JMP;}
beq { return BEQ;}
bne { return BNE;}
bgt { return BGT;}
push { return PUSH;}
pop { return POP;}
xchg { return XCHG;}
add { return ADD;}
sub { return SUB;}
mul { return MUL;}
div { return DIV;}
not { return NOT;}
and { return AND;}
or { return OR;}
xor { return XOR;}
shl { return SHL;}
ld { return LD;}
st { return ST;}
csrwr { return CSRWR;}
csrrd { return CSRRD;}
r0 { return R0;}
r1 { return R1;}
r2 { return R2;}
r3 { return R3;}
r4 { return R4;}
r5 { return R5;}
r6 { return R6;}
r7 { return R7;}
r8 { return R8;}
r9 { return R9;}
r10 { return R10;}
r11 { return R11;}
r12 { return R12;}
r13 { return R13;}
r14|sp { return R14;}
r15|pc { return R15;}
status { return STATUS;}
handler { return HANDLER;}
cause { return CAUSE;}

\" {return QUOTATION;}
\b 	{ }
\t 	{ }
\r { }
\f { }
\n {printf("\n");return EOL;}
0x[0-9a-fA-F]+  {  return LITERAL; }
[0-9]+          {  return NUMBER; }
([a-zA-Z])[a-zA-Z0-9_]* 	{  return SYMBOL;}

. { }
%%