/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"


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

Type CompoundExpr::PrintChildren(int indentLevel) {
   if (left) left->Print(indentLevel+1);
   op->Print(indentLevel+1);
   if (right) right->Print(indentLevel+1);
}
 
Type ArithmeticExpr::Check(Symtab *S) {
  //void* cmp = dynamic_cast<void*>(right);
  //cmp = dynamic_cast<void*>(left);
  //if(cmp == 0) {
  //    ReportError::IncompatibleOperands(op, left->getType(), right->getType());
  //}
  // Get left and right types
  Type t1 = left->getType();
  Type t2 = right->getType();
  const char t1Type;
  const char t2Type;
  // Check compatibility
  if( t1Type != t2Type ) {
    // float compatible with vec and mat
    if( t1Type == "float") {
      if( t2Type == "vec2" ) {
        return Type::vec2Type;
      } else if( t2Type == "vec3" ) {
        return Type::vec3Type;
      } else if( t2Type == "vec4" ) {
        return Type::vec4Type;
      } else if( t2Type == "mat2" ) {
        return Type::mat2Type;
      } else if( t2Type == "mat3" ) { 
        return Type::mat3Type;
      } else if( t2Type == "mat4" ) {
        return Type::mat4Type;
      } else {
        return Type::errorType;
      }

    } else if( t2Type == "float") {
      if( t1Type == "vec2" ) {
        return Type::vec2Type;
      } else if( t1Type == "vec3" ) {
        return Type::vec3Type;
      } else if( t1Type == "vec4" ) {
        return Type::vec4Type;
      } else if( t1Type == "mat2" ) {
        return Type::mat2Type;
      } else if( t1Type == "mat3" ) {
        return Type::mat3Type;
      } else if( t1Type == "mat4" ) {
        return Type::mat4Type;
      } else {
        return Type::errorType;
      }

    } else if( (t1Type == "vec2" && t2Type == "mat2") ||
                (t1Type == "mat2" && t2Type == "vec2") ) {
      return Type::vec2Type;
    } else if( (t1Type == "vec3" && t2Type == "mat3") ||
                (t1Type == "mat3" && t2Type == "vec3") ) {
      return Type::vec3Type;
    } 
    } else if( (t1Type == "vec4" && t2Type == "mat4") || 
		(t1Type == "mat4" && t2Type == "vec4") ) {
      return Type::vec4Type;
    } else {
      ReportError::IncompatibleOperands(op, t1, t2);
      return Type::errorType;
    }
  } else if( t1Type == "bool" ) {
    ReportError::IncompatibleOperands(op, t1, t2);
    return Type::errorType;

  } else if( t1Type == "void" ) {
    ReportError::IncompatibleOperands(op, t1, t2);
    return Type::errorType;

  }
  return t1;
}

Type RelationalExpr::Check(Symtab *S) {
  //TODO: still dunno if this is rite
  //void* cmp = dynamic_cast<void*>(right);
  //cmp = dynamic_cast<void*>(left);
  //if(cmp == 0) {
  // ReportError::IncompatibleOperands(op, left, right);   
  // }
}
  
Type EqualityExpr::Check(Symtab *S) {
  //TODO: IDK!
  //void* cmp = dynamic_cast<void*>(right);
  //cmp = dynamic_cast<void*>(left);
  //if(cmp == 0) {
  //  ReportError::IncompatibleOperands(op, left, right);
  // }
}

Type LogicalExpr::Check(Symtab *S) {
  //TODO: StIlL DoNt KnOw 
  //void* cmp = dynamic_cast<void*>(right);
  //cmp = dynamic_cast<void*>(left);
  //    if(cmp == 0) {
  //        ReportError::IncompatibleOperands(op, left, right);
  //          }

}
Type AssignExpr::Check(Symtab *S) {
  //TODO: dunno if this is gonna worK
  //void* cmp = dynamic_cast<void*>(right);
  //cmp = dynamic_cast<void*>(left);
  //if(cmp == 0) {
  //  ReportError::IncompatibleOperands(op, left, right);
  //}
}

Type PostfixExpr::Check(Symtab *S) {
  //TODO: I DONT KNOW IF THIS WORKS EITHER!! OK?
  //string type = typeid(left).name();
  //if( type == "bool" || type == "void" ) {
  // ReportError::IncompatibleOperand(op, left);
  //}
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
 