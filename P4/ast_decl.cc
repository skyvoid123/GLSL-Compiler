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
    else if (FnDecl* f = dynamic_cast<FnDecl*>(this)) {
        f->Emit();
    }
    return NULL;
}

llvm::Value* VarDecl::Emit() {
    if (DEBUG)
        cout << "VarDecl" << endl;
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
        llvm::BasicBlock* bb = Node::irgen->GetBasicBlock();
        llvm::Value *val =  new llvm::AllocaInst(t, tw, bb);
        c.decl = this;
        c.val = val;
        c.flag = LOCAL;
    }
    Node::S->insert(make_pair(getId(), c));
    return NULL;
}

llvm::Value* FnDecl::Emit() {
    if (DEBUG)
        cout << "FnDecl" << endl;
    Node::S->enterScope();
    llvm::Module *mod = Node::irgen->GetOrCreateModule("mod");
    string name = getId();
    llvm::Type* retType = returnType->convert();
    vector<llvm::Type *> argTypes;
    for (int i = 0; i < formals->NumElements(); i++) {
        argTypes.push_back(formals->Nth(i)->getType()->convert());
    }
    llvm::ArrayRef<llvm::Type *> argArray(argTypes);
    llvm::FunctionType *funcTy = llvm::FunctionType::get(retType, argArray, false);
    llvm::Function *f = llvm::cast<llvm::Function>(mod->getOrInsertFunction(name, funcTy));
    Node::irgen->SetFunction(f);
    llvm::LLVMContext *context = Node::irgen->GetContext();
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, name, f);
    Node::irgen->SetBasicBlock(bb);
    int i = 0;
    for (llvm::Function::arg_iterator arg = f->arg_begin(); 
         arg != f->arg_end(); arg++, i++) {
        formals->Nth(i)->Emit();
        string varname = formals->Nth(i)->getId();
        arg->setName(varname);
        container c = Node::S->find(varname);
        llvm::Value *v = &*arg;
        new llvm::StoreInst(v, c.val, bb);
    }
    body->Emit();
    Node::S->exitScope();
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


