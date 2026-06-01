/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2026 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
--------------------------------------------------------------------------------
--- TODO: Disable comment stripping, single-line + block comment string setter.
----------------------------------------------------------------------------- */
#ifndef __WRENCH_PREPROCESSOR_H__
#define __WRENCH_PREPROCESSOR_H__

#if !WRENCH_NO_CSTDLIB
    /*
     * For bool.
     */
    #include <stdbool.h>
#endif

/*
================================================================================
 * ~~ [ types ] ~~ *
--------------------------------------------------------------------------------
*/

struct wrench_preprocessor_t; // Opaque context C structure.
typedef struct wrench_preprocessor_t* wrench_preprocessor_p;

/*
================================================================================
 * ~~ [ macros ] ~~ *
--------------------------------------------------------------------------------
*/

/* Function declaration.
 */
#ifndef WPP_DECL
#define WPP_DECL(ret, name, args) ret wrench_preprocessor_ ## name args
#endif

/* Function definition.
 */
#ifndef WPP_IMPL
#define WPP_IMPL(ret, name, args) ret wrench_preprocessor_ ## name args
#endif

/* Helper for foreign library init and quit functions.
 */
#if !defined(WRENCH_EXPORT)
    #if defined(__cplusplus)
        #if _MSC_VER
            #define WRENCH_EXPORT __declspec(dllexport) extern "C"
        #else
            #define WRENCH_EXPORT extern "C"
        #endif
    #else
        #if _MSC_VER
            #define WRENCH_EXPORT __declspec(dllexport)
        #else
            #define WRENCH_EXPORT extern
        #endif
    #endif
#endif /* !WRENCH_EXPORT */

/* Enables safety checks at a performance cost.
 */
#if !defined(WRENCH_DEBUG)
    #if defined(_DEBUG) || defined(DEBUG) || (defined(__GNUC__) && !defined(__OPTIMIZE__)) || !NDEBUG
        #define WRENCH_DEBUG 1
    #else
        #define WRENCH_DEBUG 0
    #endif
#endif /* !WRENCH_DEBUG */

/*
================================================================================
 * ~~ [ public API ] ~~ *
--------------------------------------------------------------------------------
*/

/* Create and destroy a preprocessor context.
 */
WPP_DECL(wrench_preprocessor_p, create, (const char* base_path));
WPP_DECL(void, destroy, (wrench_preprocessor_p context));

/* Add a directory to the #include search path.
 */
WPP_DECL(bool, add_include_path, (wrench_preprocessor_p context, const char* path));

/* Define an object-like or function-like macro.
 */
WPP_DECL(bool, define, (wrench_preprocessor_p context, const char* signature, const char* value));

/* Undefine a macro.
 */
WPP_DECL(bool, undef, (wrench_preprocessor_p context, const char* name));

/* Process a file and get a null-terminated dynamically allocated string.
 * Returns NULL on failure (e.g., file not found or #error encountered).
 */
WPP_DECL(const char*, process_file, (wrench_preprocessor_p context, const char* filename));

/* Processes a string directly. Returns dynamically allocated string or NULL on error.
 */
WPP_DECL(const char*, process_string, (wrench_preprocessor_p context, const char* filename, const char* input));

/* Enables or disables the preservation of line continuations (\) in the output.
 */
WPP_DECL(void, set_keep_spliced_lines, (wrench_preprocessor_p context, bool enabled));
WPP_DECL(bool, get_keep_spliced_lines, (wrench_preprocessor_p context));

/* Preprocessor error message.
 */
WPP_DECL(void, set_error_string, (wrench_preprocessor_p context, const char* error));
WPP_DECL(const char*, get_error_string, (wrench_preprocessor_p context));

/* Enable customized syntax (#define -> @define etc).
 */
WPP_DECL(void, set_directive_prefix, (wrench_preprocessor_p context, char prefix));
WPP_DECL(char, get_directive_prefix, (wrench_preprocessor_p context));

// TODO: set_base_path
// TODO: get_base_path

#endif /* __WRENCH_PREPROCESSOR_H__ */

/*
================================================================================
 * ~~ [ implementation ] ~~ *
--------------------------------------------------------------------------------
*/

#ifdef WRENCH_PREPROCESSOR_IMPLEMENTATION
/*
 * Enable multiple file inclusions with for ease of use.
 */
#ifndef __WRENCH_PREPROCESSOR_C__
#define __WRENCH_PREPROCESSOR_C__

/* ===== [ standard library ] =============================================== */

/* Disable MSVC warnings about fopen() etc.
 */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#if !WRENCH_NO_CSTDLIB
    #include <ctype.h>
    #include <stdarg.h>
    #include <stdbool.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <time.h>
#endif

#if !_WIN32 && !WRENCH_NO_POSIX_HEADERS
    #include <signal.h>
#endif

#ifndef wrench_calloc
#define wrench_calloc calloc
#endif
#ifndef wrench_fclose
#define wrench_fclose fclose
#endif
#ifndef wrench_ferror
#define wrench_ferror ferror
#endif
#ifndef wrench_fopen
#define wrench_fopen fopen
#endif
#ifndef wrench_fprintf
#define wrench_fprintf fprintf
#endif
#ifndef wrench_fread
#define wrench_fread fread
#endif
#ifndef wrench_free
#define wrench_free(x) free((void*)(x))
#endif
#ifndef wrench_fseek
#define wrench_fseek fseek
#endif
#ifndef wrench_ftell
#define wrench_ftell ftell
#endif
#ifndef wrench_isalnum
#define wrench_isalnum isalnum
#endif
#ifndef wrench_isalpha
#define wrench_isalpha isalpha
#endif
#ifndef wrench_isspace
#define wrench_isspace isspace
#endif
#ifndef wrench_localtime
#define wrench_localtime localtime
#endif
#ifndef wrench_malloc
#define wrench_malloc malloc
#endif
#ifndef wrench_memcpy
#define wrench_memcpy memcpy
#endif
#ifndef wrench_memset
#define wrench_memset memset
#endif
#ifndef wrench_realloc
#define wrench_realloc realloc
#endif
#ifndef wrench_snprintf
#define wrench_snprintf snprintf
#endif
/*
 * XXX TODO FIXME: We #undef std(in/err/out) in wrench.h - do the same thing here.
 */
#ifndef wrench_stderr
#define wrench_stderr stderr
#endif
#ifndef wrench_strcmp
#define wrench_strcmp strcmp
#endif
#ifndef wrench_strftime
#define wrench_strftime strftime
#endif
#ifndef wrench_strlen
#define wrench_strlen strlen
#endif
#ifndef wrench_strncmp
#define wrench_strncmp strncmp
#endif
#ifndef wrench_strrchr
#define wrench_strrchr strrchr
#endif
#ifndef wrench_strtol
#define wrench_strtol strtol
#endif
#ifndef wrench_time
#define wrench_time time
#endif
#ifndef wrench_va_end
#define wrench_va_end va_end
#endif
#ifndef wrench_va_start
#define wrench_va_start va_start
#endif
#ifndef wrench_vsnprintf
#define wrench_vsnprintf vsnprintf
#endif

#if !defined(wrench_strdup)
    #if 1
        #if _MSC_VER
            #define wrench_strdup _strdup
        #else
            #define wrench_strdup strdup
        #endif
    #else
        static char* _wrench_strdup(const char* s)
        {
            const size_t len = wrench_strlen(s);
            char* d = (char*)wrench_malloc(len + 1);

            if (d == NULL)
            {
                return NULL;
            }

            return (char*)wrench_memcpy(d, s, len + 1);
        }

        #define wrench_strdup _wrench_strdup
    #endif
#endif

#if !defined(wrench_strndup)
    static char* _wrench_strndup(const char* s, size_t n)
    {
        size_t len = 0;

        while (len < n && s[len])
        {
            len++;
        }

        char* d = (char*)wrench_malloc(len + 1);

        if (d == NULL)
        {
            return NULL;
        }

        d[len] = '\0';
        return (char*)wrench_memcpy(d, s, len);
    }

    #define wrench_strndup _wrench_strndup
#endif

/* ===== [ structures ] ===================================================== */

typedef struct wrench_preprocessor_string_builder_t
{
    char* data;
    size_t length;
    size_t capacity;
}
wrench_preprocessor_string_builder_t;

/* A list of tokens being used/ignored in the current macro expansion. This
 * is used to "paint" pieces of macros "blue" to prevent infinite recursion.
 */
typedef struct wrench_preprocessor_hide_set_t
{
    const char* name;
    struct wrench_preprocessor_hide_set_t* next;
}
wrench_preprocessor_hide_set_t;

#ifndef WRENCH_PREPROCESSOR_MAX_ARGS
#define WRENCH_PREPROCESSOR_MAX_ARGS 16
#endif

typedef struct wrench_preprocessor_macro_t
{
    const char* name;
    /*
     * TODO: Fold into flags.
     */
    int is_function;
    int is_variadic;
    int num_params;
    const char* params[WRENCH_PREPROCESSOR_MAX_ARGS];
    const char* body;

    struct wrench_preprocessor_macro_t* next;
}
wrench_preprocessor_macro_t;

typedef struct wrench_preprocessor_include_directory_t
{
    const char* path;
    struct wrench_preprocessor_include_directory_t* next;
}
wrench_preprocessor_include_directory_t;

typedef struct wrench_preprocessor_t
{
    /* TODO: Keep macros in a hashtable.
     */
    wrench_preprocessor_macro_t* macros;
    wrench_preprocessor_include_directory_t* include_dirs;

    const char* base_path;
    int counter;
    bool keep_spliced_lines;
    int current_line;
    int if_stack[256];
    int if_depth;
    char directive_prefix;
    /*
     * TODO: We take the date and time on preprocessor creation, which ensures
     * that all invocations of __DATE__ and __TIME__ will be the same across
     * the entire preprocessing run. However, this breaks if the preprocessor
     * is reused at a later time. Investigate the requirements of the standard?
     */
    char date_str[32];
    char time_str[32];

    char current_file[1024];
    char error_string[1024];
}
wrench_preprocessor_t;

/* ===== [ utilities ] ====================================================== */

/* Indicates that a function is not implemented.
 */
#if !defined(WRENCH_STUB)
    #if 1
        #define WRENCH_STUB() wrench_fprintf(wrench_stderr, "TODO %s (file \"%s\", line %i)\n", __FUNCTION__, __FILE__, __LINE__)
    #else
        #define WRENCH_STUB() wrench_assert(0, "TODO")
    #endif
#endif /* !WRENCH_STUB */

#if !defined(WRENCH_STRINGIFY)
    #define WRENCH_STRINGIFY(s) _WRENCH_STR_IMPL(s)
    #define _WRENCH_STR_IMPL(s) #s
#endif

#if !defined(wrench_breakpoint)
    #if _WIN32
        extern void __cdecl __debugbreak(void);

        #ifndef wrench_breakpoint
        #define wrench_breakpoint() __debugbreak()
        #endif
    #else
        #ifndef wrench_breakpoint
        #define wrench_breakpoint() raise(SIGTRAP)
        #endif
    #endif
#endif /* !wrench_breakpoint */

#if !defined(wrench_assert)
    #if WRENCH_DEBUG
        #define wrench_assert(cnd, ...) if ((cnd) == 0)                                                     \
        {                                                                                                   \
            wrench_fprintf(wrench_stderr, "assert \"%s\" failed in func \"%s\" (file \"%s\", line %i): ",   \
                                                WRENCH_STRINGIFY(cnd), __FUNCTION__, __FILE__, __LINE__);   \
                                                                                                            \
            wrench_fprintf(wrench_stderr, __VA_ARGS__);                                                     \
            wrench_fprintf(wrench_stderr, "\n");                                                            \
                                                                                                            \
            wrench_breakpoint();                                                                            \
        }
    #else
        #define wrench_assert(cnd, ...) ((void)sizeof(cnd))
    #endif
#endif /* !wrench_assert */

#if defined(__GNUC__) || defined(__clang__)
    #define WRENCH_PRINTF_ATTR(fmt, args) __attribute__((format(printf, fmt, args)))
#else
    #define WRENCH_PRINTF_ATTR(fmt, args)
#endif

static void wrench_preprocessor_set_error_string_ex(wrench_preprocessor_p context, const char* format, ...) WRENCH_PRINTF_ATTR(2, 3);
static void wrench_preprocessor_set_error_string_ex(wrench_preprocessor_p context, const char* format, ...)
{
    wrench_assert(context != NULL, "");
    va_list args;

    wrench_va_start(args, format);
    {
        wrench_vsnprintf(context->error_string, sizeof(context->error_string), format, args);
    }
    wrench_va_end(args);
}

/* XXX TODO FIXME REMOVE!!!
 */
#ifndef WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING
#define WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING() do \
{ \
    wrench_fprintf(wrench_stderr, "TODO proper error handling in \"%s\" (file \"%s\" line %i)\n", __FUNCTION__, __FILE__, __LINE__); \
    wrench_breakpoint(); \
    \
    return 0; \
} \
while (0)

#endif /* WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING */

static bool wrench_preprocessor_string_builder_init(wrench_preprocessor_string_builder_t* s)
{
    s->capacity = 64;
    s->length = 0;
    s->data = (char*)wrench_malloc(s->capacity);

    if (s->data != NULL)
    {
        s->data[0] = '\0';
        return true;
    }
    else
    {
        return false;
    }
}

static bool wrench_preprocessor_string_builder_append_len(wrench_preprocessor_string_builder_t* s, const char* text, size_t len)
{
    const size_t needed = s->length + len + 1;

    if (needed > s->capacity)
    {
        while (s->capacity < needed)
        {
            if (1)
            {
                s->capacity = (s->capacity * 3) / 2;
            }
            else
            {
                s->capacity *= 2;
            }
        }

        s->data = (char*)wrench_realloc(s->data, s->capacity);

        if (s->data == NULL)
        {
            return false;
        }
    }

    wrench_memcpy(s->data + s->length, text, len);
    s->length += len;
    s->data[s->length] = '\0';

    return true;
}

static bool wrench_string_builder_append(wrench_preprocessor_string_builder_t* s, const char* text)
{
    return wrench_preprocessor_string_builder_append_len(s, text, wrench_strlen(text));
}

static bool wrench_preprocessor_string_builder_append_char(wrench_preprocessor_string_builder_t* s, char c)
{
    return wrench_preprocessor_string_builder_append_len(s, &c, 1);
}

static void wrench_preprocessor_string_builder_free(wrench_preprocessor_string_builder_t* s)
{
    wrench_free(s->data);
    wrench_memset(s, 0, sizeof(wrench_preprocessor_string_builder_t));
}

static bool wrench_preprocessor_hide_set_is_hidden(wrench_preprocessor_hide_set_t* hs, const char* name, size_t len)
{
    while (hs)
    {
        if (wrench_strncmp(hs->name, name, len) == 0 && hs->name[len] == '\0')
        {
            return true;
        }

        hs = hs->next;
    }

    return false;
}

static const char* wrench_preprocessor_read_entire_file(wrench_preprocessor_p context, const char* filepath)
{
    FILE* file = wrench_fopen(filepath, "rb");

    if (file == NULL)
    {
        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        return NULL;
    }

    if (wrench_fseek(file, 0, SEEK_END) < 0)
    {
        wrench_fclose(file);

        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        return NULL;
    }

    long size = wrench_ftell(file);

    if (size < 0)
    {
        wrench_fclose(file);

        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        return NULL;
    }

    if (wrench_fseek(file, 0, SEEK_SET) < 0)
    {
        wrench_fclose(file);

        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        return NULL;
    }

    char* buf = (char*)wrench_malloc(size + 1);

    if (buf == NULL)
    {
        wrench_fclose(file);

        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        return NULL;
    }

    size_t read_bytes = wrench_fread(buf, 1, size, file);

    if (read_bytes != (size_t)size && wrench_ferror(file))
    {
        wrench_fclose(file);
        wrench_free(buf);

        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        return NULL;
    }

    buf[read_bytes] = '\0';

    if (wrench_fclose(file) != 0)
    {
        wrench_free(buf);

        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        return NULL;
    }

    return (const char*)buf;
}

static int wrench_preprocessor_is_ident_start(char c)
{
    return wrench_isalpha((unsigned char)c) || c == '_';
}

static int wrench_preprocessor_is_ident_part(char c)
{
    return wrench_isalnum((unsigned char)c) || c == '_';
}

/* TODO: Take const char*, return const char* after the whitespace.
 */
static void wrench_preprocessor_skip_whitespace(const char** p)
{
    /* HACK: \x01 is a special character we use to preserve spliced lines.
     */
    while (**p != '\0' && (**p == ' ' || **p == '\t' || **p == '\x01'))
    {
        (*p)++;
    }
}

static wrench_preprocessor_macro_t* wrench_preprocessor_find_macro(wrench_preprocessor_t* context, const char* name, size_t len)
{
    for (wrench_preprocessor_macro_t* m = context->macros; m; m = m->next)
    {
        if (wrench_strncmp(m->name, name, len) == 0 && m->name[len] == '\0')
        {
            return m;
        }
    }

    return NULL;
}

static long wrench_preprocessor_eval_expression_logical_or(wrench_preprocessor_t* context, const char** p, int eval);

static long wrench_preprocessor_eval_primary(wrench_preprocessor_t* context, const char** p, int eval)
{
    wrench_preprocessor_skip_whitespace(p);

    if (**p == '(')
    {
        (*p)++;

        long val = wrench_preprocessor_eval_expression_logical_or(context, p, eval);
        wrench_preprocessor_skip_whitespace(p);

        if (**p == ')')
        {
            (*p)++;
        }
        else
        {
            wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: expected ')' in #if expression\n", context->current_file, context->current_line);
            return 0;
        }

        return val;
    }

    if (wrench_preprocessor_is_ident_start(**p))
    {
        while (wrench_preprocessor_is_ident_part(**p))
        {
            (*p)++;
        }

        // Undefined identifiers evaluate to 0 in #if directives.
        return 0;
    }

    if (**p == '\'')
    {
        (*p)++;
        long val = 0;

        if (**p == '\\')
        {
            (*p)++;

            switch (**p)
            {
                case 'n': val = '\n'; break;
                case 't': val = '\t'; break;
                case 'r': val = '\r'; break;
                case '0': val = '\0'; break;
                case '\\': val = '\\'; break;
                case '\'': val = '\''; break;
                default: val = **p; break;
            }

            if (**p)
            {
                (*p)++;
            }
        }
        else if (**p)
        {
            val = **p;
            (*p)++;
        }

        if (**p == '\'')
        {
            (*p)++;
        }
        else
        {
            wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: unterminated character constant\n", context->current_file, context->current_line);
            return 0;
        }

        return eval ? val : 0;
    }

    char* endptr;
    long val = wrench_strtol(*p, &endptr, 0);

    if (endptr == *p)
    {
        wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: invalid token in #if expression: '%c'\n", context->current_file, context->current_line, **p);
        return 0;
    }

    *p = endptr;

    while (**p == 'u' || **p == 'U' || **p == 'l' || **p == 'L')
    {
        (*p)++;
    }

    return eval ? val : 0;
}

static long wrench_preprocessor_eval_unary(wrench_preprocessor_t* context, const char** p, int eval)
{
    wrench_preprocessor_skip_whitespace(p);

    if (**p == '+') { (*p)++; return +wrench_preprocessor_eval_unary(context, p, eval); }
    if (**p == '-') { (*p)++; return -wrench_preprocessor_eval_unary(context, p, eval); }
    if (**p == '!') { (*p)++; return !wrench_preprocessor_eval_unary(context, p, eval); }
    if (**p == '~') { (*p)++; return ~wrench_preprocessor_eval_unary(context, p, eval); }

    return wrench_preprocessor_eval_primary(context, p, eval);
}

static long wrench_preprocessor_eval_mul(wrench_preprocessor_t* context, const char** p, int eval)
{
    long left = wrench_preprocessor_eval_unary(context, p, eval);

    while (1)
    {
        wrench_preprocessor_skip_whitespace(p);

        if (**p == '*')
        {
            (*p)++;
            long right = wrench_preprocessor_eval_unary(context, p, eval);

            if (eval)
            {
                left *= right;
            }
        }
        else if (**p == '/')
        {
            (*p)++;
            long right = wrench_preprocessor_eval_unary(context, p, eval);

            if (eval)
            {
                if (right == 0)
                {
                    wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: division by zero in #if expression\n", context->current_file, context->current_line);
                    left = 0;
                }
                else
                {
                    left /= right;
                }
            }
        }
        else if (**p == '%')
        {
            (*p)++;
            long right = wrench_preprocessor_eval_unary(context, p, eval);

            if (eval)
            {
                if (right == 0)
                {
                    wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: modulo by zero in #if expression\n", context->current_file, context->current_line);
                    left = 0;
                }
                else
                {
                    left %= right;
                }
            }
        }
        else
        {
            break;
        }
    }

    return left;
}

static long wrench_preprocessor_eval_add(wrench_preprocessor_t* context, const char** p, int eval)
{
    long left = wrench_preprocessor_eval_mul(context, p, eval);

    while (1)
    {
        wrench_preprocessor_skip_whitespace(p);

        if (**p == '+')
        {
            (*p)++;
            left += wrench_preprocessor_eval_mul(context, p, eval);
        }
        else if (**p == '-')
        {
            (*p)++;
            left -= wrench_preprocessor_eval_mul(context, p, eval);
        }
        else
        {
            break;
        }
    }

    return left;
}

static long wrench_preprocessor_eval_shift(wrench_preprocessor_t* context, const char** p, int eval)
{
    long left = wrench_preprocessor_eval_add(context, p, eval);

    while (context->error_string[0] == '\0')
    {
        wrench_preprocessor_skip_whitespace(p);

        if (wrench_strncmp(*p, "<<", 2) == 0)
        {
            *p += 2;
            long right = wrench_preprocessor_eval_add(context, p, eval);

            if (eval)
            {
                if (right < 0 || right >= (long)(sizeof(long) * 8))
                {
                    wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: left shift count out of bounds\n", context->current_file, context->current_line);
                    left = 0;
                }
                else
                {
                    left <<= right;
                }
            }
        }
        else if (wrench_strncmp(*p, ">>", 2) == 0)
        {
            *p += 2;
            long right = wrench_preprocessor_eval_add(context, p, eval);

            if (eval)
            {
                if (right < 0 || right >= (long)(sizeof(long) * 8))
                {
                    wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: right shift count out of bounds\n", context->current_file, context->current_line);
                    left = 0;
                }
                else
                {
                    left >>= right;
                }
            }
        }
        else
        {
            break;
        }
    }

    return left;
}

static long wrench_preprocessor_eval_relational(wrench_preprocessor_t* context, const char** p, int eval)
{
    long left = wrench_preprocessor_eval_shift(context, p, eval);

    while (1)
    {
        wrench_preprocessor_skip_whitespace(p);

        if (wrench_strncmp(*p, "<=", 2) == 0)
        {
            *p += 2;
            left = left <= wrench_preprocessor_eval_shift(context, p, eval);
        }
        else if (wrench_strncmp(*p, ">=", 2) == 0)
        {
            *p += 2;
            left = left >= wrench_preprocessor_eval_shift(context, p, eval);
        }
        else if (**p == '<')
        {
            (*p)++;
            left = left < wrench_preprocessor_eval_shift(context, p, eval);
        }
        else if (**p == '>')
        {
            (*p)++;
            left = left > wrench_preprocessor_eval_shift(context, p, eval);
        }
        else
        {
            break;
        }
    }

    return left;
}

static long wrench_preprocessor_eval_equality(wrench_preprocessor_t* context, const char** p, int eval)
{
    long left = wrench_preprocessor_eval_relational(context, p, eval);

    while (1)
    {
        wrench_preprocessor_skip_whitespace(p);

        if (wrench_strncmp(*p, "==", 2) == 0)
        {
            *p += 2;
            left = left == wrench_preprocessor_eval_relational(context, p, eval);
        }
        else if (wrench_strncmp(*p, "!=", 2) == 0)
        {
            *p += 2;
            left = left != wrench_preprocessor_eval_relational(context, p, eval);
        }
        else
        {
            break;
        }
    }

    return left;
}

static long wrench_preprocessor_eval_bitwise_and(wrench_preprocessor_t* context, const char** p, int eval)
{
    long left = wrench_preprocessor_eval_equality(context, p, eval);

    while (**p == '&' && *(*p + 1) != '&')
    {
        (*p)++;
        left &= wrench_preprocessor_eval_equality(context, p, eval);
    }

    return left;
}

static long wrench_preprocessor_eval_bitwise_xor(wrench_preprocessor_t* context, const char** p, int eval)
{
    long left = wrench_preprocessor_eval_bitwise_and(context, p, eval);

    while (**p == '^')
    {
        (*p)++;
        left ^= wrench_preprocessor_eval_bitwise_and(context, p, eval);
    }

    return left;
}

static long wrench_preprocessor_eval_bitwise_or(wrench_preprocessor_t* context, const char** p, int eval)
{
    long left = wrench_preprocessor_eval_bitwise_xor(context, p, eval);

    while (**p == '|' && *(*p + 1) != '|')
    {
        (*p)++;
        left |= wrench_preprocessor_eval_bitwise_xor(context, p, eval);
    }

    return left;
}

static long wrench_preprocessor_eval_logical_and(wrench_preprocessor_t* context, const char** p, int eval)
{
    long left = wrench_preprocessor_eval_bitwise_or(context, p, eval);

    while (wrench_strncmp(*p, "&&", 2) == 0)
    {
        *p += 2;

        /* Short-circuit: if left is 0, parse but do not evaluate the right side. */
        int eval_right = eval && (left != 0);
        long right = wrench_preprocessor_eval_bitwise_or(context, p, eval_right);

        if (eval)
        {
            left = left && right;
        }
    }

    return left;
}

static long wrench_preprocessor_eval_expression_logical_or(wrench_preprocessor_t* context, const char** p, int eval)
{
    long left = wrench_preprocessor_eval_logical_and(context, p, eval);

    while (wrench_strncmp(*p, "||", 2) == 0)
    {
        *p += 2;

        /* Short-circuit: if left is 1, parse but do not evaluate the right side. */
        int eval_right = eval && (left == 0);
        long right = wrench_preprocessor_eval_logical_and(context, p, eval_right);

        if (eval)
        {
            left = left || right;
        }
    }

    return left;
}

static bool wrench_preprocessor_internal(wrench_preprocessor_t* context, const char* input, wrench_preprocessor_string_builder_t* out, wrench_preprocessor_hide_set_t* hs);

static bool wrench_preprocessor_save_arg(wrench_preprocessor_t* context, char** args, int* arg_idx, wrench_preprocessor_string_builder_t* current_arg)
{
    if (*arg_idx < WRENCH_PREPROCESSOR_MAX_ARGS)
    {
        char* start = current_arg->data;

        while (*start == ' ' || *start == '\t' || *start == '\n')
        {
            start++;
        }

        char* end = start + wrench_strlen(start);

        while (end > start && (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '\n'))
        {
            end--;
        }

        args[*arg_idx] = wrench_strndup(start, end - start);

        if (args[*arg_idx] == NULL)
        {
            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        }

        (*arg_idx)++;
    }
    else
    {
        wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: maximum macro arguments exceeded (%d)\n", context->current_file, context->current_line, WRENCH_PREPROCESSOR_MAX_ARGS);
        return false;
    }

    current_arg->length = 0;

    if (current_arg->capacity > 0)
    {
        current_arg->data[0] = '\0';
    }

    return true;
}

static bool wrench_preprocessor_expand_macro(wrench_preprocessor_t* context, wrench_preprocessor_macro_t* m, const char** input_ptr, wrench_preprocessor_string_builder_t* out, wrench_preprocessor_hide_set_t* hs)
{
    if (context->error_string[0] != '\0')
    {
        return false;
    }

    const char* p = *input_ptr;

    char* args[WRENCH_PREPROCESSOR_MAX_ARGS] = { 0 };
    wrench_preprocessor_string_builder_t expanded;

    if (!wrench_preprocessor_string_builder_init(&expanded))
    {
        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
    }

    if (m->is_function)
    {
        wrench_preprocessor_skip_whitespace(&p);

        if (*p != '(')
        {
            if (!wrench_string_builder_append(out, m->name))
            {
                WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
            }

            *input_ptr = p;
            goto cleanup;
        }

        p++;

        int arg_idx = 0;
        int paren_depth = 0;
        int brace_depth = 0;
        int bracket_depth = 0;

        wrench_preprocessor_string_builder_t current_arg;

        if (!wrench_preprocessor_string_builder_init(&current_arg))
        {
            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        }

        int in_quote = 0;
        char quote_char = 0;

        while (*p && context->error_string[0] == '\0')
        {
            if (in_quote)
            {
                if (!wrench_preprocessor_string_builder_append_char(&current_arg, *p))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }

                if (*p == '\\' && *(p + 1))
                {
                    if (!wrench_preprocessor_string_builder_append_char(&current_arg, *(++p)))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else if (*p == quote_char)
                {
                    in_quote = 0;
                }
            }
            else
            {
                if (*p == '"' || *p == '\'')
                {
                    in_quote = 1;
                    quote_char = *p;

                    if (!wrench_preprocessor_string_builder_append_char(&current_arg, *p))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else if (*p == '(')
                {
                    paren_depth++;

                    if (!wrench_preprocessor_string_builder_append_char(&current_arg, *p))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else if (*p == ')')
                {
                    if (paren_depth == 0)
                    {
                        if (current_arg.length > 0 || arg_idx > 0 || m->num_params > 0)
                        {
                            if (!wrench_preprocessor_save_arg(context, args, &arg_idx, &current_arg))
                            {
                                WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                            }
                        }

                        p++;
                        break;
                    }

                    paren_depth--;

                    if (!wrench_preprocessor_string_builder_append_char(&current_arg, *p))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else if (*p == '{')
                {
                    brace_depth++;

                    if (!wrench_preprocessor_string_builder_append_char(&current_arg, *p))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else if (*p == '}')
                {
                    if (brace_depth > 0)
                    {
                        brace_depth--;
                    }
                    else
                    {
                        // TODO: error?
                    }

                    if (!wrench_preprocessor_string_builder_append_char(&current_arg, *p))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else if (*p == '[')
                {
                    bracket_depth++;

                    if (!wrench_preprocessor_string_builder_append_char(&current_arg, *p))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else if (*p == ']')
                {
                    if (bracket_depth > 0)
                    {
                        bracket_depth--;
                    }
                    else
                    {
                        // TODO: error?
                    }

                    if (!wrench_preprocessor_string_builder_append_char(&current_arg, *p))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else if (*p == ',' && paren_depth == 0 && (!m->is_variadic || arg_idx < m->num_params - 1))
                {
                    if (!wrench_preprocessor_save_arg(context, args, &arg_idx, &current_arg))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else
                {
                    if (!wrench_preprocessor_string_builder_append_char(&current_arg, *p))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
            }

            p++;
        }

        wrench_preprocessor_string_builder_free(&current_arg);

        if (paren_depth > 0 || brace_depth > 0 || bracket_depth > 0 || in_quote)
        {
            wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: unterminated macro invocation for '%s'\n", context->current_file, context->current_line, m->name);
            goto cleanup;
        }

        *input_ptr = p;
    }

    const char* b;

    if (context->error_string[0] != '\0') goto cleanup;
    b = m->body;

    while (*b)
    {
        if (*b == context->directive_prefix && *(b + 1) != context->directive_prefix)
        {
            b++;

            wrench_preprocessor_skip_whitespace(&b);
            const char* id_start = b;

            while (wrench_preprocessor_is_ident_part(*b))
            {
                b++;
            }

            size_t id_len = b - id_start;
            int found = 0;

            for (int i = 0; i < m->num_params; i++)
            {
                int is_match = 0;

                if (m->is_variadic && i == m->num_params - 1 && id_len == 11 && wrench_strncmp(id_start, "__VA_ARGS__", 11) == 0)
                {
                    is_match = 1;
                }
                else if (wrench_strlen(m->params[i]) == id_len && wrench_strncmp(m->params[i], id_start, id_len) == 0)
                {
                    is_match = 1;
                }

                if (is_match)
                {
                    if (!wrench_preprocessor_string_builder_append_char(&expanded, '"'))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }

                    if (args[i])
                    {
                        int last_was_space = 0;

                        for (char* cp = args[i]; *cp; cp++)
                        {
                            if (wrench_isspace((unsigned char)*cp))
                            {
                                if (!last_was_space)
                                {
                                    if (!wrench_preprocessor_string_builder_append_char(&expanded, ' '))
                                    {
                                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                                    }

                                    last_was_space = 1;
                                }

                                continue;
                            }

                            last_was_space = 0;

                            if (*cp == '"' || *cp == '\\')
                            {
                                if (!wrench_preprocessor_string_builder_append_char(&expanded, '\\'))
                                {
                                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                                }
                            }

                            if (!wrench_preprocessor_string_builder_append_char(&expanded, *cp))
                            {
                                WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                            }
                        }
                    }

                    if (!wrench_preprocessor_string_builder_append_char(&expanded, '"'))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }

                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                if (!wrench_preprocessor_string_builder_append_char(&expanded, context->directive_prefix))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }

                if (!wrench_preprocessor_string_builder_append_len(&expanded, id_start, id_len))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }
            }

            continue;
        }

        if (*b == context->directive_prefix && *(b + 1) == context->directive_prefix)
        {
            b += 2;

            while (expanded.length > 0 && wrench_isspace((unsigned char)expanded.data[expanded.length - 1]))
            {
                expanded.length--;
                expanded.data[expanded.length] = '\0';
            }

            wrench_preprocessor_skip_whitespace(&b);
            continue;
        }

        if (wrench_preprocessor_is_ident_start(*b))
        {
            const char* id_start = b;

            while (wrench_preprocessor_is_ident_part(*b))
            {
                b++;
            }

            size_t id_len = b - id_start;
            int replaced = 0;

            for (int i = 0; i < m->num_params; i++)
            {
                int is_match = 0;

                if (m->is_variadic && i == m->num_params - 1 && id_len == 11 && wrench_strncmp(id_start, "__VA_ARGS__", 11) == 0)
                {
                    is_match = 1;
                }
                else if (wrench_strlen(m->params[i]) == id_len && wrench_strncmp(m->params[i], id_start, id_len) == 0)
                {
                    is_match = 1;
                }

                if (is_match)
                {
                    if (args[i])
                    {
                        if (!wrench_string_builder_append(&expanded, args[i]))
                        {
                            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                        }
                    }

                    replaced = 1;
                    break;
                }
            }

            if (!replaced)
            {
                if (!wrench_preprocessor_string_builder_append_len(&expanded, id_start, id_len))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }
            }
        }
        else
        {
            if (!wrench_preprocessor_string_builder_append_char(&expanded, *b++))
            {
                WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
            }
        }
    }

    if (context->error_string[0] == '\0')
    {
        wrench_preprocessor_hide_set_t new_hs = { m->name, hs };

        if (!wrench_preprocessor_internal(context, expanded.data, out, &new_hs))
        {
            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        }
    }

    cleanup:
    {
        for (int i = 0; i < WRENCH_PREPROCESSOR_MAX_ARGS; i++)
        {
            if (args[i])
            {
                wrench_free(args[i]);
            }
        }

        wrench_preprocessor_string_builder_free(&expanded);
        return true;
    }
}

static char* wrench_preprocessor_preprocess_defined(wrench_preprocessor_t* context, const char* expr)
{
    wrench_preprocessor_string_builder_t out;

    if (!wrench_preprocessor_string_builder_init(&out))
    {
        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
    }

    const char* p = expr;

    while (*p)
    {
        if (wrench_preprocessor_is_ident_start(*p))
        {
            const char* id_start = p;

            while (wrench_preprocessor_is_ident_part(*p))
            {
                p++;
            }

            size_t id_len = p - id_start;

            if (id_len == 7 && wrench_strncmp(id_start, "defined", 7) == 0)
            {
                const char* temp = p;
                wrench_preprocessor_skip_whitespace(&temp);

                int has_paren = (*temp == '(');
                if (has_paren)
                {
                    temp++;
                }

                wrench_preprocessor_skip_whitespace(&temp);
                const char* target_start = temp;

                while (wrench_preprocessor_is_ident_part(*temp))
                {
                    temp++;
                }

                size_t target_len = temp - target_start;

                int is_def = wrench_preprocessor_find_macro(context, target_start, target_len) != NULL;
                wrench_preprocessor_skip_whitespace(&temp);

                if (has_paren && *temp == ')')
                {
                    temp++;
                }

                p = temp;

                if (!wrench_preprocessor_string_builder_append_char(&out, is_def ? '1' : '0'))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }
            }
            else
            {
                if (!wrench_preprocessor_string_builder_append_len(&out, id_start, id_len))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }
            }
        }
        else if (*p == '"' || *p == '\'')
        {
            char q = *p;

            if (!wrench_preprocessor_string_builder_append_char(&out, *p++))
            {
                WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
            }

            while (*p && *p != q && *p != '\n')
            {
                if (*p == '\\' && *(p + 1))
                {
                    if (!wrench_preprocessor_string_builder_append_char(&out, *p++))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }

                if (!wrench_preprocessor_string_builder_append_char(&out, *p++))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }
            }

            if (*p == q)
            {
                if (!wrench_preprocessor_string_builder_append_char(&out, *p++))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }
            }
        }
        else
        {
            if (!wrench_preprocessor_string_builder_append_char(&out, *p++))
            {
                WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
            }
        }
    }

    return out.data;
}

static char* wrench_preprocessor_get_dir_name(wrench_preprocessor_p context, const char* path)
{
    const char* last_slash = wrench_strrchr(path, '/');
    const char* last_bslash = wrench_strrchr(path, '\\');

    const char* slash = (last_slash > last_bslash) ? last_slash : last_bslash;

    if (slash == NULL)
    {
        /* TODO: Error checking!
         */
        return wrench_strdup(context->base_path != NULL ? context->base_path : ".");
    }

    return wrench_strndup(path, slash - path);
}

static bool wrench_preprocessor_internal(wrench_preprocessor_t* context, const char* input, wrench_preprocessor_string_builder_t* out, wrench_preprocessor_hide_set_t* hs)
{
    if (context->error_string[0] != '\0')
    {
        return false;
    }

    const char* p = input;

    while (*p && context->error_string[0] == '\0')
    {
        const char* line_start = p;

        while (*p == ' ' || *p == '\t')
        {
            p++;
        }

        int parent_active = (context->if_depth <= 1) || (context->if_stack[context->if_depth - 2] == 0);
        int is_active = (context->if_depth == 0) || (context->if_stack[context->if_depth - 1] == 0);

        if (*p == context->directive_prefix)
        {
            p++;

            while (*p == ' ' || *p == '\t')
            {
                p++;
            }

            const char* dir_start = p;

            while (wrench_preprocessor_is_ident_part(*p))
            {
                p++;
            }

            size_t dir_len = p - dir_start;

            if (dir_len == 5 && wrench_strncmp(dir_start, "ifdef", 5) == 0)
            {
                wrench_preprocessor_skip_whitespace(&p);
                const char* mac_start = p;

                while (wrench_preprocessor_is_ident_part(*p))
                {
                    p++;
                }

                if (context->if_depth >= WRENCH_ARRAY_COUNT(context->if_stack))
                {
                    wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: maximum #if nesting depth exceeded\n", context->current_file, context->current_line);
                    return false;
                }

                int is_def = wrench_preprocessor_find_macro(context, mac_start, p - mac_start) != NULL;
                context->if_stack[context->if_depth++] = (is_active && is_def) ? 0 : 1;

                goto skip_line;
            }
            else if (dir_len == 6 && wrench_strncmp(dir_start, "ifndef", 6) == 0)
            {
                wrench_preprocessor_skip_whitespace(&p);
                const char* mac_start = p;

                while (wrench_preprocessor_is_ident_part(*p))
                {
                    p++;
                }

                if (context->if_depth >= WRENCH_ARRAY_COUNT(context->if_stack))
                {
                    wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: maximum #if nesting depth exceeded\n", context->current_file, context->current_line);
                    return false;
                }

                int is_def = wrench_preprocessor_find_macro(context, mac_start, p - mac_start) != NULL;
                context->if_stack[context->if_depth++] = (is_active && !is_def) ? 0 : 1;

                goto skip_line;
            }
            else if (dir_len == 2 && wrench_strncmp(dir_start, "if", 2) == 0)
            {
                long val = 0;
                if (is_active)
                {
                    const char* expr_start = p;

                    while (*p && *p != '\n')
                    {
                        p++;
                    }

                    char* raw_expr = wrench_strndup(expr_start, p - expr_start);

                    if (raw_expr == NULL)
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }

                    char* def_replaced = wrench_preprocessor_preprocess_defined(context, raw_expr);
                    wrench_free(raw_expr);

                    wrench_preprocessor_string_builder_t exp_str;

                    if (!wrench_preprocessor_string_builder_init(&exp_str))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }

                    if (context->if_depth >= WRENCH_ARRAY_COUNT(context->if_stack))
                    {
                        wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: maximum #if nesting depth exceeded\n", context->current_file, context->current_line);
                        return false;
                    }

                    int old_depth = context->if_depth;
                    context->if_stack[context->if_depth++] = 0;

                    wrench_preprocessor_internal(context, def_replaced, &exp_str, hs);
                    context->if_depth = old_depth;

                    if (context->error_string[0] == '\0')
                    {
                        const char* eval_p = exp_str.data;
                        val = wrench_preprocessor_eval_expression_logical_or(context, &eval_p, 1);
                    }

                    wrench_preprocessor_string_builder_free(&exp_str);
                    wrench_free(def_replaced);
                }

                if (context->if_depth >= WRENCH_ARRAY_COUNT(context->if_stack))
                {
                    wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: maximum #if nesting depth exceeded\n", context->current_file, context->current_line);
                    return false;
                }

                context->if_stack[context->if_depth++] = (is_active && val) ? 0 : 1;
                goto skip_line;
            }
            else if (dir_len == 4 && wrench_strncmp(dir_start, "elif", 4) == 0)
            {
                /* TODO: If if_depth is 0 here, preprocessing should fail.
                 */
                if (context->if_depth > 0)
                {
                    if (context->if_stack[context->if_depth - 1] == 0)
                    {
                        context->if_stack[context->if_depth - 1] = 2;
                    }
                    else if (context->if_stack[context->if_depth - 1] == 1 && parent_active)
                    {
                        const char* expr_start = p;

                        while (*p && *p != '\n')
                        {
                            p++;
                        }

                        char* raw_expr = wrench_strndup(expr_start, p - expr_start);

                        if (raw_expr == NULL)
                        {
                            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                        }

                        char* def_replaced = wrench_preprocessor_preprocess_defined(context, raw_expr);
                        wrench_free(raw_expr);

                        wrench_preprocessor_string_builder_t exp_str;

                        if (!wrench_preprocessor_string_builder_init(&exp_str))
                        {
                            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                        }

                        if (context->if_depth >= WRENCH_ARRAY_COUNT(context->if_stack))
                        {
                            wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: maximum #if nesting depth exceeded\n", context->current_file, context->current_line);
                            return false;
                        }

                        int old_depth = context->if_depth;
                        context->if_stack[context->if_depth++] = 0;

                        wrench_preprocessor_internal(context, def_replaced, &exp_str, hs);

                        context->if_depth = old_depth;
                        long val = 0;

                        if (context->error_string[0] == '\0')
                        {
                            const char* eval_p = exp_str.data;
                            val = wrench_preprocessor_eval_expression_logical_or(context, &eval_p, 1);
                        }

                        wrench_preprocessor_string_builder_free(&exp_str);
                        wrench_free(def_replaced);

                        if (val)
                        {
                            context->if_stack[context->if_depth - 1] = 0;
                        }
                    }
                }

                goto skip_line;
            }
            else if (dir_len == 4 && wrench_strncmp(dir_start, "else", 4) == 0)
            {
                /* TODO: If if_depth is 0 here, preprocessing should fail.
                 */
                if (context->if_depth > 0)
                {
                    if (context->if_stack[context->if_depth - 1] == 0 || context->if_stack[context->if_depth - 1] == 2)
                    {
                        context->if_stack[context->if_depth - 1] = 2;
                    }
                    else if (context->if_stack[context->if_depth - 1] == 1 && parent_active)
                    {
                        context->if_stack[context->if_depth - 1] = 0;
                    }
                }

                goto skip_line;
            }
            else if (dir_len == 5 && wrench_strncmp(dir_start, "endif", 5) == 0)
            {
                /* TODO: If if_depth is 0 here, preprocessing should fail.
                 */
                if (context->if_depth > 0)
                {
                    context->if_depth--;
                }

                goto skip_line;
            }

            if (!is_active)
            {
                goto skip_line;
            }

            if (dir_len == 6 && wrench_strncmp(dir_start, "define", 6) == 0)
            {
                wrench_preprocessor_skip_whitespace(&p);
                const char* sig_start = p;

                while (*p && *p != '\n' && !(*p == ' ' || *p == '\t'))
                {
                    if (*p == '(')
                    {
                        while (*p && *p != ')')
                        {
                            p++;
                        }

                        if (*p == ')')
                        {
                            p++;
                        }

                        break;
                    }

                    p++;
                }

                char* sig = wrench_strndup(sig_start, p - sig_start);

                if (sig == NULL)
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }

                wrench_preprocessor_skip_whitespace(&p);
                const char* val_start = p;

                while (*p && *p != '\n')
                {
                    p++;
                }

                char* val = wrench_strndup(val_start, p - val_start);

                if (val == NULL)
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }

                if (!wrench_preprocessor_define(context, sig, val))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }

                wrench_free(sig);
                wrench_free(val);

                if (*p == '\n')
                {
                    p++;
                    context->current_line++;

                    if (!wrench_preprocessor_string_builder_append_char(out, '\n'))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }

                continue;
            }
            else if (dir_len == 5 && wrench_strncmp(dir_start, "undef", 5) == 0)
            {
                wrench_preprocessor_skip_whitespace(&p);
                const char* mac_start = p;

                while (wrench_preprocessor_is_ident_part(*p))
                {
                    p++;
                }

                char* mac_name = wrench_strndup(mac_start, p - mac_start);

                if (mac_name == NULL)
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }

                if (!wrench_preprocessor_undef(context, mac_name))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }

                wrench_free(mac_name);

                goto skip_line;
            }
            else if (dir_len == 5 && wrench_strncmp(dir_start, "error", 5) == 0)
            {
                wrench_preprocessor_skip_whitespace(&p);
                const char* msg_start = p;

                while (*p && *p != '\n')
                {
                    p++;
                }

                wrench_preprocessor_set_error_string_ex(context, "preprocessor error at %s:%d: %.*s\n", context->current_file, context->current_line, (int)(p - msg_start), msg_start);
                return false;
            }
            else if (dir_len == 6 && wrench_strncmp(dir_start, "pragma", 6) == 0)
            {
                p = line_start;
            }
            else if (dir_len == 7 && wrench_strncmp(dir_start, "include", 7) == 0)
            {
                wrench_preprocessor_skip_whitespace(&p);

                char quote = *p;
                char* expanded_include = NULL;
                const char* inc_p = p;

                if (quote != '"' && quote != '<')
                {
                    const char* expr_start = p;
                    while (*p && *p != '\n') p++;

                    char* raw_expr = wrench_strndup(expr_start, p - expr_start);

                    if (raw_expr == NULL)
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }

                    wrench_preprocessor_string_builder_t exp_str;

                    if (!wrench_preprocessor_string_builder_init(&exp_str))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }

                    wrench_preprocessor_internal(context, raw_expr, &exp_str, hs);
                    wrench_free(raw_expr);

                    expanded_include = exp_str.data;
                    inc_p = expanded_include;

                    wrench_preprocessor_skip_whitespace(&inc_p);
                    quote = *inc_p;
                }

                if (quote == '"' || quote == '<')
                {
                    char end_quote = (quote == '<') ? '>' : '"';
                    inc_p++;
                    const char* file_start = inc_p;

                    while (*inc_p && *inc_p != end_quote && *inc_p != '\n')
                    {
                        inc_p++;
                    }

                    char* inc_file = wrench_strndup(file_start, inc_p - file_start);

                    if (inc_file == NULL)
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }

                    const char* content = NULL;

                    if (quote == '"')
                    {
                        char* current_dir = wrench_preprocessor_get_dir_name(context, context->current_file);

                        if (current_dir == NULL)
                        {
                            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                        }

                        wrench_preprocessor_string_builder_t path_str;

                        if (!wrench_preprocessor_string_builder_init(&path_str))
                        {
                            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                        }

                        if (!wrench_string_builder_append(&path_str, current_dir))
                        {
                            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                        }

                        if (!wrench_string_builder_append(&path_str, "/"))
                        {
                            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                        }

                        if (!wrench_string_builder_append(&path_str, inc_file))
                        {
                            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                        }

                        content = wrench_preprocessor_read_entire_file(context, path_str.data);

                        wrench_preprocessor_string_builder_free(&path_str);
                        wrench_free(current_dir);

                        if (!content)
                        {
                            content = wrench_preprocessor_read_entire_file(context, inc_file);
                        }
                    }

                    if (!content)
                    {
                        for (wrench_preprocessor_include_directory_t* d = context->include_dirs; d; d = d->next)
                        {
                            wrench_preprocessor_string_builder_t path_str;

                            if (!wrench_preprocessor_string_builder_init(&path_str))
                            {
                                WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                            }

                            if (!wrench_string_builder_append(&path_str, d->path))
                            {
                                WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                            }

                            if (!wrench_string_builder_append(&path_str, "/"))
                            {
                                WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                            }

                            if (!wrench_string_builder_append(&path_str, inc_file))
                            {
                                WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                            }

                            content = wrench_preprocessor_read_entire_file(context, path_str.data);
                            wrench_preprocessor_string_builder_free(&path_str);

                            if (content)
                            {
                                break;
                            }
                        }
                    }

                    if (content)
                    {
                        char prev_file[1024];
                        wrench_snprintf(prev_file, sizeof(prev_file), "%s", context->current_file);

                        int prev_line = context->current_line;

                        wrench_snprintf(context->current_file, sizeof(context->current_file), "%s", inc_file);
                        context->current_line = 1;

                        wrench_preprocessor_internal(context, content, out, hs);

                        wrench_snprintf(context->current_file, sizeof(context->current_file), "%s", prev_file);
                        context->current_line = prev_line;

                        wrench_free(content);
                    }
                    else
                    {
                        wrench_preprocessor_set_error_string_ex(context, "preprocessor error: Could not find include '%s'\n", inc_file);
                        return false;
                    }

                    wrench_free(inc_file);
                }

                if (expanded_include)
                {
                    wrench_free(expanded_include);
                }

                if (context->error_string[0] != '\0')
                {
                    return false;
                }

                goto skip_line;
            }
            else
            {
                p = dir_start;
            }

            skip_line:
            {
                while (*p && *p != '\n')
                {
                    p++;
                }

                if (*p == '\n')
                {
                    p++;
                    context->current_line++;

                    if (!wrench_preprocessor_string_builder_append_char(out, '\n'))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }

                continue;
            }
        }

        if (!is_active)
        {
            while (*p && *p != '\n')
            {
                p++;
            }

            if (*p == '\n')
            {
                p++;
                context->current_line++;

                if (!wrench_preprocessor_string_builder_append_char(out, '\n'))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }
            }

            continue;
        }

        p = line_start;

        while (*p && *p != '\n')
        {
            if (wrench_preprocessor_is_ident_start(*p))
            {
                const char* id_start = p;

                while (wrench_preprocessor_is_ident_part(*p))
                {
                    p++;
                }

                size_t id_len = p - id_start;

                if (id_len == 8 && wrench_strncmp(id_start, "__LINE__", 8) == 0)
                {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%d", context->current_line);

                    if (!wrench_string_builder_append(out, buf))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else if (id_len == 8 && wrench_strncmp(id_start, "__FILE__", 8) == 0)
                {
                    if (!wrench_preprocessor_string_builder_append_char(out, '"'))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }

                    if (!wrench_string_builder_append(out, context->current_file))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }

                    if (!wrench_preprocessor_string_builder_append_char(out, '"'))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else if (id_len == 8 && wrench_strncmp(id_start, "__DATE__", 8) == 0)
                {
                    if (!wrench_string_builder_append(out, context->date_str))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else if (id_len == 8 && wrench_strncmp(id_start, "__TIME__", 8) == 0)
                {
                    if (!wrench_string_builder_append(out, context->time_str))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else if (id_len == 11 && wrench_strncmp(id_start, "__COUNTER__", 11) == 0)
                {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%d", context->counter++);

                    if (!wrench_string_builder_append(out, buf))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
                else
                {
                    wrench_preprocessor_macro_t* m = wrench_preprocessor_find_macro(context, id_start, id_len);

                    if (m && !wrench_preprocessor_hide_set_is_hidden(hs, id_start, id_len))
                    {
                        if (!wrench_preprocessor_expand_macro(context, m, &p, out, hs))
                        {
                            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                        }

                        if (context->error_string[0] != '\0')
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if (!wrench_preprocessor_string_builder_append_len(out, id_start, id_len))
                        {
                            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                        }
                    }
                }
            }
            else if (*p == '"' || *p == '\'')
            {
                char quote = *p;

                if (!wrench_preprocessor_string_builder_append_char(out, *p++))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }

                while (*p && *p != quote && *p != '\n')
                {
                    if (*p == '\\' && *(p + 1))
                    {
                        if (!wrench_preprocessor_string_builder_append_char(out, *p++))
                        {
                            WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                        }
                    }

                    if (!wrench_preprocessor_string_builder_append_char(out, *p++))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }

                if (*p == quote)
                {
                    if (!wrench_preprocessor_string_builder_append_char(out, *p++))
                    {
                        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                    }
                }
            }
            else
            {
                if (!wrench_preprocessor_string_builder_append_char(out, *p++))
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }
            }
        }

        if (*p == '\n')
        {
            if (!wrench_preprocessor_string_builder_append_char(out, *p++))
            {
                WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
            }

            context->current_line++;
        }
    }

    return true;
}

static char* wrench_preprocessor_strip_comments_and_splice(wrench_preprocessor_t* context, const char* input)
{
    size_t len = wrench_strlen(input);
    char* spliced = (char*)wrench_malloc(len + 1);

    if (spliced == NULL)
    {
        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        return NULL;
    }

    const char* r = input;
    char* w = spliced;
    int pending_newlines = 0;

    // Phases 1 & 2: Normalize line endings and resolve line splicing.
    while (*r)
    {
        if (*r == '\r')
        {
            r++;
            continue;
        }

        if (*r == '\\')
        {
            const char* temp = r + 1;

            /* Tolerate trailing spaces or \r before the \n.
             */
            while (*temp == ' ' || *temp == '\t' || *temp == '\r')
            {
                temp++;
            }

            if (*temp == '\n')
            {
                if (context->keep_spliced_lines)
                {
                    /* Inject internal marker.
                     */
                    *w++ = '\x01';
                }

                /* Record the deleted newline for error messages etc.
                 */
                pending_newlines++;
                r = temp + 1;

                /* Line spliced; drop the backslash and newline.
                 */
                continue;
            }
        }

        if (*r == '\n')
        {
            *w++ = *r++;

            /* Append the accumulated newlines to the end of the logical line.
             */
            while (pending_newlines > 0)
            {
                *w++ = '\n';
                pending_newlines--;
            }

            continue;
        }

        /* Append any remaining newlines if the file doesn't end with one.
         */
        while (pending_newlines > 0)
        {
            *w++ = '\n';
            pending_newlines--;
        }

        *w++ = *r++;
    }

    *w = '\0';

    /* Phase 3: Strip comments.
     */
    char* out = (char*)wrench_malloc(wrench_strlen(spliced) + 1);

    if (out == NULL)
    {
        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();

        wrench_free(spliced);
        return NULL;
    }

    r = spliced;
    w = out;

    while (*r)
    {
        if (*r == '/' && *(r + 1) == '*')
        {
            r += 2;
            *w++ = ' ';

            while (*r)
            {
                if (*r == '*' && *(r + 1) == '/')
                {
                    r += 2;
                    break;
                }

                /* Preserve newlines to keep __LINE__ synchronization intact.
                 */
                if (*r == '\n')
                {
                    *w++ = '\n';
                }

                r++;
            }

            continue;
        }

        if (*r == '/' && *(r + 1) == '/')
        {
            r += 2;
            *w++ = ' ';

            while (*r && *r != '\n')
            {
                r++;
            }

            continue;
        }

        if (*r == '"' || *r == '\'')
        {
            char q = *r;
            *w++ = *r++;

            while (*r && *r != q && *r != '\n')
            {
                if (*r == '\\' && *(r + 1))
                {
                    *w++ = *r++;
                }

                *w++ = *r++;
            }

            if (*r == q)
            {
                *w++ = *r++;
            }

            continue;
        }

        *w++ = *r++;
    }

    *w = '\0';

    wrench_free(spliced);
    return out;
}

/* ===== [ public API ] ===================================================== */

WPP_IMPL(wrench_preprocessor_p, create, (const char* base_path))
{
    time_t t = wrench_time(NULL);

    if (t == (time_t)(-1))
    {
        return NULL;
    }

    struct tm* tm_info = wrench_localtime(&t);

    if (tm_info == NULL)
    {
        return NULL;
    }

    wrench_preprocessor_p context = (
    wrench_preprocessor_p)wrench_calloc(1, sizeof(wrench_preprocessor_t));

    if (context == NULL)
    {
        return NULL;
    }

    context->base_path = wrench_strdup(base_path ? base_path : ".");

    if (context->base_path == NULL)
    {
        wrench_preprocessor_destroy(context);
        return NULL;
    }

    if (wrench_strftime(context->date_str, sizeof(context->date_str), "\"%b %d %Y\"", tm_info) == 0)
    {
        wrench_preprocessor_destroy(context);
        return NULL;
    }

    if (wrench_strftime(context->time_str, sizeof(context->time_str), "\"%H:%M:%S\"", tm_info) == 0)
    {
        wrench_preprocessor_destroy(context);
        return NULL;
    }

    //wrench_snprintf(context->current_file, sizeof(context->current_file), "<string>");
    context->current_line = 1;

    #ifndef WRENCH_PREPROCESSOR_DEFAULT_DIRECTIVE_PREFIX
    #define WRENCH_PREPROCESSOR_DEFAULT_DIRECTIVE_PREFIX '#'
    #endif
    context->directive_prefix = WRENCH_PREPROCESSOR_DEFAULT_DIRECTIVE_PREFIX;

    return context;
}

WPP_IMPL(void, destroy, (wrench_preprocessor_p context))
{
    if (context == NULL)
    {
        return;
    }

    wrench_preprocessor_macro_t* m = context->macros;

    while (m != NULL)
    {
        wrench_preprocessor_macro_t* next = m->next;

        wrench_free(m->name);
        wrench_free(m->body);

        for (int i = 0; i < m->num_params; i++)
        {
            wrench_free(m->params[i]);
        }

        wrench_free(m);
        m = next;
    }

    wrench_preprocessor_include_directory_t* d = context->include_dirs;

    while (d != NULL)
    {
        wrench_preprocessor_include_directory_t* next = d->next;

        wrench_free(d->path);
        wrench_free(d);

        d = next;
    }

    if (context->base_path != NULL)
    {
        wrench_free(context->base_path);
    }

    wrench_free(context);
}

WPP_IMPL(bool, add_include_path, (wrench_preprocessor_p context, const char* path))
{
    wrench_assert(context != NULL, "");

    wrench_preprocessor_include_directory_t* d = (
    wrench_preprocessor_include_directory_t*)wrench_malloc(sizeof(wrench_preprocessor_include_directory_t));

    if (d == NULL)
    {
        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
    }

    d->path = wrench_strdup(path);

    if (d->path == NULL)
    {
        wrench_free(d);
        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
    }

    d->next = context->include_dirs;
    context->include_dirs = d;

    return true;
}

WPP_IMPL(bool, define, (wrench_preprocessor_p context, const char* signature, const char* value))
{
    wrench_assert(context != NULL, "");

    const char* p = signature;
    const char* name_start = p;

    while (wrench_preprocessor_is_ident_part(*p))
    {
        p++;
    }

    char* macro_name = wrench_strndup(name_start, p - name_start);

    if (macro_name == NULL)
    {
        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
    }

    if (!wrench_preprocessor_undef(context, macro_name))
    {
        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
    }

    wrench_preprocessor_macro_t* m = (
    wrench_preprocessor_macro_t*)wrench_calloc(1, sizeof(wrench_preprocessor_macro_t));

    if (m == NULL)
    {
        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
    }

    m->name = macro_name;

    if (*p == '(')
    {
        m->is_function = 1;
        p++;

        while (*p && *p != ')')
        {
            wrench_preprocessor_skip_whitespace(&p);
            const char* arg_start = p;

            while (wrench_preprocessor_is_ident_part(*p) || *p == '.')
            {
                p++;
            }

            if (p > arg_start)
            {
                const char* arg_end = p;

                while (arg_end > arg_start && (arg_end[-1] == ' ' || arg_end[-1] == '\t'))
                {
                    arg_end--;
                }

                m->params[m->num_params] = wrench_strndup(arg_start, arg_end - arg_start);

                if (m->params[m->num_params] == NULL)
                {
                    WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
                }

                if (wrench_strcmp(m->params[m->num_params], "...") == 0)
                {
                    m->is_variadic = 1;
                }

                m->num_params++;
            }

            wrench_preprocessor_skip_whitespace(&p);

            if (*p == ',')
            {
                p++;
            }
        }
    }

    m->body = wrench_strdup(value ? value : "");

    if (m->body == NULL)
    {
        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
    }

    m->next = context->macros;
    context->macros = m;

    return true;
}

WPP_IMPL(bool, undef, (wrench_preprocessor_p context, const char* name))
{
    wrench_assert(context != NULL, "");
    wrench_preprocessor_macro_t** curr = &context->macros;

    while (*curr)
    {
        if (wrench_strcmp((*curr)->name, name) == 0)
        {
            wrench_preprocessor_macro_t* temp = *curr;
            *curr = (*curr)->next;

            wrench_free(temp->name);
            wrench_free(temp->body);

            for (int i = 0; i < temp->num_params; i++)
            {
                wrench_free(temp->params[i]);
            }

            wrench_free(temp);
            return true;
        }

        curr = &(*curr)->next;
    }

    return true;
}

WPP_IMPL(const char*, process_file, (wrench_preprocessor_p context, const char* filename))
{
    const char* file_contents = wrench_preprocessor_read_entire_file(context, filename);

    if (file_contents == NULL)
    {
        return NULL;
    }

    const char* result = wrench_preprocessor_process_string(context, filename, file_contents);
    wrench_free(file_contents);

    return result;
}

WPP_IMPL(const char*, process_string, (wrench_preprocessor_p context, const char* filename, const char* input))
{
    wrench_assert(context != NULL, "");
    wrench_assert(context->error_string[0] == '\0', "%s", context->error_string);

    wrench_snprintf(context->current_file, sizeof(context->current_file), "%s", filename != NULL ? filename : "<string>");
    context->current_line = 1;

    wrench_preprocessor_string_builder_t out;

    if (!wrench_preprocessor_string_builder_init(&out))
    {
        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        return NULL;
    }

    char* cleaned = wrench_preprocessor_strip_comments_and_splice(context, input);

    if (cleaned == NULL)
    {
        wrench_preprocessor_string_builder_free(&out);

        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        return NULL;
    }

    if (!wrench_preprocessor_internal(context, cleaned, &out, NULL))
    {
        wrench_preprocessor_string_builder_free(&out);
        wrench_free(cleaned);

        return NULL;
    }

    wrench_free(cleaned);

    if (context->error_string[0] != '\0')
    {
        wrench_preprocessor_string_builder_free(&out);

        WRENCH_PREPROCESSOR_TODO_PROPER_ERROR_HANDLING();
        return NULL;
    }

    /* Convert the internal marker back to real newlines for the output.
     */
    if (context->keep_spliced_lines)
    {
        for (char* ptr = out.data; *ptr; ptr++)
        {
            if (*ptr == '\x01')
            {
                *ptr = '\n';
            }
        }
    }

    return (const char*)out.data;
}

WPP_IMPL(void, set_keep_spliced_lines, (wrench_preprocessor_p context, bool enabled))
{
    wrench_assert(context != NULL, "");
    context->keep_spliced_lines = enabled;
}

WPP_IMPL(bool, get_keep_spliced_lines, (wrench_preprocessor_p context))
{
    wrench_assert(context != NULL, "");
    return context->keep_spliced_lines;
}

WPP_IMPL(void, set_error_string, (wrench_preprocessor_p context, const char* error))
{
    wrench_assert(context != NULL, "");
    wrench_snprintf(context->error_string, sizeof(context->error_string), "%s", error);
}

WPP_IMPL(const char*, get_error_string, (wrench_preprocessor_p context))
{
    wrench_assert(context != NULL, "");
    return (const char*)context->error_string;
}

WPP_IMPL(void, set_directive_prefix, (wrench_preprocessor_p context, char prefix))
{
    wrench_assert(context != NULL, "");
    context->directive_prefix = prefix;
}

WPP_IMPL(char, get_directive_prefix, (wrench_preprocessor_p context))
{
    wrench_assert(context != NULL, "");
    return context->directive_prefix;
}

#endif /* __WRENCH_PREPROCESSOR_C__ */
#endif /* WRENCH_PREPROCESSOR_IMPLEMENTATION */
