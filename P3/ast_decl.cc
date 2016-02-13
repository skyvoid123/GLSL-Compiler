/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
        
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
}


VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
}
  
void VarDecl::PrintChildren(int indentLevel) { 
   if (type) type->Print(indentLevel+1);
   if (id) id->Print(indentLevel+1);
}

void VarDecl::Check(Symtab *S) {
    
}

void FnDecl::Check(Symtab *S) {
    S->enterScope();
    for (int i = 0 ; i < formals->NumElements(); i++) {
        VarDecl *v = formals->Nth(i);
        S->insert(make_pair(v->getId(), v->getType()));
    }
    for (int i = 0; i < formals->NumElements(); i++) {
        formals->Nth(i)->Check(S);
    }
    if (StmtBlock *s = dynamic_cast<StmtBlock*>(body)) {
        s->Check(S);
    }
    S->exitScope();
}
FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}

void FnDecl::PrintChildren(int indentLevel) {
    if (returnType) returnType->Print(indentLevel+1, "(return type) ");
    if (id) id->Print(indentLevel+1);
    if (formals) formals->PrintAll(indentLevel+1, "(formals) ");
    if (body) body->Print(indentLevel+1, "(body) ");
}


