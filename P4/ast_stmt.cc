/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"

#include "irgen.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/raw_ostream.h"

Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    printf("\n");
}

llvm::Value* Program::Emit() {
    if (DEBUG) 
        Print(0);
    Node::S = new Symtab();
    // TODO:
    // This is just a reference for you to get started
    //
    // You can use this as a template and create Emit() function
    // for individual node to fill in the module structure and instructions.
    //
    llvm::Module *mod = Node::irgen->GetOrCreateModule("mod");
    Node::S->enterScope();
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->Emit();
    }
    S->exitScope();
    mod->dump();
    if (DEBUG)
        mod->dump();
    else
        llvm::WriteBitcodeToFile(mod, llvm::outs());
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

DeclStmt::DeclStmt(Decl *d) {
    Assert(d != NULL);
    (decl=d)->SetParent(this);
}

void DeclStmt::PrintChildren(int indentLevel) {
    decl->Print(indentLevel+1);
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
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

void WhileStmt::PrintChildren(int indentLevel) {
    test->Print(indentLevel+1, "(test) ");
    body->Print(indentLevel+1, "(body) ");
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


ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    expr = e;
    if (e != NULL) expr->SetParent(this);
}

void ReturnStmt::PrintChildren(int indentLevel) {
    if ( expr ) 
      expr->Print(indentLevel+1);
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

llvm::Value* StmtBlock::Emit() {
    if (DEBUG)
        cout << "StmtBlock" << endl;
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->Emit();
    }
    for (int i = 0; i < stmts->NumElements(); i++) {
        if (!Node::irgen->GetBasicBlock()->getTerminator())
            stmts->Nth(i)->Emit();
        else if ( DEBUG )
            cout << Node::irgen->GetBasicBlock()->getName().str() << endl;
    }
    return NULL;
}

llvm::Value* DeclStmt::Emit() {
    if (DEBUG)
        cout << "DeclStmt" << endl;
    decl->Emit();
    return NULL;
}

llvm::Value* ForStmt::Emit() {
    Node::S->enterScope();
    if (DEBUG)
        cout << "ForStmt" << endl;
    llvm::LLVMContext *context = Node::irgen->GetContext();
    llvm::Function *f = Node::irgen->GetFunction();
    llvm::BasicBlock *pbb = Node::breakB;
    llvm::BasicBlock *pcb = Node::continueB;
    init->Emit();
    llvm::BasicBlock *hb = Node::irgen->GetBasicBlock();
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "for body", f);
    llvm::BasicBlock *fb = llvm::BasicBlock::Create(*context, "for footer", f);
    llvm::BasicBlock *sb = llvm::BasicBlock::Create(*context, "for step", f);
    llvm::BranchInst::Create(bb, fb, test->Emit(), hb);
    Node::breakB = fb;
    Node::continueB = sb;
    bb->moveAfter(Node::irgen->GetBasicBlock());
    Node::irgen->SetBasicBlock(bb);
    body->Emit();
    if (!Node::irgen->GetBasicBlock()->getTerminator()) {
        llvm::BranchInst::Create(sb, Node::irgen->GetBasicBlock());
    }
    sb->moveAfter(Node::irgen->GetBasicBlock());
    Node::irgen->SetBasicBlock(sb);
    step->Emit();
    if (!Node::irgen->GetBasicBlock()->getTerminator()) {
        llvm::BranchInst::Create(bb, fb, test->Emit(), Node::irgen->GetBasicBlock());
    }
    fb->moveAfter(Node::irgen->GetBasicBlock());
    if (!Node::irgen->GetBasicBlock()->getTerminator()) {
        llvm::BranchInst::Create(fb, Node::irgen->GetBasicBlock());
    }
    Node::irgen->SetBasicBlock(fb);
    Node::S->exitScope();
    Node::breakB = pbb;
    Node::continueB = pcb;
    return NULL;
}

llvm::Value* WhileStmt::Emit() {
    Node::S->enterScope();
    if (DEBUG)
        cout << "WhileStmt" << endl;
    llvm::BasicBlock *pbb = Node::breakB;
    llvm::BasicBlock *pcb = Node::continueB;
    llvm::Function *f = Node::irgen->GetFunction();
    llvm::LLVMContext *context = Node::irgen->GetContext();
    llvm::BasicBlock *hb = Node::irgen->GetBasicBlock();
    llvm::BasicBlock *tb = llvm::BasicBlock::Create(*context, "while test", f);
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "while body", f);
    llvm::BasicBlock *fb = llvm::BasicBlock::Create(*context, "while footer", f);
    Node::breakB = fb;
    Node::continueB = tb;
    if (!hb->getTerminator()) {
        llvm::BranchInst::Create(tb, hb);
    }
    tb->moveAfter(Node::irgen->GetBasicBlock());
    Node::irgen->SetBasicBlock(tb);
    if (!tb->getTerminator()) {
        llvm::BranchInst::Create(bb, fb, test->Emit(), tb);
    }
    bb->moveAfter(tb);
    Node::irgen->SetBasicBlock(bb);   
    body->Emit();
    if (!bb->getTerminator()) {
        llvm::BranchInst::Create(tb, bb);
    }
    fb->moveAfter(Node::irgen->GetBasicBlock());
    if (!Node::irgen->GetBasicBlock()->getTerminator()) {
        llvm::BranchInst::Create(fb, Node::irgen->GetBasicBlock());
    }
    Node::irgen->SetBasicBlock(fb);
    Node::S->exitScope();
    Node::breakB = pbb;
    Node::continueB = pcb;
    return NULL;
}

llvm::Value* IfStmt::Emit() {
    Node::S->enterScope();
    if (DEBUG)
        cout << "IfStmt" << endl;
    llvm::LLVMContext *context = Node::irgen->GetContext();
    llvm::Function *f = Node::irgen->GetFunction();
    llvm::BasicBlock *hb = Node::irgen->GetBasicBlock();
    llvm::BasicBlock *tb = llvm::BasicBlock::Create(*context, "then", f);
    llvm::BasicBlock *eb = NULL;
    if (elseBody)
        eb = llvm::BasicBlock::Create(*context, "else", f);
    llvm::BasicBlock *fb = llvm::BasicBlock::Create(*context, "if footer", f);
    llvm::BranchInst::Create(tb, elseBody ? eb : fb, test->Emit(), hb);
    tb->moveAfter(hb);
    Node::irgen->SetBasicBlock(tb);
    body->Emit();
    if (!tb->getTerminator()) {
        llvm::BranchInst::Create(fb, tb);
    }
    if (!Node::irgen->GetBasicBlock()->getTerminator()) {
        llvm::BranchInst::Create(fb, Node::irgen->GetBasicBlock());
    }
    if (elseBody) {
        eb->moveAfter(Node::irgen->GetBasicBlock());
        Node::irgen->SetBasicBlock(eb);
        elseBody->Emit();
        if (!eb->getTerminator()) {
            llvm::BranchInst::Create(fb, eb);
        }
        Node::irgen->SetBasicBlock(eb);
    }
    fb->moveAfter(Node::irgen->GetBasicBlock());
    if (!Node::irgen->GetBasicBlock()->getTerminator()) {
        llvm::BranchInst::Create(fb, Node::irgen->GetBasicBlock());
    }
    Node::irgen->SetBasicBlock(fb);
    Node::S->exitScope();
    return NULL;
}

llvm::Value* Case::Emit() {
    if (DEBUG)
        cout << "SwitchLabel" << endl;
    llvm::LLVMContext *context = Node::irgen->GetContext();
    llvm::Function *f = Node::irgen->GetFunction();
    llvm::BasicBlock *switchB = Node::irgen->GetBasicBlock();
    llvm::BasicBlock *caseB = llvm::BasicBlock::Create(*context, "case", f);
    if (!switchB->getTerminator()) {
        llvm::BranchInst::Create(caseB, switchB);
    }
    Node::irgen->SetBasicBlock(caseB);
    stmt->Emit();
    Node::switchI->llvm::SwitchInst::addCase(llvm::cast<llvm::ConstantInt>(label->Emit()), caseB);
    return NULL;
}

llvm::Value* Default::Emit() {
    if (DEBUG)
        cout << "Default" << endl;
    llvm::LLVMContext *context = Node::irgen->GetContext();
    llvm::Function *f = Node::irgen->GetFunction();
    llvm::BasicBlock *defaultB = llvm::BasicBlock::Create(*context, "default", f);
    defaultB->moveAfter(Node::irgen->GetBasicBlock());
    Node::switchI->getDefaultDest()->removeFromParent();
    if (!Node::irgen->GetBasicBlock()->getTerminator()) {
        llvm::BranchInst::Create(defaultB, Node::irgen->GetBasicBlock());
    }
    Node::irgen->SetBasicBlock(defaultB);
    stmt->Emit();
    Node::switchI->setDefaultDest(defaultB);
    return NULL;
}

llvm::Value* SwitchStmt::Emit() {
    if (DEBUG)
        cout << "SwitchStmt" << endl;
    llvm::SwitchInst *psi = Node::switchI;
    llvm::BasicBlock *pbb = Node::breakB;
    llvm::LLVMContext *context = Node::irgen->GetContext();
    llvm::Function *f = Node::irgen->GetFunction();
    llvm::BasicBlock *head = Node::irgen->GetBasicBlock();
    llvm::BasicBlock *defC = llvm::BasicBlock::Create(*context, "default", f);
    llvm::BasicBlock *foot = llvm::BasicBlock::Create(*context, "Switch Foot", f);
    Node::breakB = foot;
    Node::switchI = llvm::SwitchInst::Create(expr->Emit(), defC, cases->NumElements(), head);
    for (int i = 0;  i < cases->NumElements(); i++) {
        if (Default *de = dynamic_cast<Default*>(cases->Nth(i))) {
            def = de;
            break;
        }
        cases->Nth(i)->Emit();
    }
    if (def) {
        def->Emit();
        if (!Node::irgen->GetBasicBlock()->getTerminator()) {
            llvm::BranchInst::Create(foot, Node::irgen->GetBasicBlock());
        }
        foot->moveAfter(Node::irgen->GetBasicBlock());
    }
    else {
        //cout << "NO DEF" << endl;
        defC->moveAfter(Node::irgen->GetBasicBlock());
        llvm::BranchInst::Create(foot, defC);
        if (!Node::irgen->GetBasicBlock()->getTerminator()) {
            llvm::BranchInst::Create(defC, Node::irgen->GetBasicBlock());
        }
        foot->moveAfter(defC);
    }
    Node::irgen->SetBasicBlock(foot);
    Node::breakB = pbb;
    Node::switchI = psi;
    return NULL;
}
llvm::Value* BreakStmt::Emit() {
    if (DEBUG)
        cout << "BreakStmt" << endl;
    llvm::BasicBlock *bb = Node::irgen->GetBasicBlock();
    llvm::BasicBlock *suc = Node::breakB;
    llvm::BranchInst::Create(suc, bb);
    return NULL;
}

llvm::Value* ContinueStmt::Emit() {
    if (DEBUG)
        cout << "ContinueStmt" << endl;
    llvm::BasicBlock *bb = Node::irgen->GetBasicBlock();
    llvm::BasicBlock *suc = Node::continueB;
    llvm::BranchInst::Create(suc, bb);
    return NULL;
}

llvm::Value* ReturnStmt::Emit() {
    if (DEBUG)
        cout << "ReturnStmt" << endl;
    llvm::LLVMContext* context = Node::irgen->GetContext();
    llvm::BasicBlock *bb = Node::irgen->GetBasicBlock();
    if (expr) {
        llvm::ReturnInst::Create(*context, expr->Emit(), bb);
    }
    else {
        llvm::ReturnInst::Create(*context, bb);
    }
    return NULL;
}
