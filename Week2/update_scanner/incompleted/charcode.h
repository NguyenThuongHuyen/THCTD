/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#ifndef __CHARCODE_H__
#define __CHARCODE_H__

typedef enum {
  CHAR_SPACE,             // Khoang trong
  CHAR_LETTER,            // Chu cai
  CHAR_DIGIT,             // Chu so
  CHAR_PLUS,              // '+'
  CHAR_MINUS,             // '-'
  CHAR_TIMES,             // '*'
  CHAR_SLASH,             // '/'
  CHAR_LT,                // '<'
  CHAR_GT,                // '>'
  CHAR_EXCLAIMATION,      // '!'
  CHAR_EQ,                // '='
  CHAR_COMMA,             // ','
  CHAR_PERIOD,            // '.'
  CHAR_COLON,             // ':'
  CHAR_SEMICOLON,         // ';'
  CHAR_SINGLEQUOTE,       // '\''
  CHAR_DOUBLEQUOTE,       // "
  CHAR_LPAR,              // '('
  CHAR_RPAR,              // ')'
  CHAR_UNKNOWN            // Ky tu ngoai bang chu cai
} CharCode;

#endif
