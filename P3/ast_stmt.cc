/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"

Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    printf("\n");
}

Type* Program::Check() {
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */
    //PrintChildren(0);
    Symtab *S = new Symtab();
    S->enterScope();
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->Add(S);
    }
    for (int i = 0; i < decls->NumElements(); i++) {
        S->enterScope();
        decls->Nth(i)->Check(S);
        S->exitScope();
    }
    S->exitScope();
    return NULL;
}

Type* Stmt::Check(Symtab *S) {
    if (StmtBlock* s = dynamic_cast<StmtBlock*>(this))
        s->Check(S);
    else if (DeclStmt* d = dynamic_cast<DeclStmt*>(this))
        d->Check(S);
    else if (SwitchLabel *sw = dynamic_cast<SwitchLabel*>(this))
        sw->Check(S);
    else if (BreakStmt *b = dynamic_cast<BreakStmt*>(this))
        b->Check(S);
    else if (ContinueStmt *co = dynamic_cast<ContinueStmt*>(this))
        co->Check(S);
    else if (ReturnStmt *r = dynamic_cast<ReturnStmt*>(this))
        r->Check(S);
    else if (ConditionalStmt *c = dynamic_cast<ConditionalStmt*>(this))
        c->Check(S);
    else if (SwitchStmt *ss = dynamic_cast<SwitchStmt*>(this))
        ss->Check(S);
    else if (Expr* e = dynamic_cast<Expr*>(this))
        e->Check(S);
    return NULL;
}
StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

void StmtBlock::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    stmts->PrintAll(indentLevel+1);
}

Type* StmtBlock::Check(Symtab *S) {
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->Add(S);
    }
    for (int i = 0; i < decls->NumElements(); i++) {
        S->enterScope();
        decls->Nth(i)->Check(S);
        S->exitScope();
    }
    for (int i = 0; i < stmts->NumElements(); i++) {
        stmts->Nth(i)->Check(S);
    }
    return NULL;
}
DeclStmt::DeclStmt(Decl *d) {
    Assert(d != NULL);
    (decl=d)->SetParent(this);
}

Type* DeclStmt::Check(Symtab *S) {
    decl->Add(S);
    S->enterScope();
    decl->Check(S);
    S->exitScope();
    return NULL;
}


void DeclStmt::PrintChildren(int indentLevel) {
    decl->Print(indentLevel+1);
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
}

Type* ConditionalStmt::Check(Symtab *S) {
    if (LoopStmt* l = dynamic_cast<LoopStmt*>(this))
        l->Check(S);
    else if (IfStmt* i = dynamic_cast<IfStmt*>(this))
        i->Check(S);
    return NULL;
}

Type* LoopStmt::Check(Symtab *S) {
    if (ForStmt* f = dynamic_cast<ForStmt*>(this))
        f->Check(S);
    else if (WhileStmt* w = dynamic_cast<WhileStmt*>(this))
        w->Check(S);
    return NULL;
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && b != NULL);
    (init=i)->SetParent(this);
    step = s;
    if ( s )
      (step=s)->SetParent(this);
}

void ForStmt::PrintChildren(int indentLevel) {
    init->Print(indentLevel+1, "(init) ");
    test->Print(indentLevel+1, "(test) ");
    if ( step )
      step->Print(indentLevel+1, "(step) ");
    body->Print(indentLevel+1, "(body) ");
}

Type* ForStmt::Check(Symtab *S) {
    Node::loopCount++;
    S->enterScope();
    init->Check(S);
    if (!(test->Check(S)->IsEquivalentTo(Type::boolType)))
        ReportError::TestNotBoolean(test);
    if (step != NULL)
        step->Check(S);
    body->Check(S);
    S->exitScope();
    Node::loopCount--;
    return NULL;
}

void WhileStmt::PrintChildren(int indentLevel) {
    test->Print(indentLevel+1, "(test) ");
    body->Print(indentLevel+1, "(body) ");
}

Type* WhileStmt::Check(Symtab *S) {
    Node::loopCount++;
    S->enterScope();
    if (!(test->Check(S)->IsEquivalentTo(Type::boolType)))
        ReportError::TestNotBoolean(test);
    body->Check(S);
    S->exitScope();
    Node::loopCount--;
    return NULL;
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

void IfStmt::PrintChildren(int indentLevel) {
    if (test) test->Print(indentLevel+1, "(test) ");
    if (body) body->Print(indentLevel+1, "(then) ");
    if (elseBody) elseBody->Print(indentLevel+1, "(else) ");
}

Type* IfStmt::Check(Symtab *S) {
    if (!(test->Check(S)->IsEquivalentTo(Type::boolType)))
        ReportError::TestNotBoolean(test);
    S->enterScope();
    body->Check(S);
    S->exitScope();
    S->enterScope();
    if (elseBody != NULL)
        elseBody->Check(S);
    S->exitScope();
    S->exitScope();
    return NULL;
}
Type* BreakStmt::Check(Symtab *S) {
    if (Node::loopCount <= 0 && Node::swCount <= 0)
        ReportError::BreakOutsideLoop(this);
    return NULL;   
}

Type* ContinueStmt::Check(Symtab *S) {
    if (Node::loopCount <= 0)
        ReportError::ContinueOutsideLoop(this);
    return NULL;
}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    expr = e;
    if (e != NULL) expr->SetParent(this);
}

void ReturnStmt::PrintChildren(int indentLevel) {
    if ( expr ) 
      expr->Print(indentLevel+1);
}

Type* ReturnStmt::Check(Symtab *S) {
    Type * t = expr->Check(S);
    if (!(t->IsEquivalentTo(Node::CurFunc->getReturnType())))
        ReportError::ReturnMismatch(this,t,Node::CurFunc->getReturnType());
    return NULL;
}
SwitchLabel::SwitchLabel(Expr *l, Stmt *s) {
    Assert(l != NULL && s != NULL);
    (label=l)->SetParent(this);
    (stmt=s)->SetParent(this);
}

SwitchLabel::SwitchLabel(Stmt *s) {
    Assert(s != NULL);
    label = NULL;
    (stmt=s)->SetParent(this);
}

void SwitchLabel::PrintChildren(int indentLevel) {
    if (label) label->Print(indentLevel+1);
    if (stmt)  stmt->Print(indentLevel+1);
}

Type* SwitchLabel::Check(Symtab *S) {
    if (Case* ca = dynamic_cast<Case*>(this))
        ca->Check(S);
    else if (Default* de = dynamic_cast<Default*>(this))
        de->Check(S);
    return NULL;
}

Type* Case::Check(Symtab *S) {
    label->Check(S);
    stmt->Check(S);
    return NULL;
}

Type* Default::Check(Symtab *S) {
    stmt->Check(S);
    return NULL;
}
SwitchStmt::SwitchStmt(Expr *e, List<Stmt *> *c, Default *d) {
    Assert(e != NULL && c != NULL && c->NumElements() != 0 );
    (expr=e)->SetParent(this);
    (cases=c)->SetParentAll(this);
    def = d;
    if (def) def->SetParent(this);
}

void SwitchStmt::PrintChildren(int indentLevel) {
    if (expr) expr->Print(indentLevel+1);
    if (cases) cases->PrintAll(indentLevel+1);
    if (def) def->Print(indentLevel+1);
}

Type* SwitchStmt::Check(Symtab *S) {
    Node::swCount++;
    S->enterScope();
    expr->Check(S);
    for (int i = 0; i < cases->NumElements(); i++) {
        cases->Nth(i)->Check(S);
    }
    if (def != NULL)
        def->Check(S);
    S->exitScope();
    Node::swCount--;
    return NULL;
}

