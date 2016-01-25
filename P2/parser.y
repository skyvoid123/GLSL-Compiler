/* File: parser.y
 * --------------
 * Bison input file to generate the parser for the compiler.
 *
 * pp2: your job is to write a parser that will construct the parse tree
 *      and if no parse errors were found, print it.  The parser should 
 *      accept the language as described in specification, and as augmented 
 *      in the pp2 handout.
 */

%{

/* Just like lex, the text within this first region delimited by %{ and %}
 * is assumed to be C/C++ code and will be copied verbatim to the y.tab.c
 * file ahead of the definitions of the yyparse() function. Add other header
 * file inclusions or C++ variable declarations/prototypes that are needed
 * by your code here.
 */
#include "scanner.h" // for yylex
#include "parser.h"
#include "errors.h"

void yyerror(const char *msg); // standard error-handling routine

%}

/* The section before the first %% is the Definitions section of the yacc
 * input file. Here is where you declare tokens and types, add precedence
 * and associativity options, and so on.
 */
 
/* yylval 
 * ------
 * Here we define the type of the yylval global variable that is used by
 * the scanner to store attibute information about the token just scanned
 * and thus communicate that information to the parser. 
 *
 * pp2: You will need to add new fields to this union as you add different 
 *      attributes to your non-terminal symbols.
 */
%union {
    int integerConstant;
    bool boolConstant;
    char *stringConstant;
    float floatConstant;
    char identifier[MaxIdentLen+1]; // +1 for terminating null
    Decl *decl;
    List<Decl*> *declList;
}


/* Tokens
 * ------
 * Here we tell yacc about all the token types that we are using.
 * Bison will assign unique numbers to these and export the #define
 * in the generated y.tab.h header file.
 */
%token   T_Void T_Bool T_Int T_Float 
%token   T_LessEqual T_GreaterEqual T_Equal T_NotEqual T_Dims
%token   T_And T_Or 
%token   T_While T_For T_If T_Else T_Return T_Break
%token   T_Inc T_Dec T_Switch T_Case T_Default
%token   T_Const T_Uniform T_Layout T_Continue T_Do T_In T_Out T_InOut
%token   T_Mat2 T_Mat3 T_Mat4 T_Vec2 T_Vec3 T_Vec4 
%token   T_IVec2 T_IVec3 T_IVec4 T_BVec2 T_BVec3 T_BVec4 
%token   T_UVec2 T_UVec3 T_UVec4 T_UInt T_Struct

%token   <identifier> T_Identifier
%token   <integerConstant> T_IntConstant
%token   <floatConstant> T_FloatConstant
%token   <boolConstant> T_BoolConstant


/* Non-terminal types
 * ------------------
 * In order for yacc to assign/access the correct field of $$, $1, we
 * must to declare which field is appropriate for the non-terminal.
 * As an example, this first type declaration establishes that the DeclList
 * non-terminal uses the field named "declList" in the yylval union. This
 * means that when we are setting $$ for a reduction for DeclList ore reading
 * $n which corresponds to a DeclList nonterminal we are accessing the field
 * of the union named "declList" which is of type List<Decl*>.
 * pp2: You'll need to add many of these of your own.
 */
%type <declList>  DeclList 
%type <decl>      Decl

%%
/* Rules
 * -----
 * All productions and actions should be placed between the start and stop
 * %% markers which delimit the Rules section.
	 
 */
Program   :    DeclList            { 
                                      @1; 
                                      /* pp2: The @1 is needed to convince 
                                       * yacc to set up yylloc. You can remove 
                                       * it once you have other uses of @n*/
                                      Program *program = new Program($1);
                                      // if no errors, advance to next phase
                                      if (ReportError::NumErrors() == 0) 
                                          program->Print(0);
                                    }
          ;

v_ident   :    T_Identfier	    { }

p_expr	  :    v_ident		    { }
	  |    T_IntConstant	    { }
	  |    T_UInt		    { }
	  |    T_FloatConstant	    { }
	  |    T_BoolConstant	    { }
	  |    '(' expr ')'   { }
	  ;

pfix_expr :    prim_expr	    { }
	  |    pfix_expr '(' int_expr ')' { }
	  |    func_call	    { }
	  |    pfix_expr '.' T_FieldSelection { }
	  |    pfix_expr T_Inc	    { }
	  |    pfix_expr T_Dec	    { }
	  ;

int_expr  :    expr		    { }
	  ;

func_call :    func_call_or_method  { }
	  ;

func_call_or_method : 	func_call_generic { }
	  ;

func_call_generic :	func_call_hdr_w_param ')' { }
	  |		func_call_hdr_no_param ')'{ }
	  ;

func_call_hdr_no_param : func_call_hdr T_Void { }
	  |		 func_call_hdr	      { }
	  ;

func_call_hdr_w_param :	func_call_hdr assign_expr { }
	  |		func_call_hdr_w_param ',' assign_expr { }
	  ;

func_call_hdr: func_ident '(' 	    { }
	  ;

func_ident:    type_spec	    { }
	  |    pfix_expr	    { }
	  ;

unary_expr:    pfix_expr	    { }
	  |    T_Inc unary_expr     { }
	  |    T_Dec unary_expr	    { }
	  |    unary_op unary_expr  { }
	  ;

unary_op  :    '+'		    { }
	  |    '-'		    { }
	  ;

multi_expr:    unary_expr	    { }
	  |    multi_expr '*' unary_expr { }
	  |    multi_expr '/' unary_expr { }
	  ;

add_expr  :    multi_expr	    { }
	  |    add_expr '+' multi_expr { }
	  |    add_expr '-' multi_expr { }
	  ;

shift_expr:    add_expr		    { }
	  ;

rel_expr  :    shift_expr	    { }
	  |    rel_expr '<' shift_expr { }
	  |    rel_expr '>' shift_expr { }
	  |    rel_expr T_LessEqual shift_expr    { }
	  |    rel_expr T_GreaterEqual shift_expr { }
	  ;

equal_expr:    rel_expr		    { }
	  |    equal_expr T_Equal rel_expr { }
	  |    equal_expr T_NotEqual rel_expr { }
	  ;

and_expr  :    equal_expr	    { }
	  ;

excl_or_expr:  and_expr		    { }
	  ;

incl_or_expr:  excl_or_expr	    { }
	  ;

logic_and_expr:incl_and_expr { }
	      |logic_and_expr T_And incl_or_expr { }
	      ;

logic_xor_expr:logic_and_expr { }
              ;

logic_or_expr:logic_xor_expr { }
             |logic_or_expression T_Or logic_xor_expr { }
             ;

cond_expr: logic_or_expr { }
	 ;

assign_expr:   cond_expr { }
	   |   unary_expr assign_op assign_expr { }
	   ;

assign_op :    '=' 		    { }
	  |    '*='		    { }
	  |    '/='		    { }
	  |    '+='		    { }
	  |    '-='		    { }
	  ;

expr	  :    assign_expr	    { }
	  |    expr ',' assign_expr { }
          ;

const_expr:    cond_expr	    { }
	  ;

decl	  :    func_proto ';'	    { }
	  |    init_declare_list ';'{ }
	  |    type_qual T_Identifier '{' struct_declare_list '}' ';' { }
	  |    type_qual T_Identifier '{' struct_declare_list '}' 
			T_Identifier ';' { }
	  |    type_qual T_Identifier '{' struct_declare_list '}'
			 T_Identifier array_spec ';' { }
	  |    type_qual ';' 	    { }
	  |    type_qual T_Identifier ';' { }
	  |    type_qual T_Identifier id_list ';' { }
	  ;

id_list   :    ',' T_Identifier     { }
          |    id_list ',' T_Identifier { }
	  ;

func_proto:    func_declarator '('     { }
	  ;

func_declare:  func_hdr		    { }
	  |    func_hdr_w_param     { }
	  ;

func_hdr_w_param: func_hdr_param_declaration { }
	  |    func_hdr_w_param ',' param_declaration { }
	  ;

func_hdr  :    fully_spec_type T_Identifier '(' { }
	  ;

param_declarator: type_spec T_Identifier { }
	  |    type_spec T_Identifer arr_spec { }

param_declaration: type_qual param_declarator { }
	  |    param_declarator     { }
	  |    type_qual param_type_spec { }
	  |    param_type_spec	    { }
	  ;

param_type_spec: type_spec          { }
	  ;

init_declare_list: single_declare   { }
	  |    init_declare_list ',' T_Identifier { }
	  |    init_declare_list ',' T_Identifier array_spec { }
	  |    init_declare_list ',' T_Identifier array_spec '=' init { }
	  |    init_declare_list ',' T_Identifier '=' init { }
	  ;

single_declaration: fully_spec_type { }
	  |    fully_spec_type T_Identifier { }
	  |    fully_spec_type T_Identifier array_spec { }
	  |    fully_spec_type T_Identifier array_spec '=' init { }
	  |    fully_spec_type T_Identifier '=' init { }
	  ;

fully_spec_type: type_spec          { }
	  |    type_qual type_spec  { }
	  ;

layout_qual:   T_Layout '(' layout_qual_id_list ')' { }
	  ;

layout_qual_id_list: layout_qual_id { }
	  |    layout_qual_id_list ',' layout_qual_id { }
          ;

layout_qual_id: T_Identifier	    { }
	  |    T_Identifier '=' T_IntConstant { }
	  |    T_Identifier '=' T_UInt { }
	  ;

type_qual :    single_type_qual     { }
	  |    type_qual single_type_qual { }
	  ;

single_type_qual: storage_qual	    { }
	  |    layout_qual	    { }
	  |    interpol_qual   	    { }
	  |    inv_qual	            { }
          ;

storage_qual:  T_Const	            { }
	  |    T_In	            { }
	  |    T_Out	  	    { }
	  |    T_InOut	 	    { }
	  |    T_Uniform	    { }
	  ;

type_spec :    type_spec_nonarray   { }
	  |    type_spec_nonarray array_spec { }
          ;

array_spec:    '{' '}'	 	    { }
	  |    '{' const_expr '}'   { }
	  |    array_spec '{' '}'   { }
	  |    array_spec '{' const_expr '}' { }
 	  ;

type_spec_nonarray: T_Void	    { }
	  |    T_Float		    { }
	  |    T_Int	            { }
	  |    T_UInt		    { }
	  |    T_Bool		    { }
	  |    T_Vec2		    { }
	  |    T_Vec3		    { }
	  |    T_Vec4		    { }
	  |    T_BVec2		    { }
	  |    T_BVec3		    { }
	  |    T_BVec4	            { }
	  |    T_IVec2              { }
          |    T_IVec3              { }
          |    T_IVec4              { }
	  |    T_UVec2              { }
          |    T_UVec3              { }
          |    T_UVec4              { }
	  |    T_Mat2	            { }
	  |    T_Mat3		    { }
	  |    T_Mat4		    { }
	  ;

struct_spec:   T_Struct T_Identifier '{' struct_declaration_list '}' { }
	  |    T_Struct '{' struct_declaration_list '}' { }
	  ;

struct_declaration_list: struct_declaration { }
	  |    struct_declaration_list struct_declaration { }
	  ;

struct_declaration: type_spec struct_declarator_list ';' { }
          |    type_qual type_spec struct_declarator_list ';' { }
	  ;

struct_declarator_list: struct_declarator { }
	  |    struct_declarator_list ',' struct_declarator { }
	  ;

struct_declarator: T_Identifier     { }
	  |    T_Identifier array_spec { }
	  ;

init      :    assign_expr	    { }
	  ;

declaration_statement: declaration  { }
	  ;

statement :    comp_statement_w_scope { }
          |    simple_statement       { }
	  ;

statement_no_new_scope: comp_statement_no_new_scope { }
          |    simple_statement	    { }
	  ;

statement_w_scope: comp_statement_no_new_scope { }
	  |    simple_statement     { }
	  ;

simple_statement: declaration_statement { }
	  |    expr_statement       { }
	  |    select_statement     { }
	  |    switch_statement     { }
	  |    case_label	    { }
	  |    iter_statement	    { }
	  |    jump_statement	    { }
	  ;

comp_statement_w_scope: '{' '}'     { }
	  |    '{' statement_list '}' { }
	  ;

statement_list: statement           { }
          |    statement_list statement { }
	  ;

expr_statement: ';'	            { }
	  |    expr ';'	            { }
	  ;

select_statement: T_If '(' expr ')' select_rest_statement { }
	  ;

select_rest_statement: statement_w_scope T_Else statement_w_scope { }
	  |    statement_w_scope    { }
	  ;

cond      :    expr	 	    { }
	  |    fully_spec_type T_Identifier '=' init { }
	  ;

switch_statement: T_Switch '(' expr ')' '{' switch_statement_list '}' { }
	  ;

switch_statement_list: { }
	  | statement_list { }
	  ;

case_label:    T_Case expr ':'      { }
	  |    T_Default ':'        { }
	  ;

iter_statement: T_While '(' cond ')' statement_no_new_scope { }
	  |    T_Do statement_with_scope T_While '(' expr ')' ';' { }
	  |    T_For '(' for_init_statement for_rest_statement ')'
			statement_no_new_scope { }
	  ;

for_init_statement: expr_statement  { }
	  |    declaration_statement { }
	  ;

condopt   :    cond	            { }
	  |    	                    { }
	  ;

for_rest_statement: condopt ';'     { }
	  |    condopt ';' expr	    { }
	  ;

jump_statement: T_Continue ';'	    { }
	  |    T_Break ';'	    { }
	  |    T_Return ';'	    { }
	  |    T_Return expr ';'    { }
          ;

trans_unit:    ext_declaration      { }
	  |    trans_unit ext_declaration { }

ext_declaration: func_def           { }
          |    declaration          { }
	  ;

func_def  :    func_proto comp_statement_no_new_scope { }
          ;

DeclList  :    DeclList Decl        { ($$=$1)->Append($2); }
          |    Decl                 { ($$ = new List<Decl*>)->Append($1); }
          ;

Decl      :    T_Void               { $$ = new VarDecl(); /* pp2: test only. Replace with correct rules  */ } 
          ;
          


%%

/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated y.tab.c file.
 * This section is where you put definitions of helper functions.
 */

/* Function: InitParser
 * --------------------
 * This function will be called before any calls to yyparse().  It is designed
 * to give you an opportunity to do anything that must be done to initialize
 * the parser (set global variables, configure starting state, etc.). One
 * thing it already does for you is assign the value of the global variable
 * yydebug that controls whether yacc prints debugging information about
 * parser actions (shift/reduce) and contents of state stack during parser.
 * If set to false, no information is printed. Setting it to true will give
 * you a running trail that might be helpful when debugging your parser.
 * Please be sure the variable is set to false when submitting your final
 * version.
 */
void InitParser()
{
   PrintDebug("parser", "Initializing parser");
   yydebug = false;
}
