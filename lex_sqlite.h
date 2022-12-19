#pragma once

#include <stddef.h>

typedef enum {
  LEX_SQLITE_TOKEN_KIND_WHITESPACE,
  LEX_SQLITE_TOKEN_KIND_STRING_LITERAL,
  LEX_SQLITE_TOKEN_KIND_COMMENT,
  LEX_SQLITE_TOKEN_KIND_OTHER,
} lex_sqlite_token_kind_t;

#ifdef __cplusplus
extern "C"
#endif
    void
    lex_sqlite(const char *sql, size_t length,
               void (*on_token)(void *cookie, lex_sqlite_token_kind_t kind,
                                const char *token_begin, size_t token_length,
                                const char *inside_begin, size_t inside_length),
               void *cookie);

#ifdef __cplusplus
#include <cstddef>
#include <functional>
#include <string_view>

void lex_sqlite(std::string_view sql,
                const std::function<void(lex_sqlite_token_kind_t kind,
                                         std::string_view token,
                                         std::string_view inside)> &on_token);

#endif
