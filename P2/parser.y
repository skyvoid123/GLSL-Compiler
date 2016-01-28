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
#include <utility>

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
    float floatConstant;
    char identifier[MaxIdentLen+1]; // +1 for terminating null
    Decl *decl;
    VarDecl *varD;
    VarDeclError *varDE;
    FnDecl *fnD;
    FormalsError *formalsE;
    List<Decl*> *declList;
    Stmt *stmt;
    StmtBlock *stmtB;
    SwitchLabel *switchL;
    Default *def;
    Case *c;
    BreakStmt *breakS;
    ConditionalStmt *conditionalS;
    LoopStmt *loopS;
    ForStmt *forS;
    WhileStmt *whileS;
    IfStmt *ifS;
    IfStmtExprError *ifSEE;
    SwitchStmt *switchS;
    SwitchStmtError *switchSE;
    Expr *expr;
    LValue *lV;
    FieldAccess *fieldA;
    ArrayAccess *arrayA;
    ExprError *exprE;
    EmptyExpr *emptyE;
    CompoundExpr *compoundE;
    PostfixExpr *postfixE;
    ArithmeticExpr *arithmeticE;
    RelationalExpr *relationalE;
    EqualityExpr *equalityE;
    LogicalExpr *logicalE;
    AssignExpr *assignE;
    ReturnStmt *returnS;
    Type *type;
    ArrayType *arrayT;
    NamedType *namedT;
    Operator *op;
    Error *error;
    Program *program;
    std::pair<Type*, Identifier*> *tup;
}


/* Tokens
 * ------
 * Here we tell yacc about all the token types that we are using.
 * Bison will assign unique numbers to these and export the #define
 * in the generated y.tab.h header file.
 */
%token   T_Void T_Bool T_Int T_Float 
%token   T_LessEqual T_GreaterEqual T_Equal T_NotEqual T_Dims
%token   T_And T_Or T_FieldSelection
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
%type <identifier>	  v_ident;
%type <expr>	  prim_expr;
%type <postfixE>	  pfix_expr;
%type <expr>  int_expr;
%type <identifier>	  func_ident;
%type <arithmeticE>          unary_expr;
%type <op>          unary_op;
%type <arithmeticE>          multi_expr;
%type <arithmeticE>          add_expr;
%type <expr>          shift_expr;
%type <relationalE>          rel_expr;
%type <equalityE>          equal_expr;
%type <logicalE>          and_expr;
%type <logicalE>          excl_or_expr;
%type <logicalE>          incl_or_expr;
%type <logicalE>          logic_and_expr;
%type <logicalE>          logic_xor_expr;
%type <logicalE>          logic_or_expr;
%type <logicalE>          cond_expr;
%type <assignE>          assign_expr;
%type <op>          assign_op;
%type <expr>          expr;
%type <expr>          const_expr;
%type <decl>          decl;
%type <fnD>          func_proto;
%type <fnD>          func_declarator;
%type <fnD>          func_hdr_w_param;
%type <fnD>          func_hdr;
%type <varD>          param_declarator;
%type <varD>          param_declaration;
%type <type>          param_type_spec;
%type <decl>          init_declarator_list;
%type <decl>          single_declaration;
%type <type>          fully_spec_type;
%type <type>          type_spec;
%type <type>          type_spec_nonarray;
%type <assignE>          init;
%type <decl>          declaration_statement;
%type <stmt>          statement;
%type <stmt>          simple_statement;
%type <stmt>          comp_statement;
%type <stmtB>          statement_list;
%type <expr>          expr_statement;
%type <ifS>          select_statement;
%type <ifS>          select_rest_statement;
%type <conditionalS>          cond;
%type <switchS>          switch_statement;
%type <switchS>          switch_statement_list;
%type <switchL>          case_label;
%type <loopS>          iter_statement;
%type <forS>          for_init_statement;
%type <conditionalS>          condopt;
%type <forS>          for_rest_statement;
%type <declList>          trans_unit;
%type <decl>          ext_declaration;
%type <fnD>          func_def;

%%
/* Rules
 * -----
 * All productions and actions should be placed between the start and stop
 * %% markers which delimit the Rules section.
	 
 */
Program                     :    trans_unit            { 
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

v_ident                     :    T_Identifier	    { }

prim_expr	                :    v_ident		    { }
	                        |    T_IntConstant	    { }
	                        |    T_FloatConstant	    { }
	                        |    T_BoolConstant	    { }
	                        |    '(' expr ')'   { }
	                        ;

pfix_expr                   :    prim_expr	    { }
	                        |    pfix_expr '.' T_FieldSelection { }
	                        |    pfix_expr T_Inc	    { }
	                        |    pfix_expr T_Dec	    { }
	                        ;
    
int_expr                    :    expr		    { }
	                        ;
    
func_ident                  :    type_spec	    { }
	                        |    pfix_expr	    { }
	                        ;
    
unary_expr                  :    pfix_expr	    { }
	                        |    T_Inc unary_expr     { }
	                        |    T_Dec unary_expr	    { }
	                        |    unary_op unary_expr  { }
	                        ;
    
unary_op                    :    '+'		    { }
	                        |    '-'		    { }
	                        ;
    
multi_expr                  :    unary_expr	    { }
	                        |    multi_expr '*' unary_expr { }
	                        |    multi_expr '/' unary_expr { }
	                        ;
    
add_expr                    :    multi_expr	    { }
	                        |    add_expr '+' multi_expr { }
	                        |    add_expr '-' multi_expr { }
	                        ;
    
shift_expr                  :    add_expr		    { }
	                        ;

rel_expr                    :    shift_expr	    { }
	                        |    rel_expr '<' shift_expr { }
	                        |    rel_expr '>' shift_expr { }
	                        |    rel_expr T_LessEqual shift_expr    { }
	                        |    rel_expr T_GreaterEqual shift_expr { }
	                        ;
        
equal_expr                  :    rel_expr		    { }
	                        |    equal_expr T_Equal rel_expr { }
	                        |    equal_expr T_NotEqual rel_expr { }
	                        ;

and_expr                    :    equal_expr	    { }
	                        ;

excl_or_expr                :    and_expr		    { }
	                        ;

incl_or_expr                :    excl_or_expr	    { }
	                        ;

logic_and_expr              :    incl_or_expr { }
	                        |    logic_and_expr T_And incl_or_expr { }
	                        ;
    
logic_xor_expr              :    logic_and_expr { }
                            ;

logic_or_expr               :    logic_xor_expr { }
                            |    logic_or_expr T_Or logic_xor_expr { }
                            ;

cond_expr                   :    logic_or_expr { }
	                        ;

assign_expr                 :    cond_expr { }
	                        |    unary_expr assign_op assign_expr { }
	                        ;

assign_op                   :    '=' 		    { }
	                        |    '*='		    { }
	                        |    '/='		    { }
	                        |    '+='		    { }
	                        |    '-='		    { }
	                        ;

expr	                    :    assign_expr	    { }
                            ;

const_expr                  :    cond_expr	    { }
	                        ;

decl	                    :    func_proto ';'	    { }
	                        |    init_declarator_list ';'{ }
	                        ;

func_proto                  :    func_declarator ')'     { }
	                        ;

func_declarator             :    func_hdr		    { }
	                        |    func_hdr_w_param     { }
	                        ;
    
func_hdr_w_param            :    func_hdr param_declaration {  }
	                        |    func_hdr_w_param ',' param_declaration { }
	                        ;

func_hdr                    :    fully_spec_type T_Identifier '(' { $$ = new FnDecl(new Identifier(@1, $2), $1, new List<VarDecl*>);}

param_declarator            :    type_spec T_Identifier {$$ = new VarDecl(new Identifier(@1, $2), $1); }
                            ;

param_declaration           :    param_declarator     { $$ = $1; }
	                        ;

param_type_spec             :    type_spec          { $$ = $1;}
	                        ;

init_declarator_list        :    single_declaration   { $$ = $1; }
	                        ;

single_declaration          :    fully_spec_type T_Identifier { $$ = new VarDecl(new Identifier(@1, $2), $1); }
	                        ;

fully_spec_type             :    type_spec          { $$ = $1; }
	                        ;

type_spec                   :    type_spec_nonarray   { $$ = $1; }
                            ;

type_spec_nonarray          :    T_Void	    { $$ = Type::voidType; }
                            |    T_Bool     { $$ = Type::boolType; }
	                        |    T_Float    { $$ = Type::floatType;}
	                        |    T_Int	    { $$ = Type::intType;  }
	                        |    T_Vec2		{ $$ = Type::vec2Type; }
	                        |    T_Vec3		{ $$ = Type::vec3Type; }
	                        |    T_Vec4		{ $$ = Type::vec4Type; }
	                        |    T_Mat2	    { $$ = Type::mat2Type; }
	                        |    T_Mat3		{ $$ = Type::mat3Type; }
	                        |    T_Mat4		{ $$ = Type::mat4Type; }
	                        ;

init                        :    assign_expr	    { }
	                        ;

declaration_statement       : decl  { }
	                        ;

statement                   :    simple_statement       { }
	                        ;

simple_statement            : declaration_statement { }
	                        |    expr_statement       { }
	                        |    select_statement     { }
	                        |    switch_statement     { }
	                        |    case_label	    { }
	                        |    iter_statement	    { }
	                        ;

comp_statement              : '{' '}'     {$$ = NULL; }
	                        |    '{' statement_list '}' { }
	                        ;

statement_list              : statement           { }
                            | statement_list statement { }
	                        ;

expr_statement              : ';'	            { }
	                        |    expr ';'	            { }
	                        ;

select_statement            : T_If '(' expr ')' select_rest_statement { }
	                        ;

select_rest_statement       : statement T_Else statement { }
	                        |    statement    { }
	                        ;

cond                        :    expr	 	    { }
	                        |    fully_spec_type T_Identifier '=' init { }
	                        ;

switch_statement            : T_Switch '(' expr ')' '{' switch_statement_list '}' { }
	                        ;

switch_statement_list       : { }
	                        | statement_list { }
	                        ;

case_label                  :    T_Case expr ':'      { }
	                        |    T_Default ':'        { }
	                        ;

iter_statement              : T_While '(' cond ')' statement { }
	                        |    T_For '(' for_init_statement for_rest_statement ')'
			                                statement { }
	                        ;

for_init_statement          : expr_statement  { }
	                        |    declaration_statement { }
	                        ;

condopt                     :    cond	            { }
	                        |    	                    { }
	                        ;

for_rest_statement          :    condopt ';'     { }
	                        |    condopt ';' expr	    { }
	                        ;

trans_unit                  :    ext_declaration      { ($$ = new List<Decl*>)->Append($1); }
	                        |    trans_unit ext_declaration { ($$=$1)->Append($2); }
                            ;

ext_declaration             :    func_def           { }
                            |    decl          { }
	                        ;

func_def                    :    func_proto comp_statement { if ($2 != NULL) $1->SetFunctionBody($2); }
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
