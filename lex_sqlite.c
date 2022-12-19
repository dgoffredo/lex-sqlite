#include "lex_sqlite.h"

#include <stdbool.h> // bool, true, false

static bool not_whitespace(char ch) {
  switch (ch) {
  case ' ':
  case '\n':
  case '\r':
  case '\t':
    return false;
  }
  return true;
}

static const char *find(const char *begin, const char *end, char ch) {
  while (begin != end && *begin != ch) {
    ++begin;
  }
  return begin;
}

static const char *find_if(const char *begin, const char *end,
                           bool (*predicate)(char)) {
  while (begin != end && !predicate(*begin)) {
    ++begin;
  }
  return begin;
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
      const char *const closing_quote = find(current + 1, end, *current);
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
      const char *const start_comment = current;
      for (;;) {
        current = find(current + 2, end, '*');
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
      const char *const newline = find(current + 2, end, '\n');
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
      const char *const begin_space = current;
      const char *const end_space = find_if(current + 1, end, &not_whitespace);
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
