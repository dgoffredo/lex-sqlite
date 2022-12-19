#pragma once

#include <stddef.h> // size_t

typedef enum {
  LEX_SQLITE_TOKEN_KIND_WHITESPACE,
  LEX_SQLITE_TOKEN_KIND_STRING_LITERAL,
  LEX_SQLITE_TOKEN_KIND_COMMENT,
  LEX_SQLITE_TOKEN_KIND_OTHER,
} lex_sqlite_token_kind_t;

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

EXTERN_C
void lex_sqlite(const char *sql, size_t length,
                void (*on_token)(void *cookie, lex_sqlite_token_kind_t kind,
                                 const char *token_begin, size_t token_length,
                                 const char *inside_begin,
                                 size_t inside_length),
                void *cookie);

EXTERN_C
char *lex_sqlite_normalize(const char *sql, size_t length, char *output);

#undef EXTERN_C
