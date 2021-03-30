/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#ifndef __TOKEN_H__
#define __TOKEN_H__

#define MAX_IDENT_LEN 15
#define MAX_NUMBER_LEN 10
#define KEYWORDS_COUNT 24

typedef enum {
  TK_NONE,        // Dai dien cho mot loi
  TK_IDENT,       // Dinh danh
  TK_FLOAT,       // So thuc
  TK_NUMBER,      // So nguyen
  TK_CHAR,        // Hang ky tu
  TK_EOF,         // Ket thuc chuong trinh
  TK_STRING,      // Hang xau
  // Cac tu khoa
  KW_PROGRAM, KW_CONST, KW_TYPE, KW_VAR,
  KW_INTEGER, KW_FLOAT, KW_CHAR, KW_STRING, KW_ARRAY, KW_OF, 
  KW_FUNCTION, KW_PROCEDURE,
  KW_BEGIN, KW_END, KW_CALL,
  KW_IF, KW_THEN, KW_ELSE,
  KW_WHILE, KW_DO, KW_FOR, KW_TO,
  // Cac ky hieu hac biet
  SB_SEMICOLON, SB_COLON, SB_PERIOD, SB_COMMA,
  SB_ASSIGN, SB_EQ, SB_NEQ, SB_LT, SB_LE, SB_GT, SB_GE,
  SB_PLUS, SB_MINUS, SB_TIMES, SB_SLASH,
  SB_LPAR, SB_RPAR, SB_LSEL, SB_RSEL
} TokenType; 
   
// Cau truc luu tru cua mot token
typedef struct {
  char string[MAX_IDENT_LEN + 1];
  int lineNo, colNo;
  TokenType tokenType;
  int value;
} Token;
// Kiem tra mot xau co la tu khoa khong
TokenType checkKeyword(char *string);
// Tao mot token moi voi kieu va vi tri
Token* makeToken(TokenType tokenType, int lineNo, int colNo);


#endif
