#include "lex_sqlite.h"

#include <functional>
#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>

namespace {

// C++ wrapper
void lex_sqlite(std::string_view sql,
                const std::function<void(lex_sqlite_token_kind_t kind,
                                         std::string_view token,
                                         std::string_view inside)> &on_token) {
  typedef void (*c_on_token_t)(void *cookie, lex_sqlite_token_kind_t kind,
                               const char *token_begin, size_t token_length,
                               const char *inside_begin, size_t inside_length);

  const auto handler = [](void *cookie, lex_sqlite_token_kind_t kind,
                          const char *token_begin, size_t token_length,
                          const char *inside_begin, size_t inside_length) {
    const auto on_token = static_cast<const std::function<void(
        lex_sqlite_token_kind_t, std::string_view, std::string_view)> *>(
        cookie);
    (*on_token)(kind, std::string_view{token_begin, token_length},
                std::string_view{inside_begin, inside_length});
  };

  const auto handler_ptr =
      const_cast<void *>(static_cast<const void *>(&on_token));
  lex_sqlite(sql.data(), sql.size(), static_cast<c_on_token_t>(handler),
             handler_ptr);
}

} // namespace

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

  std::vector<char> buffer(sql.size());
  auto end = lex_sqlite_normalize(sql.data(), sql.size(), buffer.data());
  std::cout << "\nnormalized: ";
  std::cout.write(buffer.data(), end - buffer.data()) << '\n';
}
