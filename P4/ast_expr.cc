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

llvm::Value* VarExpr::EmitAddress() {
  llvm::Value* mem = S->find(id->getName()).val;
  return mem;
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
    llvm::Value* addr = right->EmitAddress();
    llvm::Type* rType = rhs->getType();
    char* oper = op->getOp();
    if( rType->isFloatTy() ) {
      //rhs is float
      if( strcmp(oper, "++") == 0 ) {
        //Prefix increment
        llvm::Type* fConst = irgen->IRGenerator::GetFloatType();
        llvm::Value* inc = llvm::ConstantFP::get(fConst, 1.0);
        llvm::Value* result = llvm::BinaryOperator::CreateFAdd(inc, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
        llvm::StoreInst(result, addr, "",
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "--") == 0 ) {
        //Prefix decrement
        llvm::Type* fConst = irgen->IRGenerator::GetFloatType();
        llvm::Value* dec = llvm::ConstantFP::get(fConst, 1.0);
        llvm::Value* result = llvm::BinaryOperator::CreateFSub(dec, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
        llvm::StoreInst(result, addr, "",
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "+") == 0 ) {
        //This does nothing???
        return rhs;
      } else if( strcmp(oper, "-") == 0 ) {
        //Neg
        llvm::Value* result = llvm::BinaryOperator::CreateFNeg(rhs, "", 
		irgen->IRGenerator::GetBasicBlock());
        llvm::StoreInst(result, addr, "",
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else {
        //shouldnt be here
        return NULL;
      }
    } else if( rType->isIntegerTy() ) {
      //rhs is integer
      if( strcmp(oper, "++") == 0 ) {
        //Prefix increment
        llvm::Type* iConst = irgen->IRGenerator::GetIntType();
        llvm::Value* dec = llvm::ConstantInt::get(iConst, 1, true);
        llvm::Value* result = llvm::BinaryOperator::CreateAdd(dec, rhs, "",
		irgen->IRGenerator::GetBasicBlock());
        llvm::StoreInst(result, addr, "",
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "--") == 0 ) {
        //Prefix decrement
        llvm::Type* iConst = irgen->IRGenerator::GetIntType();
        llvm::Value* inc = llvm::ConstantInt::get(iConst, 1, true);
        llvm::Value* result = llvm::BinaryOperator::CreateSub(inc, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
        llvm::StoreInst(result, addr, "",
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "+") == 0 ) {
        //Pos?
        return rhs;
      } else if( strcmp(oper, "-") == 0 ) {
        //Neg
        llvm::Value* result = llvm::BinaryOperator::CreateNeg(rhs, "",
                irgen->IRGenerator::GetBasicBlock());
        llvm::StoreInst(result, addr, "",
                irgen->IRGenerator::GetBasicBlock());
        return result;
      
      } else {
        //shouldn't be here
        return NULL;
      }
    } else if( rType->isVectorTy() ) {
      //rhs is vector
      std::vector<llvm::Constant*> fVec;
      llvm::VectorType* vec = (llvm::VectorType*) rType;
      llvm::Constant* fl1 = llvm::ConstantFP::get(
                irgen->IRGenerator::GetFloatType(), 1.0);
      llvm::Constant* fl2 = llvm::ConstantFP::get(
                irgen->IRGenerator::GetFloatType(), 1.0);
      llvm::Constant* fl3 = llvm::ConstantFP::get(
                irgen->IRGenerator::GetFloatType(), 1.0);
      llvm::Constant* fl4 = llvm::ConstantFP::get(
                irgen->IRGenerator::GetFloatType(), 1.0);
      fVec.push_back(fl1);
      fVec.push_back(fl2);
      if( vec->getNumElements() == 3 ) {
        fVec.push_back(fl3);
      } else if( vec->getNumElements() == 4 ) {
        fVec.push_back(fl3);
        fVec.push_back(fl4);
      }
      llvm::ConstantVector* vect = 
		(llvm::ConstantVector *)llvm::ConstantVector::get(fVec);
      if( strcmp(oper, "++") == 0 ) {
        //prefix inc
        llvm::Value* result = llvm::BinaryOperator::CreateFAdd(vect, rhs, "",
		irgen->IRGenerator::GetBasicBlock());
        llvm::StoreInst(result, addr, "",
		irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "--") == 0 ) {
        //prefix dec
        llvm::Value* result = llvm::BinaryOperator::CreateFSub(vect, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
        llvm::StoreInst(result, addr, "",
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "+") == 0 ) {
        //do nothing
        return rhs;
      } else if( strcmp(oper, "-") == 0 ) {
        //negate
        llvm::Value* result = llvm::BinaryOperator::CreateFNeg(rhs, "",
		irgen->IRGenerator::GetBasicBlock());
        llvm::StoreInst(result, addr, "", 
		irgen->IRGenerator::GetBasicBlock());
        return result;
      } else {
        //shouldn't be here
      }
    } else {
      //shouldn't be here
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
        llvm::VectorType* vec = (llvm::VectorType*) rType;
        llvm::Value* addr = right->EmitAddress();
        for( int i = 0; i < vec->getNumElements(); ++i ) {
          llvm::Value* baseAddr = new llvm::LoadInst(addr, "",
		irgen->IRGenerator::GetBasicBlock());
          llvm::Constant* vecId = 
		llvm::ConstantInt::get(irgen->IRGenerator::GetIntType(), i);
          llvm::Value* val = llvm::ExtractElementInst::Create(rhs, vecId, "",
		irgen->IRGenerator::GetBasicBlock());
          llvm::Value* fRes = ArithmeticExpr::fcomp(lhs, val, oper);
          llvm::InsertElementInst::Create(rhs, fRes, vecId, "", 
		irgen->IRGenerator::GetBasicBlock());
        }
        return rhs;
      } else if( lType->isVectorTy() && rType->isFloatTy() ) {
        //Lhs is vec rhs is float
        llvm::VectorType* vec = (llvm::VectorType*) lType;
        llvm::Value* addr = left->EmitAddress();
        for( int i = 0; i < vec->getNumElements(); ++i ) {
          llvm::Value* baseAddr = new llvm::LoadInst(addr, "",
                irgen->IRGenerator::GetBasicBlock());
          llvm::Constant* vecId =
                llvm::ConstantInt::get(irgen->IRGenerator::GetIntType(), i);
          llvm::Value* val = llvm::ExtractElementInst::Create(lhs, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
          llvm::Value* fRes = ArithmeticExpr::fcomp(val, rhs, oper);
          llvm::InsertElementInst::Create(lhs, fRes, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
        }
        return lhs;
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
  if( lType->isFloatTy() ) {
    //lhs is float
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
  } else if ( lType->isVectorTy() ) {
    llvm::CmpInst::OtherOps llvmOP = llvm::CmpInst::FCmp;
    llvm::CmpInst::Predicate pred = llvm::CmpInst::FCMP_OEQ;
    llvm::Value* vecResult = llvm::CmpInst::Create(llvmOP, pred, lhs, rhs, "",
	irgen->IRGenerator::GetBasicBlock());
    llvm::VectorType* vec = (llvm::VectorType *) vecResult->getType();
    llvm::Value* ind = llvm::ConstantInt::get(irgen->IRGenerator::GetIntType(),
	0, false);
    llvm::Value* result = llvm::ExtractElementInst::Create(vecResult,
	 ind, "", irgen->IRGenerator::GetBasicBlock());
    //TODO: getNumElements() may not return correct number
    for( int i = 1; i < vec->getNumElements(); ++i ) {
      llvm::Value* index = 
	llvm::ConstantInt::get(irgen->IRGenerator::GetIntType(), i, false);
      llvm::Value *next = llvm::ExtractElementInst::Create(vecResult, index, "",
	irgen->IRGenerator::GetBasicBlock());
      result = llvm::BinaryOperator::CreateAnd(result, next, "", 
	irgen->IRGenerator::GetBasicBlock());
    }
    if( strcmp(oper, "==") == 0 ) {
      //Is equal operation
      return result;
    } else {
      //Is not equal operation
      return llvm::BinaryOperator::CreateNot(result, "",
	irgen->IRGenerator::GetBasicBlock());
    }
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
    llvm::Value* result = llvm::BinaryOperator::CreateOr(lhs, rhs, "",
	irgen->IRGenerator::GetBasicBlock());
    return result;
  } else if( strcmp(oper, "&&") == 0 ) {
    llvm::Value* result = llvm::BinaryOperator::CreateAnd(lhs, rhs, "",
	irgen->IRGenerator::GetBasicBlock());
    return result;
  } else {
    //shouldn't be here
  }

  return NULL;
}

llvm::Value* AssignExpr::Emit() {
  llvm::Value* lhs = left->EmitAddress();
  llvm::Value* rhs = right->Emit();
  llvm::Type* lType = lhs->getType();
  llvm::Type* rType = rhs->getType();
  char* oper = op->getOp();
  if( strcmp(oper, "=") == 0 ) {
    //normal assign
    llvm::Value* result = new llvm::StoreInst(rhs, lhs,
	irgen->IRGenerator::GetBasicBlock());
    return result;
  } else if( strcmp(oper, "+=") == 0 ) {
    //plus equals
    if( lType->isFloatTy() || lType->isVectorTy() ) {
      //lhs is float or vec2/3/4
      llvm::Value* add = llvm::BinaryOperator::CreateFAdd(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      llvm::Value* result = new llvm::StoreInst(add, lhs,
        irgen->IRGenerator::GetBasicBlock());
      return result;
    } else if( lType->isIntegerTy() ) {
      //lhs is integer
      llvm::Value* add = llvm::BinaryOperator::CreateAdd(lhs, rhs, "", 
	irgen->IRGenerator::GetBasicBlock());
      llvm::Value* result = new llvm::StoreInst(add, lhs,
	irgen->IRGenerator::GetBasicBlock());
      return result;
    }
  } else if( strcmp(oper, "-=") == 0 ) {
    //minus equals
    if( lType->isFloatTy() || lType->isVectorTy() ) {
      //lhs is float or vec2/3/4
      llvm::Value* add = llvm::BinaryOperator::CreateFSub(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      llvm::Value* result = new llvm::StoreInst(add, lhs,
        irgen->IRGenerator::GetBasicBlock());
      return result;
    } else if( lType->isIntegerTy() ) {
      //lhs is integer
      llvm::Value* add = llvm::BinaryOperator::CreateSub(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      llvm::Value* result = new llvm::StoreInst(add, lhs,
        irgen->IRGenerator::GetBasicBlock());
      return result;
    }
  } else if( strcmp(oper, "*=") == 0 ) {
    //multipy equals
    if( lType->isFloatTy() || lType->isVectorTy() ) {
      //lhs is float or vec2/3/4
      llvm::Value* add = llvm::BinaryOperator::CreateFMul(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      llvm::Value* result = new llvm::StoreInst(add, lhs,
        irgen->IRGenerator::GetBasicBlock());
      return result;
    } else if( lType->isIntegerTy() ) {
      //lhs is integer
      llvm::Value* add = llvm::BinaryOperator::CreateMul(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      llvm::Value* result = new llvm::StoreInst(add, lhs,
        irgen->IRGenerator::GetBasicBlock());
      return result;
    }
  } else if( strcmp(oper, "/=") == 0 ) {
    //divide equals
    if( lType->isFloatTy() || lType->isVectorTy() ) {
      //lhs is float or vec2/3/4
      llvm::Value* add = llvm::BinaryOperator::CreateFDiv(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      llvm::Value* result = new llvm::StoreInst(add, lhs,
        irgen->IRGenerator::GetBasicBlock());
      return result;   
    } else if( lType->isIntegerTy() ) {
      //lhs is integer
      llvm::Value* add = llvm::BinaryOperator::CreateSDiv(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      llvm::Value* result = new llvm::StoreInst(add, lhs,
        irgen->IRGenerator::GetBasicBlock());
      return result;
    }
  } else {
    //should't be here

  }

  return NULL;
}

llvm::Value* PostfixExpr::Emit() {
  llvm::Value* lhs = left->Emit();
  llvm::Value* addr = left->EmitAddress();
  llvm::Type* lType = lhs->getType();
  char* oper = op->getOp();
  if( lType->isFloatTy() || lType->isVectorTy() ) {
    if( strcmp(oper, "++") == 0 ) {
      //Postfix inc
      llvm::Type* iConst = irgen->IRGenerator::GetIntType();
      llvm::Value* inc = llvm::ConstantInt::get(iConst, 1, true);
      llvm::Value* result = llvm::BinaryOperator::CreateFAdd(lhs, inc, "",
                irgen->IRGenerator::GetBasicBlock());
      llvm::StoreInst(result, addr, "", 
		irgen->IRGenerator::GetBasicBlock());
      return result;  
    } else if( strcmp(oper, "--") == 0 ) {
      //Postfix dec
      llvm::Type* iConst = irgen->IRGenerator::GetIntType();
      llvm::Value* dec = llvm::ConstantInt::get(iConst, 1, true);
      llvm::Value* result = llvm::BinaryOperator::CreateFSub(lhs, dec, "",
                irgen->IRGenerator::GetBasicBlock());
      llvm::StoreInst(result, addr, "",
                irgen->IRGenerator::GetBasicBlock());
      return result;
    } else {
      //are there any other postfix ops?
    }
  } else if( lType->isIntegerTy() ) {
    if( strcmp(oper, "++") == 0 ) {
      //Postfix inc
      llvm::Type* iConst = irgen->IRGenerator::GetIntType();
      llvm::Value* inc = llvm::ConstantInt::get(iConst, 1, true);
      llvm::Value* result = llvm::BinaryOperator::CreateAdd(lhs, inc, "",
                irgen->IRGenerator::GetBasicBlock());
      llvm::StoreInst(result, addr, "",
                irgen->IRGenerator::GetBasicBlock());
      return result;
    } else if( strcmp(oper, "--") == 0 ) {
      //Postfix dec
      llvm::Type* iConst = irgen->IRGenerator::GetIntType();
      llvm::Value* dec = llvm::ConstantInt::get(iConst, 1, true);
      llvm::Value* result = llvm::BinaryOperator::CreateSub(lhs, dec, "",
                irgen->IRGenerator::GetBasicBlock());
      llvm::StoreInst(result, addr, "",
                irgen->IRGenerator::GetBasicBlock());
      return result;
    } else {
      //are there any other postfix ops?
    }
  /*
  //TODO:: mat2/3/4 EC???
  } else if( lhs->getType() == lType ) {
    
  }
  */
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

llvm::Value* FieldAccess::Emit() {

  return NULL;
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
 
