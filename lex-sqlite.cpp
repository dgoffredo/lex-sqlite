#include "lex_sqlite.h"

#include <iostream>
#include <sstream>

int main() {
  std::ostringstream stream;
  stream << std::cin.rdbuf();
  const std::string sql = stream.str();
  lex_sqlite(sql, [](lex_sqlite_token_kind_t kind, std::string_view /*token*/,
                     std::string_view inside) {
    switch (kind) {
    case LEX_SQLITE_TOKEN_KIND_COMMENT:
      std::cout << "comment: " << inside << '\n';
      break;
    case LEX_SQLITE_TOKEN_KIND_STRING_LITERAL:
      std::cout << "string: " << inside << '\n';
      break;
    case LEX_SQLITE_TOKEN_KIND_OTHER:
      // `inside` is the same as `token` for "other" kind
      std::cout << "other: " << inside << '\n';
    default:
      break;
    }
  });
}