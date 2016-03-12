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
  if( DEBUG ) {
    printf("Int\n");
  }
  llvm::Type* iConst = irgen->IRGenerator::GetIntType();
  llvm::Value* result = llvm::ConstantInt::get(iConst, value);
  return result;
}

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}

void IntConstant::PrintChildren(int indentLevel) { 
    printf("%d", value);
}

llvm::Value* FloatConstant::Emit() {
  if( DEBUG ) {
    printf("Float\n");
  }
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
  if( DEBUG ) {
    printf("Bool\n");
  }
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
  if( DEBUG ) {
    printf("VarExpr\n");
  }
  /*
  container c = Node::S->find(id->getName());
  if (c.flag == GLOBAL)
      return new llvm::LoadInst(c.val, id->getName(), Node::irgen->GetBasicBlock());
  else if (c.flag == LOCAL)
      return c.val;
  */
  llvm::Value* mem = S->find(id->getName()).val;
  llvm::Value* result = new llvm::LoadInst(mem, id->getName(), 
		irgen->IRGenerator::GetBasicBlock());
  return result;
}

llvm::Value* VarExpr::EmitAddress() {
  if( DEBUG ) {
    printf("VarExpr EmitAddress\n");
  }
   
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
    if( DEBUG ) {
      printf("Prefix\n");
    }
    llvm::Value* rhs = right->Emit();
    llvm::Value* addr;
    const char* cSwiz = "";
    if( VarExpr* rightV = dynamic_cast<VarExpr*>(right) ) {
      addr = rightV->EmitAddress();
    } else if( FieldAccess* f = dynamic_cast<FieldAccess*>(right) ) {
      addr = f->EmitAddress();
      cSwiz = f->getId()->getName();
    } else {
      if( DEBUG ) printf("prefix not var or field\n");
      addr = right->Emit();
    }
    
    llvm::Type* rType = rhs->getType();
    char* oper = op->getOp();
    if( strlen(cSwiz) != 0 ) {
      //field assignment
      llvm::Constant* inc = llvm::ConstantFP::get(
		irgen->IRGenerator::GetFloatType(), 1.0);
      llvm::Value* baseAddr = new llvm::LoadInst(addr, "",
                irgen->IRGenerator::GetBasicBlock());
      llvm::Constant* vecId;
      for( unsigned i = 0; i < strlen(cSwiz); i++ ) {
        char c = cSwiz[i];
        if( c == 'x' ) {
          vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 0);
        } else if( c == 'y' ) {
          vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 1);
        } else if( c == 'z' ) {
          vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 2);
        } else {
          vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 3);
        }
        llvm::Value* ext = llvm::ExtractElementInst::Create(baseAddr, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
        if( strcmp(oper, "++") == 0 ) {
          llvm::Value* result = llvm::BinaryOperator::CreateFAdd(ext, inc, "",
                irgen->IRGenerator::GetBasicBlock());
           baseAddr = llvm::InsertElementInst::Create(baseAddr, result, vecId, 
		"", irgen->IRGenerator::GetBasicBlock());
        } else if( strcmp(oper, "--") == 0 ) {
          llvm::Value* result = llvm::BinaryOperator::CreateFSub(ext, inc, "",
                irgen->IRGenerator::GetBasicBlock());
           baseAddr = llvm::InsertElementInst::Create(baseAddr, result, vecId, 
		"", irgen->IRGenerator::GetBasicBlock());
        } else if( strcmp(oper, "+") == 0 ) {
          
        } else if( strcmp(oper, "-") == 0 ) {
           llvm::Value* result = llvm::BinaryOperator::CreateFNeg(ext, "",
                irgen->IRGenerator::GetBasicBlock());
            baseAddr = llvm::InsertElementInst::Create(baseAddr, result, vecId,
                "", irgen->IRGenerator::GetBasicBlock());
        }
      }
      llvm::Value* res = new llvm::StoreInst(baseAddr, addr, "",
                irgen->IRGenerator::GetBasicBlock());
        return baseAddr;
    }
    if( rType->isFloatTy() ) {
      //rhs is float
      if( strcmp(oper, "++") == 0 ) {
        //Prefix increment
        llvm::Type* fConst = irgen->IRGenerator::GetFloatType();
        llvm::Value* inc = llvm::ConstantFP::get(fConst, 1.0);
        llvm::Value* result = llvm::BinaryOperator::CreateFAdd(inc, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
        new llvm::StoreInst(result, addr,
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "--") == 0 ) {
        //Prefix decrement
        llvm::Type* fConst = irgen->IRGenerator::GetFloatType();
        llvm::Value* dec = llvm::ConstantFP::get(fConst, 1.0);
        llvm::Value* result = llvm::BinaryOperator::CreateFSub(rhs, dec, "",
                irgen->IRGenerator::GetBasicBlock());
        new llvm::StoreInst(result, addr,
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "+") == 0 ) {
        //This does nothing???
        return rhs;
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
      //rhs is integer
      if( strcmp(oper, "++") == 0 ) {
        //Prefix increment
        llvm::Type* iConst = irgen->IRGenerator::GetIntType();
        llvm::Value* inc = llvm::ConstantInt::get(iConst, 1, true);
        llvm::Value* result = llvm::BinaryOperator::CreateAdd(inc, rhs, "",
		irgen->IRGenerator::GetBasicBlock());
        new llvm::StoreInst(result, addr,
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "--") == 0 ) {
        //Prefix decrement
        llvm::Type* iConst = irgen->IRGenerator::GetIntType();
        llvm::Value* dec = llvm::ConstantInt::get(iConst, 1, true);
        llvm::Value* result = llvm::BinaryOperator::CreateSub(rhs, dec, "",
                irgen->IRGenerator::GetBasicBlock());
        new llvm::StoreInst(result, addr,
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "+") == 0 ) {
        //Pos?
        return rhs;
      } else if( strcmp(oper, "-") == 0 ) {
        //Neg
        llvm::Value* result = llvm::BinaryOperator::CreateNeg(rhs, "",
                irgen->IRGenerator::GetBasicBlock());
        new llvm::StoreInst(result, addr,
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
        new llvm::StoreInst(result, addr,
		irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "--") == 0 ) {
        //prefix dec
        llvm::Value* result = llvm::BinaryOperator::CreateFSub(vect, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
        new llvm::StoreInst(result, addr,
                irgen->IRGenerator::GetBasicBlock());
        return result;
      } else if( strcmp(oper, "+") == 0 ) {
        //do nothing
        return rhs;
      } else if( strcmp(oper, "-") == 0 ) {
        //negate
        llvm::Value* result = llvm::BinaryOperator::CreateFNeg(rhs, "",
		irgen->IRGenerator::GetBasicBlock());
        new llvm::StoreInst(result, addr, 
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
    if( DEBUG ) {
      printf("Arithmetic\n");
    }
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
        //VarExpr* rightV = dynamic_cast<VarExpr*>(right);
        //llvm::Value* addr = rightV->EmitAddress();
        for( int i = 0; i < vec->getNumElements(); ++i ) {
          //llvm::Value* baseAddr = new llvm::LoadInst(addr, "",
	//	irgen->IRGenerator::GetBasicBlock());
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
        //VarExpr* leftV = dynamic_cast<VarExpr*>(left);
        //llvm::Value* addr = leftV->EmitAddress();
        for( int i = 0; i < vec->getNumElements(); ++i ) {
          //llvm::Value* baseAddr = new llvm::LoadInst(addr, "",
          //      irgen->IRGenerator::GetBasicBlock());
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
  if( DEBUG ) {
    printf("Relational\n");
  }
  llvm::Value* lhs = left->Emit();
  llvm::Value* rhs = right->Emit();
  llvm::Type* lType = lhs->getType();
  llvm::Type* rType = rhs->getType();
  char* oper = op->getOp();
  if( lType->isFloatTy() ) {
    //lhs is float
    llvm::CmpInst::OtherOps llvmOP = llvm::CmpInst::FCmp;
    llvm::CmpInst::Predicate pred;
    if( strcmp(oper, ">") == 0 ) {
      //Is less greater than operation
      pred = llvm::CmpInst::FCMP_OGT;
    } else if( strcmp(oper, "<") == 0 ) {
      //Is less than operation
      pred = llvm::CmpInst::FCMP_OLT;
    } else if( strcmp(oper, ">=") == 0 ) {
      //Is greater than or equal operation
      pred = llvm::CmpInst::FCMP_OGE;
    } else if( strcmp(oper, "<=") == 0 ) {
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
  if( DEBUG ) {
    printf("Equality\n");
  }
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
  if( DEBUG ) {
    printf("Assign\n");
  }
  llvm::Value* lhsAddr;
  llvm::Value* lhs;
  const char* cSwiz = "";
  if( VarExpr* leftV = dynamic_cast<VarExpr*>(left) ) {;
    lhsAddr = leftV->EmitAddress();
  } else if( FieldAccess *f = dynamic_cast<FieldAccess*>(left)) {
    lhsAddr = f->EmitAddress();
    cSwiz = f->getId()->getName();
  } else {
    if( DEBUG ) printf("assign expr not var or field\n");
    lhsAddr = right->Emit();
  }
  
  llvm::Value* rhs = right->Emit();
  if( llvm::StoreInst* si = dynamic_cast<llvm::StoreInst*>(rhs) ) {
    rhs = si->getValueOperand();
  }
  llvm::Type* lType;
  llvm::Type* rType = rhs->getType();
  char* oper = op->getOp();
  if( strcmp(oper, "=") == 0 ) {
    //normal assign
    if( strlen(cSwiz) != 0 ) {
      //Is field assignment
      llvm::Value* baseAddr = new llvm::LoadInst(lhsAddr, "",
		irgen->IRGenerator::GetBasicBlock());
      llvm::Constant* vecId;
      if( rType->isVectorTy() ) {
        //Assigning vector to a vector
        for( int i = 0; i < strlen(cSwiz); i++ ) {
          char c = cSwiz[i];
          if( c == 'x' ) {
            vecId = llvm::ConstantInt::get(
		irgen->IRGenerator::GetIntType(), 0);
          } else if( c == 'y' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 1);
          } else if( c == 'z' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 2);
          } else {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 3);
          }
          llvm::Constant* extPos = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), i);
          llvm::Value* ext = llvm::ExtractElementInst::Create(rhs, extPos, "",
		irgen->IRGenerator::GetBasicBlock());
          baseAddr = llvm::InsertElementInst::Create(baseAddr, ext, vecId, "",
		irgen->IRGenerator::GetBasicBlock());
        }
      } else if( rType->isFloatTy() ) {
        //Assigning float to a vector
        for( unsigned i = 0; i < strlen(cSwiz); i++ ) {
          char c = cSwiz[i];
          if( c == 'x' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 0);
          } else if( c == 'y' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 1);
          } else if( c == 'z' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 2);
          } else {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 3);
          }
          baseAddr = llvm::InsertElementInst::Create(baseAddr, rhs, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
        }
      }
      llvm::Value* result = new llvm::StoreInst(baseAddr, lhsAddr, "",
		irgen->IRGenerator::GetBasicBlock());
      //TODO: check if works
      return rhs;
      //return result;
    }
    llvm::Value* result = new llvm::StoreInst(rhs, lhsAddr,
	irgen->IRGenerator::GetBasicBlock());
    return rhs;
    return result;
  } else if( strcmp(oper, "+=") == 0 ) {
    //plus equals
    if( strlen(cSwiz) != 0 ) {
      //Is field assignment
      llvm::Value* baseAddr = new llvm::LoadInst(lhsAddr, "",
                irgen->IRGenerator::GetBasicBlock());
      llvm::Constant* vecId;
      if( rType->isVectorTy() ) {
        //Assigning vector to a vector
        for( int i = 0; i < strlen(cSwiz); i++ ) {
          char c = cSwiz[i];
          if( c == 'x' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 0);
          } else if( c == 'y' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 1);
          } else if( c == 'z' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 2);
          } else {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 3);
          }
          llvm::Constant* extPos = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), i);
          llvm::Value* extR = llvm::ExtractElementInst::Create(rhs, extPos, "",
                irgen->IRGenerator::GetBasicBlock());
          llvm::Value* extL = llvm::ExtractElementInst::Create(baseAddr, vecId, 
		"", irgen->IRGenerator::GetBasicBlock());
          llvm::Value* binOp = llvm::BinaryOperator::CreateFAdd(extL, extR, "",
		irgen->IRGenerator::GetBasicBlock()); 
          baseAddr = llvm::InsertElementInst::Create(baseAddr, binOp, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
        }
      } else if( rType->isFloatTy() ) {
        //Assigning float to a vector
        for( unsigned i = 0; i < strlen(cSwiz); i++ ) {
          char c = cSwiz[i];
          if( c == 'x' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 0);
          } else if( c == 'y' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 1);
          } else if( c == 'z' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 2);
          } else {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 3);
          }
          llvm::Value* extL = llvm::ExtractElementInst::Create(baseAddr, vecId,
		"", irgen->IRGenerator::GetBasicBlock());
          llvm::Value* binOp = llvm::BinaryOperator::CreateFAdd(extL, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
          baseAddr = llvm::InsertElementInst::Create(baseAddr, binOp, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
        }

        llvm::Value* result = new llvm::StoreInst(baseAddr, lhsAddr, "",
                irgen->IRGenerator::GetBasicBlock());
        return rhs;
      }
    }
    lhs = left->Emit();
    lType = lhs->getType();
    if( lType->isFloatTy() || lType->isVectorTy() ) {
      //lhs is float or vec2/3/4
      llvm::Value* result = llvm::BinaryOperator::CreateFAdd(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      new llvm::StoreInst(result, lhsAddr, irgen->IRGenerator::GetBasicBlock());
      return result;
    } else if( lType->isIntegerTy() ) {
      //lhs is integer
      llvm::Value* result = llvm::BinaryOperator::CreateAdd(lhs, rhs, "", 
	irgen->IRGenerator::GetBasicBlock());
      new llvm::StoreInst(result, lhsAddr, irgen->IRGenerator::GetBasicBlock());
      return result;
    }
  } else if( strcmp(oper, "-=") == 0 ) {
    //minus equals
    if( strlen(cSwiz) != 0 ) {
      //Is field assignment
      llvm::Value* baseAddr = new llvm::LoadInst(lhsAddr, "",
                irgen->IRGenerator::GetBasicBlock());
      llvm::Constant* vecId;
      if( rType->isVectorTy() ) {
        //Assigning vector to a vector
        for( int i = 0; i < strlen(cSwiz); i++ ) {
          char c = cSwiz[i];
          if( c == 'x' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 0);
          } else if( c == 'y' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 1);
          } else if( c == 'z' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 2);
          } else {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 3);
          }
          llvm::Constant* extPos = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), i);
          llvm::Value* extR = llvm::ExtractElementInst::Create(rhs, extPos, "",
                irgen->IRGenerator::GetBasicBlock());
          llvm::Value* extL = llvm::ExtractElementInst::Create(baseAddr, vecId,
                "", irgen->IRGenerator::GetBasicBlock());
          llvm::Value* binOp = llvm::BinaryOperator::CreateFSub(extL, extR, "",
                irgen->IRGenerator::GetBasicBlock());
          baseAddr = llvm::InsertElementInst::Create(baseAddr, binOp, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
        }
      } else if( rType->isFloatTy() ) {
        //Assigning float to a vector
        for( unsigned i = 0; i < strlen(cSwiz); i++ ) {
          char c = cSwiz[i];
          if( c == 'x' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 0);
          } else if( c == 'y' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 1);
          } else if( c == 'z' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 2);
          } else {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 3);
          }
          llvm::Value* extL = llvm::ExtractElementInst::Create(baseAddr, vecId,
                "", irgen->IRGenerator::GetBasicBlock());
          llvm::Value* binOp = llvm::BinaryOperator::CreateFSub(extL, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
          baseAddr = llvm::InsertElementInst::Create(baseAddr, binOp, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
        }
        llvm::Value* result = new llvm::StoreInst(baseAddr, lhsAddr, "",
                irgen->IRGenerator::GetBasicBlock());
        return rhs;
      }
    }
    lhs = left->Emit();
    lType = lhs->getType();
    if( lType->isFloatTy() || lType->isVectorTy() ) {
      //lhs is float or vec2/3/4
      llvm::Value* result = llvm::BinaryOperator::CreateFSub(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      new llvm::StoreInst(result, lhsAddr, irgen->IRGenerator::GetBasicBlock());
      return result;
    } else if( lType->isIntegerTy() ) {
      //lhs is integer
      llvm::Value* result = llvm::BinaryOperator::CreateSub(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      new llvm::StoreInst(result, lhsAddr, irgen->IRGenerator::GetBasicBlock());
      return result;
    }
  } else if( strcmp(oper, "*=") == 0 ) {
    //multipy equals
    if( strlen(cSwiz) != 0 ) {
      //Is field assignment
      llvm::Value* baseAddr = new llvm::LoadInst(lhsAddr, "",
                irgen->IRGenerator::GetBasicBlock());
      llvm::Constant* vecId;
      if( rType->isVectorTy() ) {
        //Assigning vector to a vector
        for( int i = 0; i < strlen(cSwiz); i++ ) {
          char c = cSwiz[i];
          if( c == 'x' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 0);
          } else if( c == 'y' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 1);
          } else if( c == 'z' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 2);
          } else {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 3);
          }
          llvm::Constant* extPos = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), i);
          llvm::Value* extR = llvm::ExtractElementInst::Create(rhs, extPos, "",
                irgen->IRGenerator::GetBasicBlock());
          llvm::Value* extL = llvm::ExtractElementInst::Create(baseAddr, vecId,
                "", irgen->IRGenerator::GetBasicBlock());
          llvm::Value* binOp = llvm::BinaryOperator::CreateFMul(extL, extR, "",
                irgen->IRGenerator::GetBasicBlock());
          baseAddr = llvm::InsertElementInst::Create(baseAddr, binOp, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
        }
      } else if( rType->isFloatTy() ) {
        //Assigning float to a vector
        for( unsigned i = 0; i < strlen(cSwiz); i++ ) {
          char c = cSwiz[i];
          if( c == 'x' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 0);
          } else if( c == 'y' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 1);
          } else if( c == 'z' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 2);
          } else {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 3);
          }
          llvm::Value* extL = llvm::ExtractElementInst::Create(baseAddr, vecId,
                "", irgen->IRGenerator::GetBasicBlock());
          llvm::Value* binOp = llvm::BinaryOperator::CreateFMul(extL, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
          baseAddr = llvm::InsertElementInst::Create(baseAddr, binOp, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
        }
        llvm::Value* result = new llvm::StoreInst(baseAddr, lhsAddr, "",
                irgen->IRGenerator::GetBasicBlock());
        return rhs;
      }
    }
    lhs = left->Emit();
    lType = lhs->getType();
    if( lType->isFloatTy() || lType->isVectorTy() ) {
      //lhs is float or vec2/3/4
      llvm::Value* result = llvm::BinaryOperator::CreateFMul(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      new llvm::StoreInst(result, lhsAddr, irgen->IRGenerator::GetBasicBlock());
      return result;
    } else if( lType->isIntegerTy() ) {
      //lhs is integer
      llvm::Value* result = llvm::BinaryOperator::CreateMul(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      new llvm::StoreInst(result, lhsAddr, irgen->IRGenerator::GetBasicBlock());
      return result;
    }
  } else if( strcmp(oper, "/=") == 0 ) {
    //divide equals
    if( strlen(cSwiz) != 0 ) {
      //Is field assignment
      llvm::Value* baseAddr = new llvm::LoadInst(lhsAddr, "",
                irgen->IRGenerator::GetBasicBlock());
      llvm::Constant* vecId;
      if( rType->isVectorTy() ) {
        //Assigning vector to a vector
        for( int i = 0; i < strlen(cSwiz); i++ ) {
          char c = cSwiz[i];
          if( c == 'x' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 0);
          } else if( c == 'y' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 1);
          } else if( c == 'z' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 2);
          } else {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 3);
          }
          llvm::Constant* extPos = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), i);
          llvm::Value* extR = llvm::ExtractElementInst::Create(rhs, extPos, "",
                irgen->IRGenerator::GetBasicBlock());
          llvm::Value* extL = llvm::ExtractElementInst::Create(baseAddr, vecId,
                "", irgen->IRGenerator::GetBasicBlock());
          llvm::Value* binOp = llvm::BinaryOperator::CreateFDiv(extL, extR, "",
                irgen->IRGenerator::GetBasicBlock());
          baseAddr = llvm::InsertElementInst::Create(baseAddr, binOp, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
        }
      } else if( rType->isFloatTy() ) {
        //Assigning float to a vector
        for( unsigned i = 0; i < strlen(cSwiz); i++ ) {
          char c = cSwiz[i];
          if( c == 'x' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 0);
          } else if( c == 'y' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 1);
          } else if( c == 'z' ) {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 2);
          } else {
            vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 3);
          }
          llvm::Value* extL = llvm::ExtractElementInst::Create(baseAddr, vecId,
                "", irgen->IRGenerator::GetBasicBlock());
          llvm::Value* binOp = llvm::BinaryOperator::CreateFDiv(extL, rhs, "",
                irgen->IRGenerator::GetBasicBlock());
          baseAddr = llvm::InsertElementInst::Create(baseAddr, binOp, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
        }
        llvm::Value* result = new llvm::StoreInst(baseAddr, lhsAddr, "",
                irgen->IRGenerator::GetBasicBlock());
        return rhs;
      }
    }
    lhs = left->Emit();
    lType = lhs->getType();
    if( lType->isFloatTy() || lType->isVectorTy() ) {
      //lhs is float or vec2/3/4
      llvm::Value* result = llvm::BinaryOperator::CreateFDiv(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      new llvm::StoreInst(result, lhsAddr, irgen->IRGenerator::GetBasicBlock());
      return result;   
    } else if( lType->isIntegerTy() ) {
      //lhs is integer
      llvm::Value* result = llvm::BinaryOperator::CreateSDiv(lhs, rhs, "",
        irgen->IRGenerator::GetBasicBlock());
      new llvm::StoreInst(result, lhsAddr, irgen->IRGenerator::GetBasicBlock());
      return result;
    }
  } else {
    //should't be here

  }

  return NULL;
}

llvm::Value* PostfixExpr::Emit() {
  if( DEBUG ) {
    printf("Postfix\n");
  }
  llvm::Value* lhs = left->Emit();
  llvm::Value* addr;
  const char* cSwiz = "";
  if( VarExpr* leftV = dynamic_cast<VarExpr*>(left) ) {
    addr = leftV->EmitAddress();
  } else if( FieldAccess* f = dynamic_cast<FieldAccess*>(left) ) {
    addr = f->EmitAddress();
    cSwiz = f->getId()->getName();
  } else {
    if( DEBUG ) printf("postfix address not var or field\n");
    addr = left->Emit();
  }
  
  llvm::Type* lType = lhs->getType();
  char* oper = op->getOp();
  if( strlen(cSwiz) != 0 ) {
    //field assignment
    llvm::Constant* inc = llvm::ConstantFP::get(
		irgen->IRGenerator::GetFloatType(), 1.0);
    llvm::Value* baseAddr = new llvm::LoadInst(addr, "",
		irgen->IRGenerator::GetBasicBlock());
    llvm::Value* baseAddr1 = baseAddr;
    llvm::Constant* vecId;
    for( unsigned i = 0; i < strlen(cSwiz); i++ ) {
      char c = cSwiz[i];
      if( c == 'x' ) {
        vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 0);
      } else if( c == 'y' ) {
        vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 1);
      } else if( c == 'z' ) {
        vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 2);
      } else {
        vecId = llvm::ConstantInt::get(
                irgen->IRGenerator::GetIntType(), 3);
      }
      llvm::Value* ext = llvm::ExtractElementInst::Create(baseAddr, vecId, "",
		irgen->IRGenerator::GetBasicBlock());
      if( strcmp(oper, "++") == 0 ) {
        llvm::Value* result = llvm::BinaryOperator::CreateFAdd(ext, inc, "",
		irgen->IRGenerator::GetBasicBlock());
        baseAddr = llvm::InsertElementInst::Create(baseAddr, result, vecId, "",
		irgen->IRGenerator::GetBasicBlock());
      } else if( strcmp(oper, "--") == 0 ) {
        llvm::Value* result = llvm::BinaryOperator::CreateFSub(ext, inc, "",
                irgen->IRGenerator::GetBasicBlock());
        baseAddr = llvm::InsertElementInst::Create(baseAddr, result, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
      } else {
        //shouldnt be here
      } 
    }
    llvm::Value* res = new llvm::StoreInst(baseAddr, addr, "",
		irgen->IRGenerator::GetBasicBlock());
    return baseAddr1;
  }
  if( lType->isVectorTy() ) {
    llvm::VectorType* vec = (llvm::VectorType*) lType;
    llvm::Value* ret = lhs;
    llvm::Type* fConst = irgen->IRGenerator::GetFloatType();
    llvm::Value* inc = llvm::ConstantFP::get(fConst, 1.0);
    for( int i = 0; i < vec->getNumElements(); i++ ) {
      llvm::Constant* vecId = llvm::ConstantInt::get( 
		irgen->IRGenerator::GetIntType(), i);
      llvm::Value* val = llvm::ExtractElementInst::Create(lhs, vecId, "",
		irgen->IRGenerator::GetBasicBlock());
      if( strcmp(oper, "++") == 0 ) {
        llvm::Value* result = llvm::BinaryOperator::CreateFAdd(val, inc, "",
		irgen->IRGenerator::GetBasicBlock());
        lhs = llvm::InsertElementInst::Create(lhs, result, vecId, "", 
		irgen->IRGenerator::GetBasicBlock());
      } else if( strcmp(oper, "--") == 0 ) {
        llvm::Value* result = llvm::BinaryOperator::CreateFSub(val, inc, "",
                irgen->IRGenerator::GetBasicBlock());
        lhs = llvm::InsertElementInst::Create(lhs, result, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
      }
      
    } 
    llvm::Value* res = new llvm::StoreInst(lhs, addr, "",
		irgen->IRGenerator::GetBasicBlock());
    return ret;
  } else if( lType->isFloatTy() ) {
    if( strcmp(oper, "++") == 0 ) {
      //Postfix inc
      llvm::Type* fConst = irgen->IRGenerator::GetFloatType();
      llvm::Value* inc = llvm::ConstantFP::get(fConst, 1.0);
      llvm::Value* result = llvm::BinaryOperator::CreateFAdd(lhs, inc, "",
                irgen->IRGenerator::GetBasicBlock());
      new llvm::StoreInst(result, addr, 
		irgen->IRGenerator::GetBasicBlock());
      return lhs;  
    } else if( strcmp(oper, "--") == 0 ) {
      //Postfix dec
      llvm::Type* fConst = irgen->IRGenerator::GetFloatType();
      llvm::Value* dec = llvm::ConstantFP::get(fConst, 1.0);
      llvm::Value* result = llvm::BinaryOperator::CreateFSub(lhs, dec, "",
                irgen->IRGenerator::GetBasicBlock());
      new llvm::StoreInst(result, addr,
                irgen->IRGenerator::GetBasicBlock());
      return lhs;
    } else if( strcmp(oper, ".") == 0 ) {
      //Field Selection??
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
      new llvm::StoreInst(result, addr,
                irgen->IRGenerator::GetBasicBlock());
      return lhs;
    } else if( strcmp(oper, "--") == 0 ) {
      //Postfix dec
      llvm::Type* iConst = irgen->IRGenerator::GetIntType();
      llvm::Value* dec = llvm::ConstantInt::get(iConst, 1, true);
      llvm::Value* result = llvm::BinaryOperator::CreateSub(lhs, dec, "",
                irgen->IRGenerator::GetBasicBlock());
      new llvm::StoreInst(result, addr,
                irgen->IRGenerator::GetBasicBlock());
      return lhs;
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
  if( DEBUG ) {
    printf("FieldAccess\n");
  }
  llvm::Value* lhs = base->Emit();
  const char* swizC = field->getName();
  std::vector<llvm::Constant*> indices;
  int len = strlen(swizC);
  if( len == 1 ) {
    char c = swizC[0];
    if( c == 'x' ) {
      //first element
      llvm::Constant* vecId =
		llvm::ConstantInt::get(irgen->IRGenerator::GetIntType(), 0);
      llvm::Value* result = llvm::ExtractElementInst::Create(lhs, vecId, "",
		irgen->IRGenerator::GetBasicBlock());
      return result;
    } else if( c == 'y' ) {
      //second element
      llvm::Constant* vecId =
                llvm::ConstantInt::get(irgen->IRGenerator::GetIntType(), 1);
      llvm::Value* result = llvm::ExtractElementInst::Create(lhs, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
      return result;
    } else if( c == 'z' ) {
      //third element
      llvm::Constant* vecId =
                llvm::ConstantInt::get(irgen->IRGenerator::GetIntType(), 2);
      llvm::Value* result = llvm::ExtractElementInst::Create(lhs, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
      return result;
    } else {
      //fourth element
      llvm::Constant* vecId =
                llvm::ConstantInt::get(irgen->IRGenerator::GetIntType(), 3);
      llvm::Value* result = llvm::ExtractElementInst::Create(lhs, vecId, "",
                irgen->IRGenerator::GetBasicBlock());
      return result;
    }
  } else {
    for(int i = 0; i < len; ++i) {
      char c = swizC[i];
      if( c == 'x' ) {
        //first element
        llvm::Constant* vecId =
		llvm::ConstantInt::get(irgen->IRGenerator::GetIntType(), 0);
        indices.push_back(vecId);
      } else if( c == 'y' ) {
        //second element
        llvm::Constant* vecId =
                llvm::ConstantInt::get(irgen->IRGenerator::GetIntType(), 1);
        indices.push_back(vecId);
      } else if( c == 'z' ) {
        //third element    
        llvm::Constant* vecId =
                llvm::ConstantInt::get(irgen->IRGenerator::GetIntType(), 2);
        indices.push_back(vecId);
      } else {
        //fourth element
        llvm::Constant* vecId =
                llvm::ConstantInt::get(irgen->IRGenerator::GetIntType(), 3);
        indices.push_back(vecId);
      }
    }
    llvm::ConstantVector* mask = 
		(llvm::ConstantVector *)llvm::ConstantVector::get(indices);
    llvm::Value* result = new llvm::ShuffleVectorInst(lhs, llvm::UndefValue::get(
	lhs->getType()), mask, "", irgen->IRGenerator::GetBasicBlock());
    return result;
  }
}

llvm::Value* FieldAccess::EmitAddress() {
  if( DEBUG ) {
    printf("Field Access EmitAddress\n");
  }
  const char *c = field->getName();
  if( VarExpr* varE = dynamic_cast<VarExpr*>(base) ) {
    return varE->EmitAddress();
  } else if( FieldAccess* f = dynamic_cast<FieldAccess*>(base) ) {
    return f->EmitAddress();
  } else {
    if( DEBUG ) printf("fieldaccess not var or field\n");
    return base->EmitAddress();
  }
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
 
