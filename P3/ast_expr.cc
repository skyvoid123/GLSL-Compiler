/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "errors.h"

Type* Expr::Check(Symtab* S) {
  if(FieldAccess *f = dynamic_cast<FieldAccess*>(this)) {
    f->Check(S);
  }
  return NULL;
}
Type* ExprError::Check(Symtab* S) {
  return Type::errorType;
}

Type* EmptyExpr::Check(Symtab* S) {
  return NULL;
}

Type* IntConstant::Check(Symtab* S) {
  return Type::intType;
}

Type* FloatConstant::Check(Symtab* S) {
  return Type::floatType;
}

Type* BoolConstant::Check(Symtab* S) {
  return Type::boolType;
}

Type* VarExpr::Check(Symtab* S) {
  Decl* findMe = new VarDecl(id, Type::intType);
  return S->findType(findMe);
}

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}

void IntConstant::PrintChildren(int indentLevel) { 
    printf("%d", value);
}

FloatConstant::FloatConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}
void FloatConstant::PrintChildren(int indentLevel) { 
    printf("%g", value);
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

void VarExpr::PrintChildren(int indentLevel) {
    id->Print(indentLevel+1);
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}

char* Operator::getOp() {
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
 
Type* ArithmeticExpr::Check(Symtab *S) {
  Type* t1;
  if( left == NULL ) {
    t1 = NULL;
  } else {
    t1 = left->Check(S);
  }
  Type* t2 = right->Check(S);
  char* oper = op->getOp();
  // Check unary expr first to avoid segfaults from t1 being NULL
  if( t1 == NULL && t2 != NULL) {
    if(strcmp(oper, "++") == 0 || strcmp(oper,"--") == 0
                || strcmp(oper,"-") == 0 || strcmp(oper, "+") == 0) {
      if( t2->IsEquivalentTo(Type::intType) ) {
        return Type::intType;
      } else if( t2->IsEquivalentTo(Type::floatType) ) {
        return Type::floatType;
      } else if( t2->IsEquivalentTo(Type::vec2Type) ) {
         return Type::vec2Type;
      } else if( t2->IsEquivalentTo(Type::vec3Type) ) {
         return Type::vec3Type;
      } else if( t2->IsEquivalentTo(Type::vec4Type) ) {
         return Type::vec4Type;
      } else if( t2->IsEquivalentTo(Type::mat2Type) ) {
         return Type::mat2Type;
      } else if( t2->IsEquivalentTo(Type::mat3Type) ) {
         return Type::mat3Type;
      } else if( t2->IsEquivalentTo(Type::mat4Type) ) {
         return Type::mat4Type;
      } else {
         ReportError::IncompatibleOperand(op, t2);
         return Type::errorType;
      }
    } else {
      ReportError::IncompatibleOperand(op, t2);
      return Type::errorType;
    }
  }
  // Check for error type, avoid kaskades
  if( t1->IsEquivalentTo(Type::errorType) || 
		t2->IsEquivalentTo(Type::errorType) ) {
    return Type::errorType;
  }
  // Check compatibility
  if( !t1->IsEquivalentTo(t2) ) {
    printf("not equiv\n");
    // float compatible with vec and mat, return the vec/mat
    if( t1->IsEquivalentTo(Type::floatType)) {
      if( t2->IsEquivalentTo(Type::vec2Type) ) {
        return Type::vec2Type;
      } else if( t2->IsEquivalentTo(Type::vec3Type) ) {
        return Type::vec3Type;
      } else if( t2->IsEquivalentTo(Type::vec4Type) ) {
        return Type::vec4Type;
      } else if( t2->IsEquivalentTo(Type::mat2Type) ) {
        return Type::mat2Type;
      } else if( t2->IsEquivalentTo(Type::mat3Type) ) { 
        return Type::mat3Type;
      } else if( t2->IsEquivalentTo(Type::mat4Type) ) {
        return Type::mat4Type;
      } else {
        ReportError::IncompatibleOperands(op, t1, t2);
        return Type::errorType;
      }

    } else if( t2->IsEquivalentTo(Type::floatType)) {
      printf("t2 is float\n");
      if( t1->IsEquivalentTo(Type::vec2Type) ) {
        return Type::vec2Type;
      } else if( t1->IsEquivalentTo(Type::vec3Type) ) {
        return Type::vec3Type;
      } else if( t1->IsEquivalentTo(Type::vec4Type) ) {
        return Type::vec4Type;
      } else if( t1->IsEquivalentTo(Type::mat2Type) ) {
        return Type::mat2Type;
      } else if( t1->IsEquivalentTo(Type::mat3Type) ) {
        return Type::mat3Type;
      } else if( t1->IsEquivalentTo(Type::mat4Type) ) {
        return Type::mat4Type;
      } else {
        ReportError::IncompatibleOperands(op, t1, t2);
        return Type::errorType;
      }
    // If op is *, then can vec can match with mat of equal size, returns vec
    } else if( strcmp(oper, "*")  == 0 && ((t1->IsEquivalentTo(Type::vec2Type) &&
	t2->IsEquivalentTo(Type::mat2Type)) || (t1->IsEquivalentTo(Type::mat2Type) && t2->IsEquivalentTo(Type::vec2Type))) ) {
      return Type::vec2Type;
    } else if( strcmp(oper, "*")  == 0 && ((t1->IsEquivalentTo(Type::vec3Type) &
	t2->IsEquivalentTo(Type::mat3Type)) || (t1->IsEquivalentTo(Type::mat3Type) && t2->IsEquivalentTo(Type::vec3Type))) ) {
      return Type::vec3Type; 
    } else if( strcmp(oper, "*")  == 0 && ((t1->IsEquivalentTo(Type::vec4Type) & 
	t2->IsEquivalentTo(Type::mat4Type)) || (t1->IsEquivalentTo(Type::mat4Type) && t2->IsEquivalentTo(Type::vec4Type))) ) {
      return Type::vec4Type;
    } else {
      ReportError::IncompatibleOperands(op, t1, t2);
      return Type::errorType;
    }
  } else if( t1->IsEquivalentTo(Type::boolType) ) {
    //types cant be bool
    ReportError::IncompatibleOperands(op, t1, t2);
    return Type::errorType;
  } else if( t1->IsEquivalentTo(Type::voidType) ) {
    //types can't be void
    ReportError::IncompatibleOperands(op, t1, t2);
    return Type::errorType;

  }
  return t1;
}

Type* RelationalExpr::Check(Symtab *S) {
  Type* t1 = left->Check(S);
  Type* t2 = right->Check(S);
  // Check for error type, avoid kaskades
  if( t1->IsEquivalentTo(Type::errorType) || 
		t2->IsEquivalentTo(Type::errorType) ) {
    return Type::errorType;
  }
  // Relational can be int int or float float, returns a bool Type
  if( t1->IsEquivalentTo(Type::intType) && t2->IsEquivalentTo(Type::intType) ) {

  } else if( t1->IsEquivalentTo(Type::floatType) && 
		t2->IsEquivalentTo(Type::floatType) ) {
 
  } else {
    ReportError::IncompatibleOperands(op, t1, t2);
    return Type::errorType;
  }
    return Type::boolType;
}
  
Type* EqualityExpr::Check(Symtab *S) {
  Type* t1 = left->Check(S);
  Type* t2 = right->Check(S);
  // Check for error type, avoid kaskades
  if( t1->IsEquivalentTo(Type::errorType) || 
		t2->IsEquivalentTo(Type::errorType) ) {
    return Type::errorType;
  }
  // Equality expr takes two of the same type, returns a bool
  if( t1->IsEquivalentTo(t2) ) {
    return Type::boolType;
  } else {
    ReportError::IncompatibleOperands(op, t1, t2);
    return Type::errorType;
  }
}

Type* LogicalExpr::Check(Symtab *S) {
  Type* t1 = left->Check(S);
  Type* t2 = right->Check(S);
  // Check for error type, avoid kaskades
  if( t1->IsEquivalentTo(Type::errorType) || 
		t2->IsEquivalentTo(Type::errorType) ) {
    return Type::errorType;
  }
  // Logical expr takes two bools and returns a bool
  if( t1->IsEquivalentTo(Type::boolType) && t2->IsEquivalentTo(Type::boolType)) {
    return Type::boolType;
  } else {
    ReportError::IncompatibleOperands(op, t1, t2);
    return Type::errorType;
  }
}
Type* AssignExpr::Check(Symtab *S) {
  Type* t1 = left->Check(S);
  Type* t2 = right->Check(S);
  // Check for error type, avoid kaskades
  if( t1->IsEquivalentTo(Type::errorType) || 
		t2->IsEquivalentTo(Type::errorType) ) {
    return Type::errorType;
  }
  // Assign expr takes two equal types and returns that type
  if( t1->IsEquivalentTo(t2)) {
    return t1;
  } else {
    ReportError::IncompatibleOperands(op, t1, t2);
    return Type::errorType;
  }
}

Type* PostfixExpr::Check(Symtab *S) {
  Type* t1 = left->Check(S);
  // Check for error type, avoid kaskades
  if( t1->IsEquivalentTo(Type::errorType) ) {
    return Type::errorType;
  }
  // Postfix expr can be int, float, vec, or mat, returns same type
  if( t1->IsEquivalentTo(Type::intType) ) {
    return Type::intType;
  } else if( t1->IsEquivalentTo(Type::floatType) ) {
    return Type::floatType;
  } else if( t1->IsEquivalentTo(Type::vec2Type) ) { 
    return Type::vec2Type;
  } else if ( t1->IsEquivalentTo(Type::vec3Type) ) {
     return Type::vec3Type;
  } else if ( t1->IsEquivalentTo(Type::vec4Type) ) {
     return Type::vec4Type;
  } else if ( t1->IsEquivalentTo(Type::mat2Type) ) {
     return Type::mat2Type;
  } else if ( t1->IsEquivalentTo(Type::mat3Type) ) {
     return Type::mat3Type;
  } else if ( t1->IsEquivalentTo(Type::mat4Type) ) {
     return Type::mat4Type;
  } else {
    ReportError::IncompatibleOperand(op, t1);
    return Type::errorType;
  }
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


Type* FieldAccess::Check(Symtab* S) {
  Type* t1 = base->Check(S);
  // I made a char* and string, just cuz
  const char* swizC = field->getName();
  std::string swiz(field->getName());
  // Check for error type, avoid kaskades
  if( t1->IsEquivalentTo(Type::errorType) ) {
    return Type::errorType;
  }
  // Field access accepts vec2, vec3, and vec4 
  if( !t1->IsEquivalentTo(Type::vec2Type) && !t1->IsEquivalentTo(Type::vec3Type)
	 && !t1->IsEquivalentTo(Type::vec4Type) ) {
    ReportError::InaccessibleSwizzle(field, base);
    return Type::errorType;
  }
  // Check if swizzle contains anything besides xyzw
  int i; 
  for( i = 0; i < swiz.size(); i++ ) {
    if( swiz[i] != 'x' && swiz[i] != 'y' && swiz[i] != 'z' && swiz[i] != 'w' ) {
      ReportError::InvalidSwizzle(field, base);
      return Type::errorType;
    }
  }
  // Check if swizzle is within the proper scope of its vector
  if( t1->IsEquivalentTo(Type::vec2Type) ) {
    if( strchr( swizC, 'z' ) != NULL || strchr( swizC, 'w' ) != NULL) {
      ReportError::SwizzleOutOfBound(field, base);
      return Type::errorType;
    }
  } else if( t1->IsEquivalentTo(Type::vec3Type) ) {
    if( strchr( swizC, 'w' ) != NULL ) {
      ReportError::SwizzleOutOfBound(field, base);
      return Type::errorType;
    }
  }
  // Check if swizzle is too long, over 4 values
  if( swiz.length() > 4 ) {
    ReportError::OversizedVector(field, base);
    return Type::errorType;
  }
  // Otherwise, swizzle is properly formed, return vector of swizzle length
  if( swiz.length() == 1 ) {
    return Type::floatType;
  } else if( swiz.length() == 2 ) {
    return Type::vec2Type;
  } else if( swiz.length() == 3 ) {
    return Type::vec3Type;
  } else if( swiz.length() == 4 ) {
    return Type::vec4Type;
  } else {
    printf("shouldnt be here\n From, field access\n");
    return NULL;
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
 
