/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"

llvm::Value* Expr::Emit() {
  return NULL;
}

llvm::Value* ExprError::Emit() {
  //Shouldn't have any ExprErrors
  return NULL;
}

llvm::Value* EmptyExpr::Emit() {
  //Shouldn't have any EmptyExpr
  return NULL;
}

llvm::Value* IntConstant::Emit() {
  llvm::Type* iConst = irgen->IRGenerator::GetIntType();
  llvm::Value* result = llvm::ConstantFP::get(iConst, value);
  return result;
}

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}

void IntConstant::PrintChildren(int indentLevel) { 
    printf("%d", value);
}

llvm::Value* FloatConstant::Emit() {
  llvm::Type* fConst = irgen->IRGenerator::GetFloatType();
  llvm::Value* result = llvm::ConstantFP::get(fConst, value);
  return result;
}


FloatConstant::FloatConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}
void FloatConstant::PrintChildren(int indentLevel) { 
    printf("%g", value);
}

llvm::Value* BoolConstant::Emit() {
  llvm::Type* bConst = irgen->IRGenerator::GetBoolType();
  llvm::Value* result = llvm::ConstantInt::get(bConst, value);
  return result;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}
void BoolConstant::PrintChildren(int indentLevel) { 
    printf("%s", value ? "true" : "false");
}

VarExpr::VarExpr(yyltype loc, Identifier *ident) : Expr(loc) {
    Assert(ident != NULL);
    this->id = ident;
}

llvm::Value* VarExpr::Emit() {
  llvm::Value* mem = S->find(id->getName()).val;
  llvm::Value* result = new llvm::LoadInst(mem, id->getName(), 
		irgen->IRGenerator::GetBasicBlock());
 
  return result;
}

void VarExpr::PrintChildren(int indentLevel) {
    id->Print(indentLevel+1);
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}

char *Operator::getOp() {
  return tokenString;
}

void Operator::PrintChildren(int indentLevel) {
    printf("%s",tokenString);
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o) 
  : Expr(Join(l->GetLocation(), o->GetLocation())) {
    Assert(l != NULL && o != NULL);
    (left=l)->SetParent(this);
    (op=o)->SetParent(this);
}

void CompoundExpr::PrintChildren(int indentLevel) {
   if (left) left->Print(indentLevel+1);
   op->Print(indentLevel+1);
   if (right) right->Print(indentLevel+1);
}

llvm::Value* ArithmeticExpr::Emit() {
  if( left == NULL ) {
    //Prefix expression
    llvm::Value* rhs = right->Emit();
    llvm::Type* rType = rhs->getType();
    char* oper = op->getOp();
    if( rType->isFloatTy() || rType->isVectorTy() ) {
      if( strcmp(oper, "++") == 0 ) {
        //Prefix increment
        llvm::Type* fConst = irgen->IRGenerator::GetFloatType();
        llvm::Value* inc = llvm::ConstantFP::get(fConst, 1.0);
        llvm::Value* result = llvm::BinaryOperator::CreateFAdd(inc, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "--") == 0 ) {
        //Prefix decrement
        llvm::Type* fConst = irgen->IRGenerator::GetFloatType();
        llvm::Value* dec = llvm::ConstantFP::get(fConst, 1.0);
        llvm::Value* result = llvm::BinaryOperator::CreateFSub(dec, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "+") == 0 ) {
        //Pos?

      } else if( strcmp(oper, "-") == 0 ) {
        //Neg
        llvm::Value* result = llvm::BinaryOperator::CreateFNeg(rhs, "", 
		irgen->IRGenerator::GetBasicBlock());
        return result;
      } else {
        //shouldnt be here
        return NULL;
      }
    } else if( rType->isIntegerTy() ) {
      if( strcmp(oper, "++") == 0 ) {
        //Prefix increment
        llvm::Type* iConst = irgen->IRGenerator::GetIntType();
        llvm::Value* dec = llvm::ConstantInt::get(iConst, 1, true);
        llvm::Value* result = llvm::BinaryOperator::CreateAdd(dec, rhs, "",
		irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "--") == 0 ) {
        //Prefix decrement
        llvm::Type* iConst = irgen->IRGenerator::GetIntType();
        llvm::Value* inc = llvm::ConstantInt::get(iConst, 1, true);
        llvm::Value* result = llvm::BinaryOperator::CreateSub(inc, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "+") == 0 ) {
        //Pos?

      } else if( strcmp(oper, "-") == 0 ) {
        //Neg
        llvm::Value* result = llvm::BinaryOperator::CreateNeg(rhs, "",
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else {
        //shouldn't be here
        return NULL;
      }
    }
  } else {
    //is a normal ArithmeticExpr
    llvm::Value* lhs = left->Emit();
    llvm::Value* rhs = right->Emit();
    llvm::Type* lType = lhs->getType();
    llvm::Type* rType = rhs->getType();
    char * oper = op->getOp();
    if( lType == rType ) {
      //Left and right are of same type
      if( lType->isFloatTy() || lType->isVectorTy() ) {
        //Left and right are floats or vec2/3/4
         llvm::Value* result = ArithmeticExpr::fcomp(lhs, rhs, oper);
         return result;
      } else if( lType->isIntegerTy() ) {
        llvm::Value* result = ArithmeticExpr::comp(lhs, rhs, oper);
        return result;
      }
      /*
      //TODO: find how to compare types (check lhs = mat2/3/4)
      } else if( lhs->getType() == lType ) {
      
      }
      */
    } else {
      //lhs and rhs are of different types
      if( lType->isFloatTy() && rType->isVectorTy() ) {
        //Lhs is float rhs is vec
        llvm::Value* fToVec; 
        llvm::VectorType* vec = (llvm::VectorType*) rType;
        if( vec->getNumElements() == 2 ) {
          //TODO: make fToVec a vec2 and insert lhs' value
          llvm::Value* result = ArithmeticExpr::fcomp(fToVec, rhs, oper);
          return result; 
        } else if( vec->getNumElements() == 3 ){
          //TODO: make fToVec a vec3 and insert lhs' value
          llvm::Value* result = ArithmeticExpr::fcomp(fToVec, rhs, oper);
          return result;
        } else if( vec->getNumElements() == 4 ) {
          //TODO: make fToVec a vec4 and insert lhs' value
          llvm::Value* result = ArithmeticExpr::fcomp(fToVec, rhs, oper);
          return result;
        }
      } else if( lType->isVectorTy() && rType->isFloatTy() ) {
        //Lhs is vec rhs is float
        llvm::Value* fToVec;
        llvm::VectorType* vec = (llvm::VectorType*) lType;
        if( vec->getNumElements() == 2 ) {
          //TODO: make fToVec a vec2 and insert rhs' value
          llvm::Value* result = ArithmeticExpr::fcomp(lhs, fToVec, oper);
          return result;
        } else if( vec->getNumElements() == 3 ){
          //TODO: make fToVec a vec3 and insert rhs' value
          llvm::Value* result = ArithmeticExpr::fcomp(lhs, fToVec, oper);
          return result;
        } else if( vec->getNumElements() == 4 ) {
          //TODO: make fToVec a vec4 and insert rhs' value
          llvm::Value* result = ArithmeticExpr::fcomp(lhs, fToVec, oper);
          return result;
        }
      /*
      //TODO: float with mat2/3/4?? EC??
      } else if( lhs->getType() == lType ) {
      
      //TODO:: vec2/3/4 with mat2/3/4?? 
      } else if( lhs->getType() == lType ) {
      
      }
      */
      }
    }
  
  }
  return NULL;
}

llvm::Value* ArithmeticExpr::comp(llvm::Value* lhs, 
	llvm::Value* rhs, char* oper) {
  if( strcmp(oper, "+") == 0 ) {
    //Operation add
    llvm::Value* result =
	llvm::BinaryOperator::CreateAdd(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
    return result;
  } else if( strcmp(oper, "-") == 0 ) {
    //Operation sub
    llvm::Value* result =
        llvm::BinaryOperator::CreateSub(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
    return result;
  } else if( strcmp(oper, "*") == 0 ) {
    //Operation mul
    llvm::Value* result =
        llvm::BinaryOperator::CreateMul(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
    return result;
  } else {
    //Operation div
    llvm::Value* result =
        llvm::BinaryOperator::CreateSDiv(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
    return result;
  }
  return NULL;
}

llvm::Value* ArithmeticExpr::fcomp(llvm::Value* lhs, 
	llvm::Value* rhs, char* oper) {
  if( strcmp(oper, "+") == 0 ) {
    //Operation add
    llvm::Value* result =
        llvm::BinaryOperator::CreateFAdd(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
    return result;
  } else if( strcmp(oper, "-") == 0 ) {
    //Operation sub
    llvm::Value* result =
        llvm::BinaryOperator::CreateFSub(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
    return result;
  } else if( strcmp(oper, "*") == 0 ) {
    //Operation mul
    llvm::Value* result =
        llvm::BinaryOperator::CreateFMul(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
    return result;
  } else {
    //Operation div
    llvm::Value* result =
        llvm::BinaryOperator::CreateFDiv(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
    return result;
  }
  return NULL;
}

llvm::Value* RelationalExpr::Emit() {
  llvm::Value* lhs = left->Emit();
  llvm::Value* rhs = right->Emit();
  llvm::Type* lType = lhs->getType();
  llvm::Type* rType = rhs->getType();
  char* oper = op->getOp();
  if( lType->isFloatTy() ) {
    //lhs is float
    llvm::CmpInst::OtherOps llvmOP = llvm::CmpInst::FCmp;
    llvm::CmpInst::Predicate pred;
    if( strcmp(oper, ">") ) {
      //Is less greater than operation
      pred = llvm::CmpInst::FCMP_OGT;
    } else if( strcmp(oper, "<") ) {
      //Is less than operation
      pred = llvm::CmpInst::FCMP_OLT;
    } else if( strcmp(oper, ">=") ) {
      //Is greater than or equal operation
      pred = llvm::CmpInst::FCMP_OGE;
    } else if( strcmp(oper, "<=") ) {
      //Is less than or equal operation
      pred = llvm::CmpInst::FCMP_OLE;
    } else {
      //Get rid of warning
      pred = llvm::CmpInst::FCMP_OLE;
    }
    llvm::Value* result = llvm::CmpInst::Create(llvmOP, pred, lhs, rhs, "", 
	irgen->IRGenerator::GetBasicBlock());
    return result;
  } else if( rType->isIntegerTy() ) {
    //lhs is int
    llvm::CmpInst::OtherOps llvmOP = llvm::CmpInst::ICmp;
    llvm::CmpInst::Predicate pred;
    if( strcmp(oper, ">") == 0 ) {
      //Is less greater than operation
      pred = llvm::CmpInst::ICMP_SGT;
    } else if( strcmp(oper, "<") == 0 ) {
      //Is less than operation
      pred = llvm::CmpInst::ICMP_SLT;
    } else if( strcmp(oper, ">=") == 0 ) {
      //Is greater than or equal operation
      pred = llvm::CmpInst::ICMP_SGE;
    } else if( strcmp(oper, "<=") == 0 ) {
      //Is less than or equal operation
      pred = llvm::CmpInst::ICMP_SLE;
    } else {
      //Get rid of warning
      pred = llvm::CmpInst::ICMP_SLE;
    }
    llvm::Value* result = llvm::CmpInst::Create(llvmOP, pred, lhs, rhs, "",
	irgen->IRGenerator::GetBasicBlock());
    return result;
  }
  return NULL;
}

llvm::Value* EqualityExpr::Emit() {
  llvm::Value* lhs = left->Emit();
  llvm::Value* rhs = right->Emit();
  llvm::Type* lType = lhs->getType();
  llvm::Type* rType = rhs->getType();
  char* oper = op->getOp();
  if( lType->isFloatTy() || lType->isVectorTy() ) {
    //lhs is float or vec2/3/4
    llvm::CmpInst::OtherOps llvmOP = llvm::CmpInst::FCmp;
    llvm::CmpInst::Predicate pred;
    if( strcmp(oper, "==") == 0) {
      //Is equal operation
      pred = llvm::CmpInst::FCMP_OEQ;
    } else if( strcmp(oper, "!=") == 0 ) {
      //Is not equal operation
      pred = llvm::CmpInst::FCMP_ONE;
    } else {
      //Get rid of warning
      pred = llvm::CmpInst::FCMP_ONE;
    }
    llvm::Value* result = llvm::CmpInst::Create(llvmOP, pred, lhs, rhs, "",
	irgen->IRGenerator::GetBasicBlock());
    return result;
  } else if( lType->isIntegerTy() ) {
    //lhs is int or bool
    llvm::CmpInst::OtherOps llvmOP = llvm::CmpInst::ICmp;
    llvm::CmpInst::Predicate pred;
    if( strcmp(oper, "==") == 0) {
      //Is equal operation
      pred = llvm::CmpInst::ICMP_EQ;
    } else if( strcmp(oper, "!=") == 0 ) {
      //Is not equal operation
      pred = llvm::CmpInst::ICMP_NE;
    } else {
      //Get rid of warning
      pred = llvm::CmpInst::ICMP_NE;
    }
    llvm::Value* result = llvm::CmpInst::Create(llvmOP, pred, lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
    return result;
  }
  //TODO: Can this have a void type?
  return NULL;
}

llvm::Value* LogicalExpr::Emit() {
  llvm::Value* lhs = left->Emit();
  llvm::Value* rhs = right->Emit();
  llvm::Type* lType = lhs->getType();
  llvm::Type* rType = rhs->getType();
  char* oper = op->getOp();
  if( strcmp(oper, "||") == 0 ) {
  
  } else if( strcmp(oper, "&&") == 0 ) {
  
  } else {
    //Get rid of warnings
  
  }

  return NULL;
}

llvm::Value* AssignExpr::Emit() {
  llvm::Value* lhs = left->Emit();
  llvm::Value* rhs = right->Emit();
  char* oper = op->getOp();

  return NULL;
}

llvm::Value* PostfixExpr::Emit() {
  llvm::Value* lhs = left->Emit();
  llvm::Type* lType = lhs->getType();
  char* oper = op->getOp();
  //TODO: (lhs = float | vec2/3/4)
  if( lhs->getType() == lType ) {
    if( strcmp(oper, "++") == 0 ) {
      //Postfix inc
      llvm::Type* iConst = irgen->IRGenerator::GetIntType();
      llvm::Value* inc = llvm::ConstantInt::get(iConst, 1, true);
      llvm::Value* result = llvm::BinaryOperator::CreateFAdd(lhs, inc, "",
                irgen->IRGenerator::GetBasicBlock());
      return result;  
    } else if( strcmp(oper, "--") == 0 ) {
      //Postfix dec
      llvm::Type* iConst = irgen->IRGenerator::GetIntType();
      llvm::Value* dec = llvm::ConstantInt::get(iConst, 1, true);
      llvm::Value* result = llvm::BinaryOperator::CreateFSub(lhs, dec, "",
                irgen->IRGenerator::GetBasicBlock());
      return result;
    } else {
      //are there any other postfix ops?
    }
  //TODO: (lhs = int)
  } else if( lhs->getType() == lType ) {
    if( strcmp(oper, "++") == 0 ) {
      //Postfix inc
      llvm::Type* iConst = irgen->IRGenerator::GetIntType();
      llvm::Value* inc = llvm::ConstantInt::get(iConst, 1, true);
      llvm::Value* result = llvm::BinaryOperator::CreateAdd(lhs, inc, "",
                irgen->IRGenerator::GetBasicBlock());
      return result;
    } else if( strcmp(oper, "--") == 0 ) {
      //Postfix dec
      llvm::Type* iConst = irgen->IRGenerator::GetIntType();
      llvm::Value* dec = llvm::ConstantInt::get(iConst, 1, true);
      llvm::Value* result = llvm::BinaryOperator::CreateSub(lhs, dec, "",
                irgen->IRGenerator::GetBasicBlock());
      return result;
    } else {
      //are there any other postfix ops?
    }
  //TODO:: mat2/3/4 EC???
  } else if( lhs->getType() == lType ) {
    
  }
  return NULL;
}
  
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

void ArrayAccess::PrintChildren(int indentLevel) {
    base->Print(indentLevel+1);
    subscript->Print(indentLevel+1, "(subscript) ");
  }
     
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}


  void FieldAccess::PrintChildren(int indentLevel) {
    if (base) base->Print(indentLevel+1);
    field->Print(indentLevel+1);
  }

Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}

 void Call::PrintChildren(int indentLevel) {
    if (base) base->Print(indentLevel+1);
    if (field) field->Print(indentLevel+1);
    if (actuals) actuals->PrintAll(indentLevel+1, "(actuals) ");
  }
 
