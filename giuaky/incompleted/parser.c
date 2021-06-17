/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */
#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "semantics.h"
#include "error.h"
#include "debug.h"

Token *currentToken;
Token *lookAhead;

extern Type* intType;
extern Type* charType;
extern Type* doubleType;
extern Type* stringType;
extern SymTab* symtab;

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
  Object* program;

  eat(KW_PROGRAM);
  eat(TK_IDENT);

  program = createProgramObject(currentToken->string);
  enterBlock(program->progAttrs->scope);

  eat(SB_SEMICOLON);

  compileBlock();
  eat(SB_PERIOD);

  exitBlock();
}

void compileBlock(void) {
  Object* constObj;
  ConstantValue* constValue;

  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);

    do {
      eat(TK_IDENT);
      
      checkFreshIdent(currentToken->string);
      constObj = createConstantObject(currentToken->string);
      
      eat(SB_EQ);
      constValue = compileConstant();
      
      constObj->constAttrs->value = constValue;
      declareObject(constObj);
      
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock2();
  } 
  else compileBlock2();
}

void compileBlock2(void) {
  Object* typeObj;
  Type* actualType;

  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);

    do {
      eat(TK_IDENT);
      
      checkFreshIdent(currentToken->string);
      typeObj = createTypeObject(currentToken->string);
      
      eat(SB_EQ);
      actualType = compileType();
      
      typeObj->typeAttrs->actualType = actualType;
      declareObject(typeObj);
      
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock3();
  } 
  else compileBlock3();
}

void compileBlock3(void) {
  Object* varObj;
  Type* varType;

  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);

    do {
      eat(TK_IDENT);
      
      checkFreshIdent(currentToken->string);
      varObj = createVariableObject(currentToken->string);

      eat(SB_COLON);
      varType = compileType();
      
      varObj->varAttrs->type = varType;
      declareObject(varObj);
      
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock4();
  } 
  else compileBlock4();
}

void compileBlock4(void) {
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileSubDecls(void) {
  while ((lookAhead->tokenType == KW_FUNCTION) || (lookAhead->tokenType == KW_PROCEDURE)) {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else compileProcDecl();
  }
}

void compileFuncDecl(void) {
  Object* funcObj;
  Type* returnType;

  eat(KW_FUNCTION);
  eat(TK_IDENT);

  checkFreshIdent(currentToken->string);
  funcObj = createFunctionObject(currentToken->string);
  declareObject(funcObj);

  enterBlock(funcObj->funcAttrs->scope);
  
  compileParams();

  eat(SB_COLON);
  returnType = compileBasicType();
  funcObj->funcAttrs->returnType = returnType;

  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);

  exitBlock();
}

void compileProcDecl(void) {
  Object* procObj;

  eat(KW_PROCEDURE);
  eat(TK_IDENT);

  checkFreshIdent(currentToken->string);
  procObj = createProcedureObject(currentToken->string);
  declareObject(procObj);

  enterBlock(procObj->procAttrs->scope);

  compileParams();

  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);

  exitBlock();
}

ConstantValue* compileUnsignedConstant(void) {
  ConstantValue* constValue = NULL;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    constValue = makeIntConstant(currentToken->value);
    break;
  case TK_DOUBLE:
    eat(TK_DOUBLE);
    constValue = makeDoubleConstant(currentToken->value);
    break;
  case TK_IDENT:
    eat(TK_IDENT);

    obj = checkDeclaredConstant(currentToken->string);
    constValue = duplicateConstantValue(obj->constAttrs->value);

    break;
  case TK_CHAR:
    eat(TK_CHAR);
    constValue = makeCharConstant(currentToken->string[0]);
    break;
  case TK_STRING:
    eat(TK_STRING);
    constValue = makeStringConstant(currentToken->string);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

ConstantValue* compileConstant(void) {
  ConstantValue* constValue = NULL;

  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    constValue = compileConstant2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    constValue = compileConstant2();
    if (constValue->type == TP_INT) {
      constValue->intValue = - constValue->intValue;
    } else if (constValue->type == TP_DOUBLE) {
      constValue->doubleValue = - constValue->doubleValue;
    }
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    constValue = makeCharConstant(currentToken->string[0]);
    break;
  case TK_STRING:
    eat(TK_STRING);
    constValue = makeStringConstant(currentToken->string);
    break;
  default:
    constValue = compileConstant2();
    break;
  }
  return constValue;
}

ConstantValue* compileConstant2(void) {
  ConstantValue* constValue = NULL;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    constValue = makeIntConstant(currentToken->value);
    break;
  case TK_DOUBLE:
    eat(TK_DOUBLE);
    constValue = makeDoubleConstant(currentToken->value);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredConstant(currentToken->string);
    if (obj->constAttrs->value->type == TP_INT || obj->constAttrs->value->type == TP_DOUBLE
        || obj->constAttrs->value->type == TP_CHAR || obj->constAttrs->value->type == TP_STRING)
      constValue = duplicateConstantValue(obj->constAttrs->value);
    else
      error(ERR_UNDECLARED_NUMBER_CONSTANT,currentToken->lineNo, currentToken->colNo);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

Type* compileType(void) {
  Type* type = NULL;
  Type* elementType;
  int arraySize;
  Object* obj;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    eat(KW_INTEGER);
    type =  makeIntType();
    break;
  case KW_DOUBLE:
    eat(KW_DOUBLE);
    type = makeDoubleType();
    break;
  case KW_CHAR: 
    eat(KW_CHAR); 
    type = makeCharType();
    break;
  case KW_STRING:
    eat(KW_STRING);
    type = makeStringType();
    break;
  case KW_ARRAY:
    eat(KW_ARRAY);
    eat(SB_LSEL);
    eat(TK_NUMBER);

    arraySize = currentToken->value;

    eat(SB_RSEL);
    eat(KW_OF);
    elementType = compileType();
    type = makeArrayType(arraySize, elementType);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredType(currentToken->string);
    type = duplicateType(obj->typeAttrs->actualType);
    break;
  default:
    error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

Type* compileBasicType(void) {
  Type* type = NULL;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    eat(KW_INTEGER); 
    type = makeIntType();
    break;
  case KW_CHAR: 
    eat(KW_CHAR); 
    type = makeCharType();
    break;
  case KW_DOUBLE:
    eat(KW_DOUBLE);
    type = makeDoubleType();
    break;
  case KW_STRING:
    eat(KW_STRING);
    type = makeStringType();
    break;
  default:
    error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

void compileParams(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileParam();
    while (lookAhead->tokenType == SB_SEMICOLON) {
      eat(SB_SEMICOLON);
      compileParam();
    }
    eat(SB_RPAR);
  }
}

void compileParam(void) {
  Object* param;
  Type* type;
  enum ParamKind paramKind = PARAM_VALUE;

  switch (lookAhead->tokenType) {
  case TK_IDENT:
    paramKind = PARAM_VALUE;
    break;
  case KW_VAR:
    eat(KW_VAR);
    paramKind = PARAM_REFERENCE;
    break;
  default:
    error(ERR_INVALID_PARAMETER, lookAhead->lineNo, lookAhead->colNo);
    break;
  }

  eat(TK_IDENT);
  checkFreshIdent(currentToken->string);
  param = createParameterObject(currentToken->string, paramKind, symtab->currentScope->owner);
  eat(SB_COLON);
  type = compileBasicType();
  param->paramAttrs->type = type;
  declareObject(param);
}

void compileStatements(void) {
  compileStatement();
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileStatement();
  }
}

void compileStatement(void) {
  switch (lookAhead->tokenType) {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    compileGroupSt();
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_DO:
    compileDoSt();
    break;
  case KW_FOR:
    compileForSt();
    break;
    // EmptySt needs to check FOLLOW tokens
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
    // Error occurs
  default:
    error(ERR_INVALID_STATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

Type* compileLValue(void) {
  // parse a lvalue (a variable, an array element, a parameter, the current function identifier)
  Object* var;
  Type* varType;

  eat(TK_IDENT);
  // check if the identifier is a function identifier, or a variable identifier, or a parameter  
  var = checkDeclaredLValueIdent(currentToken->string);

  if (var->kind == OBJ_VARIABLE) {
    if (var->varAttrs->type->typeClass == TP_ARRAY)
      varType = compileIndexes(var->varAttrs->type);
    else
      varType = duplicateType(var->varAttrs->type);
  } else if (var->kind == OBJ_PARAMETER) {
    varType = duplicateType(var->paramAttrs->type);
  } else {
    varType = duplicateType(var->funcAttrs->returnType);
  } 

  return varType;
}

int compileLeftAssign(Type** LAssign, int top) {
  LAssign[top] = compileLValue();

  switch (lookAhead->tokenType)
  {
  case SB_COMMA:
    eat(SB_COMMA);
    return compileLeftAssign(LAssign, top + 1);
    break;
  case SB_ASSIGN:
    return top + 1;
    break;
  default:
    error(ERR_INVALID_STATEMENT, currentToken->lineNo, currentToken->colNo);
    break;
  }
  return -1;
}

int compileRightAssign(Type** RAssign, int top) {
  RAssign[top] = compileExpression();

  switch (lookAhead->tokenType)
  {
  case SB_COMMA:
    eat(SB_COMMA);
    return compileRightAssign(RAssign, top + 1);
    break;
  default:
    return top + 1;
  }
}

void compileAssignSt(void) {
  //  parse the assignment and check type consistency
  Type** LAssign = (Type**) calloc(MAX_ASSIGN, sizeof(Type*));
  Type** RAssign = (Type**) calloc(MAX_ASSIGN, sizeof(Type*));
  if (LAssign == NULL || RAssign == NULL) {
    printf("Error when calloc!");
    exit(1);
  }

  int Lvar = compileLeftAssign(LAssign, 0);
  eat(SB_ASSIGN);
  int Rvar = compileRightAssign(RAssign, 0);

  if (Lvar < Rvar) {
    error(ERR_ASSIGN_LEFT_LESS, currentToken->lineNo, currentToken->colNo);
  }
  else if (Lvar > Rvar) {
    error(ERR_ASSIGN_LEFT_MORE, currentToken->lineNo, currentToken->colNo);
  }

  for (int i = 0; i < Lvar; i++) {
    if (LAssign[i]->typeClass == TP_DOUBLE) {
      checkNumberType(RAssign[i]);
    } else {
      checkTypeEquality(RAssign[i], LAssign[i]);
    }
  }

  if (LAssign != NULL) free(LAssign);
  if (RAssign != NULL) free(RAssign);
}

void compileCallSt(void) {
  Object* proc;

  eat(KW_CALL);
  eat(TK_IDENT);

  proc = checkDeclaredProcedure(currentToken->string);

  compileArguments(proc->procAttrs->paramList);
}

void compileGroupSt(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileIfSt(void) {
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) 
    compileElseSt();
}

void compileElseSt(void) {
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void) {
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
}

//do - while
void compileDoSt(void) {
  eat(KW_DO);
  compileStatement();
  eat(KW_WHILE);
  compileCondition();
}

void compileForSt(void) {
  // TCheck type consistency of FOR's variable
  Object *idex = NULL;
  eat(KW_FOR);
  eat(TK_IDENT);

  // check if the identifier is a variable
  idex = checkDeclaredVariable(currentToken->string);

  eat(SB_ASSIGN);
  Type* exp1 = compileExpression();
  checkTypeEquality(exp1, idex->varAttrs->type);

  eat(KW_TO);
  Type* exp2 = compileExpression();
  checkTypeEquality(exp2, idex->varAttrs->type);

  eat(KW_DO);
  compileStatement();
}

void compileArgument(Object* param) {
  // parse an argument, and check type consistency
  //       If the corresponding parameter is a reference, the argument must be a lvalue
  Type *expType;
  if (param == NULL) {
    error(ERR_INVALID_PARAMETER, currentToken->lineNo, currentToken->colNo);
  }

  if (param->paramAttrs->kind == PARAM_VALUE) {
    expType = compileExpression();
    checkTypeEquality(expType, param->paramAttrs->type);
  } else if (param->paramAttrs->kind == PARAM_REFERENCE) {
    expType = compileLValue();
    checkTypeEquality(expType, param->paramAttrs->type);
  }
}

void compileArguments(ObjectNode* paramList) {
  //parse a list of arguments, check the consistency of the arguments and the given parameters
  Object *param;
  ObjectNode *root = paramList;
  if (root == NULL) {
    param = NULL;
  } else {
    param = root->object;
    root = root->next;
  }

  switch (lookAhead->tokenType) {
  case SB_LPAR:
    eat(SB_LPAR);
    compileArgument(param);

    while (lookAhead->tokenType == SB_COMMA) {
      eat(SB_COMMA);
      if (root != NULL) {
        param = root->object;
        root = root->next;
      } else {
        param = NULL;
      }
      compileArgument(param);
    }
    
    eat(SB_RPAR);
    break;
    // Check FOLLOW set 
  case SB_TIMES:
  case SB_SLASH:
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
  case KW_WHILE:
    break;
  default:
    error(ERR_INVALID_ARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileCondition(void) {
  // check the type consistency of LHS and RSH, check the basic type
  Type * LType = compileExpression();

  switch (lookAhead->tokenType) {
  case SB_EQ:
    eat(SB_EQ);
    break;
  case SB_NEQ:
    eat(SB_NEQ);
    break;
  case SB_LE:
    eat(SB_LE);
    break;
  case SB_LT:
    eat(SB_LT);
    break;
  case SB_GE:
    eat(SB_GE);
    break;
  case SB_GT:
    eat(SB_GT);
    break;
  default:
    error(ERR_INVALID_COMPARATOR, lookAhead->lineNo, lookAhead->colNo);
  }

  Type *RType = compileExpression();

  if (LType->typeClass == TP_INT || LType->typeClass == TP_DOUBLE) {
    checkNumberType(RType);
  } else {
    checkTypeEquality(RType, LType);
  }
}

Type* compileExpression(void) {
  Type* type;
  
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    type = compileExpression2();
    checkNumberType(type);
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    type = compileExpression2();
    checkNumberType(type);
    break;
  default:
    type = compileExpression2();
  }
  return type;
}

Type* compileExpression2(void) {
  Type* type1;
  Type* type2;

  type1 = compileTerm();
  type2 = compileExpression3();
  if (type2 == NULL) return type1;
  else {
    return autoUpcasting(type1, type2);
  }
}


Type* compileExpression3(void) {
  Type* type1;
  Type* type2;

  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    type1 = compileTerm();
    checkNumberType(type1);

    type2 = compileExpression3();
    if (type2 != NULL) {
      return autoUpcasting(type1, type2);
    }
    
    return type1;
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    type1 = compileTerm();
    checkNumberType(type1);

    type2 = compileExpression3();
    if (type2 != NULL) {
      return autoUpcasting(type1, type2);
    }
    
    return type1;
    break;
    // check the FOLLOW set
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
  case KW_WHILE:
    return NULL;
    break;
  default:
    error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
  }
  return NULL;
}

Type* compileTerm(void) {
  Type* type1;
  Type* type2;

  type1 = compileFactor();
  type2 = compileTerm2();

  if (type2 != NULL) {
    return autoUpcasting(type1, type2);
  }

  return type1;
}

Type* compileTerm2(void) {
  Type* type1;
  Type* type2;

  switch (lookAhead->tokenType) {
  case SB_TIMES:
    eat(SB_TIMES);
    type1 = compileFactor();
    checkNumberType(type1);

    type2 = compileTerm2();
    if (type2 != NULL) {
      return autoUpcasting(type1, type2);
    }
    return type1;
    break;
  case SB_SLASH:
    eat(SB_SLASH);
    type1 = compileFactor();
    checkNumberType(type1);

    type2 = compileTerm2();
    if (type2 != NULL) {
      return autoUpcasting(type1, type2);
    }
    return type1;
    break;
    // check the FOLLOW set
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
  case KW_WHILE:

    break;
  default:
    error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
  }
  return NULL;
}

Type* compileFactor(void) {
  // parse a factor and return the factor's type

  Object* obj;
  Type* type = NULL;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    type = makeIntType();
    break;
  case TK_DOUBLE:
    eat(TK_DOUBLE);
    type = makeDoubleType();
    break;
  case TK_STRING:
    eat(TK_STRING);
    type = makeStringType();
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    type = makeCharType();
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    // check if the identifier is declared

    obj = checkDeclaredIdent(currentToken->string);

    switch (obj->kind) {
    case OBJ_CONSTANT:
      if (obj->constAttrs->value->type == TP_INT) {
        type = makeIntType();
      } else if (obj->constAttrs->value->type == TP_DOUBLE) {
        type = makeDoubleType();
      } else if (obj->constAttrs->value->type == TP_CHAR) {
        type = makeCharType();
      } else if (obj->constAttrs->value->type == TP_STRING) {
        type = makeStringType();
      } else {
        error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);
      }
      break;
    case OBJ_VARIABLE:
      if (obj->varAttrs->type->typeClass == TP_ARRAY) {
        type = compileIndexes(obj->varAttrs->type);
      } else {
        type = duplicateType(obj->varAttrs->type);
      }
      break;
    case OBJ_PARAMETER:
      type = duplicateType(obj->paramAttrs->type);
      break;
    case OBJ_FUNCTION:
      compileArguments(obj->funcAttrs->paramList);
      type = duplicateType(obj->funcAttrs->returnType);
      break;
    default: 
      error(ERR_INVALID_FACTOR, currentToken->lineNo, currentToken->colNo);
      break;
    }
    break;
  default:
    error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
  }
  
  return type;
}

Type* compileIndexes(Type* arrayType) {
  // parse a sequence of indexes, check the consistency to the arrayType, and return the element type
  Type *expType;
  while (lookAhead->tokenType == SB_LSEL) {
    eat(SB_LSEL);
    expType = compileExpression();
    checkIntType(expType);
    if (arrayType->typeClass == TP_ARRAY)
      arrayType = arrayType->elementType;
    else 
      error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
    eat(SB_RSEL);
  }
  return arrayType;
}

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;
  
  currentToken = NULL;
  printf("");
  lookAhead = getValidToken();

  initSymTab();
  compileProgram();

  printObject(symtab->program,0);

  cleanSymTab();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;

}