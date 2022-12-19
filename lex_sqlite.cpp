#include "lex_sqlite.h"

#include <algorithm>
#include <cctype>

namespace {

bool not_whitespace(unsigned char ch) { return !std::isspace(ch); }

} // namespace

// The C++ version is a wrapper around the C version (which itself is
// implemented in C++).
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

void lex_sqlite(const char *sql, size_t length,
                void (*on_token)(void *cookie, lex_sqlite_token_kind_t kind,
                                 const char *token_begin, size_t token_length,
                                 const char *inside_begin,
                                 size_t inside_length),
                void *cookie) {
  const char *const end = sql + length;
  const char *token = sql;
  for (const char *current = sql; current != end;) {
    switch (*current) {
    case '\'':
    case '\"': {
      // emit the previous token
      if (token != current) {
        on_token(cookie, LEX_SQLITE_TOKEN_KIND_OTHER, token, current - token,
                 token, current - token);
      }
      // emit quoted string token
      const auto closing_quote = std::find(current + 1, end, *current);
      if (closing_quote == end) {
        on_token(cookie, LEX_SQLITE_TOKEN_KIND_STRING_LITERAL, current,
                 end - current, current + 1, end - (current + 1));
        current = token = end;
      } else {
        on_token(cookie, LEX_SQLITE_TOKEN_KIND_STRING_LITERAL, current,
                 (closing_quote + 1) - current, current + 1,
                 closing_quote - (current + 1));
        current = token = closing_quote + 1;
      }
      break;
    }
    case '/': {
      if (current + 1 == end || *(current + 1) != '*') {
        ++current;
        break;
      }
      // emit the previous token
      if (token != current) {
        on_token(cookie, LEX_SQLITE_TOKEN_KIND_OTHER, token, current - token,
                 token, current - token);
      }
      // emit block comment token
      const auto start_comment = current;
      for (;;) {
        current = std::find(current + 2, end, '*');
        if (current == end || current + 1 == end) {
          on_token(cookie, LEX_SQLITE_TOKEN_KIND_COMMENT, start_comment,
                   end - start_comment, start_comment + 2,
                   end - (start_comment + 2));
          token = end;
          break;
        } else if (*(current + 1) != '/') {
          ++current;
        } else {
          on_token(cookie, LEX_SQLITE_TOKEN_KIND_COMMENT, start_comment,
                   (current + 2) - start_comment, start_comment + 2,
                   current - (start_comment + 2));
          token = current = current + 2;
          break;
        }
      }
      break;
    }
    case '-': {
      if (current + 1 == end || *(current + 1) != '-') {
        ++current;
        break;
      }
      // emit the previous token
      if (token != current) {
        on_token(cookie, LEX_SQLITE_TOKEN_KIND_OTHER, token, current - token,
                 token, current - token);
      }
      // emit line comment token
      const auto newline = std::find(current + 2, end, '\n');
      if (newline == end) {
        on_token(cookie, LEX_SQLITE_TOKEN_KIND_COMMENT, current, end - current,
                 current + 2, end - (current + 2));
        current = token = end;
      } else {
        on_token(cookie, LEX_SQLITE_TOKEN_KIND_COMMENT, current,
                 newline + 1 - current, current + 2, newline - (current + 2));
        current = token = newline + 1;
      }
      break;
    }
    default: {
      if (not_whitespace(*current)) {
        ++current;
        break;
      }
      // emit the previous token
      if (token != current) {
        on_token(cookie, LEX_SQLITE_TOKEN_KIND_OTHER, token, current - token,
                 token, current - token);
      }
      // emit whitespace token
      const auto begin_space = current;
      const auto end_space = std::find_if(current + 1, end, &not_whitespace);
      on_token(cookie, LEX_SQLITE_TOKEN_KIND_WHITESPACE, begin_space,
               end_space - begin_space, begin_space, end_space - begin_space);
      current = token = end_space;
    }
    }
  }

  // handle trailing token (if any)
  if (token != end) {
    on_token(cookie, LEX_SQLITE_TOKEN_KIND_OTHER, token, end - token, token,
             end - token);
  }
}
