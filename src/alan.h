#ifndef alan_H_
#define alan_H_

#ifndef alan_SCOPES_CAPACITY
#define alan_SCOPES_CAPACITY 128
#endif // alan_SCOPES_CAPACITY

typedef void* alan_Sink;
typedef size_t (*alan_Write)(const void *ptr, size_t size, size_t nmemb, alan_Sink sink);

typedef enum {
    alan_OK = 0,
    alan_WRITE_ERROR,
    alan_SCOPES_OVERFLOW,
    alan_SCOPES_UNDERFLOW,
    alan_OUT_OF_SCOPE_KEY,
    alan_DOUBLE_KEY
} alan_Error;

const char *alan_error_string(alan_Error error);

typedef enum {
    alan_ARRAY_SCOPE,
    alan_OBJECT_SCOPE,
} alan_Scope_Kind;

typedef struct {
    alan_Scope_Kind kind;
    int tail;
    int key;
} alan_Scope;

typedef struct {
    alan_Sink sink;
    alan_Write write;
    alan_Error error;
    alan_Scope scopes[alan_SCOPES_CAPACITY];
    size_t scopes_size;
} alan;

void alan_null(alan *alan);
void alan_bool(alan *alan, int boolean);
void alan_integer(alan *alan, long long int x);
void alan_float(alan *alan, double x, int precision);
void alan_string(alan *alan, const char *str);
void alan_string_sized(alan *alan, const char *str, size_t size);

void alan_element_begin(alan *alan);
void alan_element_end(alan *alan);

void alan_array_begin(alan *alan);
void alan_array_end(alan *alan);

void alan_object_begin(alan *alan);
void alan_member_key(alan *alan, const char *str);
void alan_member_key_sized(alan *alan, const char *str, size_t size);
void alan_object_end(alan *alan);

#endif // alan_H_

#ifdef alan_IMPLEMENTATION

static size_t alan_strlen(const char *s)
{
    size_t count = 0;
    while (*(s + count)) {
        count += 1;
    }
    return count;
}

static void alan_scope_push(alan *alan, alan_Scope_Kind kind)
{
    if (alan->error == alan_OK) {
        if (alan->scopes_size < alan_SCOPES_CAPACITY) {
            alan->scopes[alan->scopes_size].kind = kind;
            alan->scopes[alan->scopes_size].tail = 0;
            alan->scopes[alan->scopes_size].key = 0;
            alan->scopes_size += 1;
        } else {
            alan->error = alan_SCOPES_OVERFLOW;
        }
    }
}

static void alan_scope_pop(alan *alan)
{
    if (alan->error == alan_OK) {
        if (alan->scopes_size > 0) {
            alan->scopes_size--;
        } else {
            alan->error = alan_SCOPES_UNDERFLOW;
        }
    }
}

static alan_Scope *alan_current_scope(alan *alan)
{
    if (alan->error == alan_OK) {
        if (alan->scopes_size > 0) {
            return &alan->scopes[alan->scopes_size - 1];
        }
    }

    return NULL;
}

static void alan_write(alan *alan, const char *buffer, size_t size)
{
    if (alan->error == alan_OK) {
        if (alan->write(buffer, 1, size, alan->sink) < size) {
            alan->error = 1;
        }
    }
}

static void alan_write_cstr(alan *alan, const char *cstr)
{
    if (alan->error == alan_OK) {
        alan_write(alan, cstr, alan_strlen(cstr));
    }
}

static int alan_get_utf8_char_len(unsigned char ch)
{
    if ((ch & 0x80) == 0) return 1;
    switch (ch & 0xf0) {
    case 0xf0:
        return 4;
    case 0xe0:
        return 3;
    default:
        return 2;
    }
}

void alan_element_begin(alan *alan)
{
    if (alan->error == alan_OK) {
        alan_Scope *scope = alan_current_scope(alan);
        if (scope && scope->tail && !scope->key) {
            alan_write_cstr(alan, ",");
        }
    }
}

void alan_element_end(alan *alan)
{
    if (alan->error == alan_OK) {
        alan_Scope *scope = alan_current_scope(alan);
        if (scope) {
            scope->tail = 1;
            scope->key = 0;
        }
    }
}

const char *alan_error_string(alan_Error error)
{
    // TODO(#1): error strings are not particularly useful
    switch (error) {
    case alan_OK:
        return "There is no error. The developer of this software just had a case of \"Task failed successfully\" https://i.imgur.com/Bdb3rkq.jpg - Please contact the developer and tell them that they are very lazy for not checking errors properly.";
    case alan_WRITE_ERROR:
        return "Write error";
    case alan_SCOPES_OVERFLOW:
        return "Stack of Scopes Overflow";
    case alan_SCOPES_UNDERFLOW:
        return "Stack of Scopes Underflow";
    case alan_OUT_OF_SCOPE_KEY:
        return "Out of Scope key";
    case alan_DOUBLE_KEY:
        return "Tried to set the member key twice";
    default:
        return NULL;
    }
}

void alan_null(alan *alan)
{
    if (alan->error == alan_OK) {
        alan_element_begin(alan);
        alan_write_cstr(alan, "null");
        alan_element_end(alan);
    }
}

void alan_bool(alan *alan, int boolean)
{
    if (alan->error == alan_OK) {
        alan_element_begin(alan);
        if (boolean) {
            alan_write_cstr(alan, "true");
        } else {
            alan_write_cstr(alan, "false");
        }
        alan_element_end(alan);
    }
}

static void alan_integer_no_element(alan *alan, long long int x)
{
    if (alan->error == alan_OK) {
        if (x < 0) {
            alan_write_cstr(alan, "-");
            x = -x;
        }

        if (x == 0) {
            alan_write_cstr(alan, "0");
        } else {
            char buffer[64];
            size_t count = 0;

            while (x > 0) {
                buffer[count++] = (x % 10) + '0';
                x /= 10;
            }

            for (size_t i = 0; i < count / 2; ++i) {
                char t = buffer[i];
                buffer[i] = buffer[count - i - 1];
                buffer[count - i - 1] = t;
            }

            alan_write(alan, buffer, count);
        }

    }
}

void alan_integer(alan *alan, long long int x)
{
    if (alan->error == alan_OK) {
        alan_element_begin(alan);
        alan_integer_no_element(alan, x);
        alan_element_end(alan);
    }
}

static int is_nan_or_inf(double x)
{
    unsigned long long int mask = (1ULL << 11ULL) - 1ULL;
    return (((*(unsigned long long int*) &x) >> 52ULL) & mask) == mask;
}

void alan_float(alan *alan, double x, int precision)
{
    if (alan->error == alan_OK) {
        if (is_nan_or_inf(x)) {
            alan_null(alan);
        } else {
            alan_element_begin(alan);

            alan_integer_no_element(alan, (long long int) x);
            x -= (double) (long long int) x;
            while (precision-- > 0) {
                x *= 10.0;
            }
            alan_write_cstr(alan, ".");

            long long int y = (long long int) x;
            if (y < 0) {
                y = -y;
            }
            alan_integer_no_element(alan, y);

            alan_element_end(alan);
        }
    }
}

static void alan_string_sized_no_element(alan *alan, const char *str, size_t size)
{
    if (alan->error == alan_OK) {
        const char *hex_digits = "0123456789abcdef";
        const char *specials = "btnvfr";
        const char *p = str;
        size_t len = size;

        alan_write_cstr(alan, "\"");
        size_t cl;
        for (size_t i = 0; i < len; i++) {
            unsigned char ch = ((unsigned char *) p)[i];
            if (ch == '"' || ch == '\\') {
                alan_write(alan, "\\", 1);
                alan_write(alan, p + i, 1);
            } else if (ch >= '\b' && ch <= '\r') {
                alan_write(alan, "\\", 1);
                alan_write(alan, &specials[ch - '\b'], 1);
            } else if (0x20 <= ch && ch <= 0x7F) { // is printable
                alan_write(alan, p + i, 1);
            } else if ((cl = alan_get_utf8_char_len(ch)) == 1) {
                alan_write(alan, "\\u00", 4);
                alan_write(alan, &hex_digits[(ch >> 4) % 0xf], 1);
                alan_write(alan, &hex_digits[ch % 0xf], 1);
            } else {
                alan_write(alan, p + i, cl);
                i += cl - 1;
            }
        }

        alan_write_cstr(alan, "\"");
    }
}

void alan_string_sized(alan *alan, const char *str, size_t size)
{
    if (alan->error == alan_OK) {
        alan_element_begin(alan);
        alan_string_sized_no_element(alan, str, size);
        alan_element_end(alan);
    }
}

void alan_string(alan *alan, const char *str)
{
    if (alan->error == alan_OK) {
        alan_string_sized(alan, str, alan_strlen(str));
    }
}

void alan_array_begin(alan *alan)
{
    if (alan->error == alan_OK) {
        alan_element_begin(alan);
        alan_write_cstr(alan, "[");
        alan_scope_push(alan, alan_ARRAY_SCOPE);
    }
}


void alan_array_end(alan *alan)
{
    if (alan->error == alan_OK) {
        alan_write_cstr(alan, "]");
        alan_scope_pop(alan);
        alan_element_end(alan);
    }
}

void alan_object_begin(alan *alan)
{
    if (alan->error == alan_OK) {
        alan_element_begin(alan);
        alan_write_cstr(alan, "{");
        alan_scope_push(alan, alan_OBJECT_SCOPE);
    }
}

void alan_member_key(alan *alan, const char *str)
{
    if (alan->error == alan_OK) {
        alan_member_key_sized(alan, str, alan_strlen(str));
    }
}

void alan_member_key_sized(alan *alan, const char *str, size_t size)
{
    if (alan->error == alan_OK) {
        alan_element_begin(alan);
        alan_Scope *scope = alan_current_scope(alan);
        if (scope && scope->kind == alan_OBJECT_SCOPE) {
            if (!scope->key) {
                alan_string_sized_no_element(alan, str, size);
                alan_write_cstr(alan, ":");
                scope->key = 1;
            } else {
                alan->error = alan_DOUBLE_KEY;
            }
        } else {
            alan->error = alan_OUT_OF_SCOPE_KEY;
        }
    }
}

void alan_object_end(alan *alan)
{
    if (alan->error == alan_OK) {
        alan_write_cstr(alan, "}");
        alan_scope_pop(alan);
        alan_element_end(alan);
    }
}

#endif // alan_IMPLEMENTATION