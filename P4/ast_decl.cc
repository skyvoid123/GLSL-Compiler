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

llvm::Value* Decl::Emit() {
    if (VarDecl* v = dynamic_cast<VarDecl*>(this))
        v->Emit();
    else if (FnDecl* f = dynamic_cast<FnDecl*>(this))
        f->Emit();
    return NULL;
}

llvm::Value* VarDecl::Emit() {
    llvm::Module *mod = irgen->GetOrCreateModule("mod");
    const llvm::Twine tw(getId());
    llvm::Type* t = getType()->convert();
    container c;
    if (Node::S->getLevelNumber() == 1) {
        llvm::Value* val = new llvm::GlobalVariable(
            *mod,
            t, 
            false, 
            llvm::GlobalValue::ExternalLinkage, 
            llvm::Constant::getNullValue(t), 
            tw);
        c.decl = this;
        c.val = val;
        c.flag = GLOBAL;
    }
    else {
        llvm::Value *val =  new llvm::AllocaInst(t, tw);
        c.decl = this;
        c.val = val;
        c.flag = LOCAL;
    }
    Node::S->insert(make_pair(getId(), c));
    container temp = Node::S->find(getId());
    return NULL;
}

llvm::Value* FnDecl::Emit() {
    llvm::Module *mod = Node::irgen->GetOrCreateModule("mod");
    string name = getId();
    llvm::Type* retType = returnType->convert();
    if (formals->NumElements() > 0) {
        vector<llvm::Type *> argTypes;
        for (int i = 0; i < formals->NumElements(); i++) {
            argTypes.push_back(formals->Nth(i)->getType()->convert());
        }
        llvm::ArrayRef<llvm::Type *> argArray(argTypes);
        llvm::FunctionType *funcTy = llvm::FunctionType::get(retType, argArray, false);
        llvm::Function *f = llvm::cast<llvm::Function>(mod->getOrInsertFunction(name, funcTy));
        int i = 0;
        for (llvm::Function::arg_iterator arg = f->arg_begin(); 
            arg != f->arg_end(); arg++, i++) {
            arg->setName(formals->Nth(i)->getId());
        }
    }
    else {
        llvm::FunctionType *funcTy = llvm::FunctionType::get(retType, false);
        mod->getOrInsertFunction(name, funcTy);
    }
    return NULL;
}
VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
}
  
void VarDecl::PrintChildren(int indentLevel) { 
   if (type) type->Print(indentLevel+1);
   if (id) id->Print(indentLevel+1);
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


