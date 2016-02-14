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

Type* Decl::Check(Symtab *S) {
    if (VarDecl* v = dynamic_cast<VarDecl*>(this))
        v->Check(S);
    else if (FnDecl* f = dynamic_cast<FnDecl*>(this))
        f->Check(S);
    return NULL;
}

void Decl::Add(Symtab *S) {
    if (VarDecl* v = dynamic_cast<VarDecl*>(this))
        S->insert(make_pair(v,v->getType()));
    else if (FnDecl* f = dynamic_cast<FnDecl*>(this))
        S->insert(make_pair(f, f->getReturnType()));
}
Type* VarDecl::Check(Symtab *S) {
  return NULL;
}

Type* FnDecl::Check(Symtab *S) {
    S->enterScope();
    for (int i = 0 ; i < formals->NumElements(); i++) {
        VarDecl *v = formals->Nth(i);
        S->insert(make_pair(v, v->getType()));
    }
    for (int i = 0; i < formals->NumElements(); i++) {
        formals->Nth(i)->Check(S);
    }
    body->Check(S);
    S->exitScope();
    return NULL;
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


