#include "token.h"

using namespace rvcc;

const std::map<TokenType, std::string> Token::type_names{
  {TokenType::TOKEN_ID,        "TOKEN_ID"},
  {TokenType::TOKEN_EOF,       "TOKEN_EOF"},
  {TokenType::TOKEN_NUM,       "TOKEN_NUM"},
  {TokenType::TOKEN_PUNCT,     "TOKEN_PUNCT"},
  {TokenType::TOKEN_ILLEGAL,   "TOKEN_ILLEGAL"}
};