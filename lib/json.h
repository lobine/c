#ifndef __robin_c_json
#define __robin_c_json


#include "c.h"
#include "string_builder.h"


//
// Declarations
//


typedef enum json_value_type json_value_type;

typedef struct json_array      json_array;

typedef struct json_value_spec json_value_spec;
typedef struct json_field_spec json_field_spec;
typedef struct json_array_spec json_array_spec;

typedef struct json_decoder json_decoder;

typedef json_array (*json_array_fn)  (json_decoder *decoder);
typedef void*      (*json_object_fn) (json_decoder *decoder);


external json_decoder* json_make_decoder(const c8 *data);
external b8            json_parse_object(json_decoder *decoder, json_field_spec *fields, s32 n_fields);
external b8            json_parse_array (json_decoder *decoder, json_array_spec *array);

external json_array json_decode_array_of_integer(json_decoder *decoder);
external json_array json_decode_array_of_float  (json_decoder *decoder);
external json_array json_decode_array_of_string (json_decoder *decoder);

internal s32 json__find_field(c8 *name, json_field_spec *fields, s32 n_fields);

internal inline b32 json__is_whitespace(const c8 c);
internal inline b32 json__is_alpha     (const c8 c);
internal inline b32 json__is_digit     (const c8 c);
internal inline b32 json__is_escapable (const c8 c);
internal inline c8  json__escaped      (const c8 c);

// @Improvement: have skip_XXX procedures that are simpler - and more efficient - that parse_XXX.
// @Bug: errors don't bubble up immediatly, other errors are printed on the way.
internal b8 json__parse_string        (json_decoder *decoder, c8 **dst);
internal b8 json__parse_number        (json_decoder *decoder, void *dst);
internal b8 json__parse_boolean       (json_decoder *decoder, b32 *dst);
internal b8 json__parse_null          (json_decoder *decoder, void **dst);
internal b8 json__parse_array         (json_decoder *decoder, json_array_spec *array);
internal b8 json__parse_object        (json_decoder *decoder, json_field_spec *fields, s32 n_fields);
internal b8 json__parse_array_field   (json_decoder *decoder, json_field_spec *field);
internal b8 json__parse_object_field  (json_decoder *decoder, json_field_spec *field);

inline internal c8   json__read   (json_decoder *decoder);
inline internal void json__back   (json_decoder *decoder);
inline internal c8   json__char   (json_decoder *decoder);

internal void json__error(json_decoder *decoder, c8 *msg);
internal void json__errorf(json_decoder *decoder, const c8 *fmt, ...);
internal b8   json__check_type(json_decoder *decoder, json_value_type expected, json_value_type spec);


//
// Definitions
//


// @Cleanup: doesn't need to be an enum
enum json_value_type {
    JSON_UNKNOWN_KIND = 0,
    JSON_STRING       = 1 << 0,
    JSON_INTEGER      = 1 << 1,
    JSON_FLOAT        = 1 << 2,
    JSON_BOOLEAN      = 1 << 3,
    JSON_NULL         = 1 << 4,
    JSON_ARRAY        = 1 << 5,
    JSON_OBJECT       = 1 << 6,
};

struct json_decoder {
    b32      root;
    const c8 *data;  // Data to parse
    s32      cursor; // To store the current position, to not expose it to callbacks
};

struct json_array {
    s32  length;
    void *data;
};

struct json_value_spec {
    json_value_type kind;
    union {
        json_object_fn object_fn;
        json_array_fn  array_fn;
    } callback;
    union {
        c8         **string;
        s32        *integer;
        f32        *real;
        b32        *boolean;
        void       **object; // @Improvement: does this have to be a pointer of pointer ?
        json_array *array;
        //
        // @Improvement: nullable values
        // s32 **nullable_int_dst;
        // f32 **nullable_float_dst;
        // b32 **nullable_bool_dst;
        //
    } target;
};

struct json_field_spec {
    c8*             name;
    json_value_spec spec;
};

struct json_array_spec {
    s32             item_size;
    json_array      *array;
    json_value_spec spec;
};


json_decoder* json_make_decoder(const c8 *data) {
    json_decoder *decoder = struct_alloc(json_decoder);
    decoder->data         = data;
    decoder->root         = true;
    decoder->cursor       = 0;
    return decoder;
}

b8 json_parse_object(json_decoder *decoder, json_field_spec *fields, s32 n_fields) {
    //
    // If we're the root object, we need to do some extra stuff:
    // - discard leading whitespaces
    // - check that the first non-whitespace is an opening bracket
    // - check that there are no non-whitespace character after the closing bracket
    //
    b32 root = decoder->root;
    c8 c;

    if (root) {
        decoder->root = false;

        decoder->cursor = -1;
        c = json__read(decoder);
        while (json__is_whitespace(c)) c = json__read(decoder);
    }

    // Put back the non-whitespace char (which should be '{') in the
    // read buffer, for json__parse_object to check.
    b8 ok = json__parse_object(decoder, fields, n_fields);
    if (!ok) return false;

    if (root) {
        c = json__read(decoder);
        while (json__is_whitespace(c)) c = json__read(decoder);
        if (c != '\0') {
            json__error(decoder, "parse object: expected end of string, but found other data");
            return false;
        }
    }

    return true;
}

b8 json_parse_array(json_decoder *decoder, json_array_spec *array) {
    //
    // If we're the root object, we need to do some extra stuff:
    // - discard leading whitespaces
    // - check that the first non-whitespace is an opening bracket
    // - check that there are no non-whitespace character after the closing bracket
    //
    b32 root = decoder->root;
    c8 c;

    if (root) {
        decoder->root = false;

        decoder->cursor = -1;
        c = json__read(decoder);
        while (json__is_whitespace(c)) c = json__read(decoder);
    }

    // Put back the non-whitespace char (which should be '[') in the
    // read buffer, for json__array_object to check.
    b8 ok = json__parse_array(decoder, array);
    if (!ok) return false;

    if (root) {
        c = json__read(decoder);
        while (json__is_whitespace(c)) c = json__read(decoder);
        if (c != '\0') {
            json__error(decoder, "parse array: expected end of string, but found other data");
            return false;
        }
    }

    return true;
}

s32 json__find_field(c8 *name, json_field_spec *fields, s32 n_fields) {
    for (s32 i = 0; i < n_fields; i++) {
        if (string_equal(fields[i].name, name)) {
            return i;
        }
    }
    return -1;
}

b32 json__is_whitespace(const c8 c) {
    switch (c) {
        case ' ': case '\t':
        case '\n': case '\r':
            return true;
    }
    return false;
}

b32 json__is_alpha(const c8 c) {
    return ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z'));
}

b32 json__is_digit(const c8 c) {
    return ('0' <= c && c <= '9');
}

// @Improvement: handle hexs (e.g. \uFFFF).
b32 json__is_escapable(const c8 c) {
    switch (c) {
        case '\\':
        case '/': case 'b': case 'f':
        case 'n': case 'r': case 't':
            return true;
    }
    return false;
}

// Return the corresponding character, assuming it follows a '\' character.
c8 json__escaped(const c8 c) {
    switch (c) {
        case '\\':
            return '\\';
        case '/':
            return '/';
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
    }
    return 0;
}

//
// This assumes the initial quote was already processed,
// and i is the index of the first character in the string.
//
b8 json__parse_string(json_decoder *decoder, c8 **dst) {
    b32 is_escaped          = false;
    string_builder *builder = string_make_builder();
    // @Improvement: check current char is '"'

    c8 c = json__read(decoder);
    while (c != '\0') {

        switch(c) {
            case '\\':
                is_escaped = true;
                break;

            case '"':
                if (is_escaped) {
                    string_write_char(builder, '"');
                } else {
                    goto end;
                }
                break;

            default:
                if (is_escaped) {
                    if(json__is_escapable(c)) {
                        string_write_char(builder, json__escaped(c));
                    } else {
                        json__error(decoder, "parse string: invalid escaped character");
                        goto end;
                    }
                } else {
                    string_write_char(builder, c);
                }
        }

        c = json__read(decoder);
    }

end:
    if (c != '"') {
        json__error(decoder, "parse string: couldn't find '\"' at the end");
        return false;
    } else if (dst) {
        // @Improvement: use custom allocator
        *dst = string_builder_to_c(builder);
    }
    string_free_builder(builder);
    return true;
}

//
// `dst` should only be a `f32*` or a `s32*`, depending on the expected type.
// Never returns an error value, but instead stops whenever the number ends,
// and let the caller detect the error (e.g. "1234a" will return 1234, and the
// caller should detect an error when finding the 'a' after a value.
//
// @Improvement: handle exponents (e.g. 1e5).
//
b8 json__parse_number(json_decoder *decoder, void *dst) {
    b32 is_fraction = false;
    s32 value = 0, sign = 1;
    f32 fract = 0.0, div = 1.0;

    c8 c = json__char(decoder);

    {
        // Handle "-" for negative numbers.
        if (c == '-') {
            sign = -1;
            c = json__read(decoder);
        }
    }

    {
        // A leading 0 can only be followed by a dot,
        // or the end of the number.
        if (c == '0') {
            c = json__read(decoder);
            if (c == '.') {
                goto fraction;
            }
            goto end;
        }
    }

    {
        // Parse the integer part.
        for (;;) {
            if (json__is_digit(c)) {
                value *= 10;
                value += c - '0';
            } else if(c == '.') {
                goto fraction;
            } else {
                goto end;
            }
            c = json__read(decoder);
        }
    }

fraction:
    {
        // Parse the the fractional part.
        is_fraction = true;
        for (;;) {
            c = json__read(decoder);
            if (!json__is_digit(c)) {
                goto end;
            }
            div   *= 10.0;
            fract += ((f32) c - '0') / div;
        }
    }

end:
    if (dst) {
        value *= sign;
        if (is_fraction) {
            *(f32*) dst = (f32) value + fract;
        } else {
            *(s32*) dst = value;
        }
    }

    // The cursor is on the next character after a valid number, let the caller handle it.
    json__back(decoder);
    return true;
}

// Errors out if anything else than "true" or "false".
b8 json__parse_boolean(json_decoder *decoder, b32 *dst) {
    c8 c = json__char(decoder);
    c8 *bool_str;
    c8 bool_len;
    b32 bool_val;
    if (c == 't') {
        bool_val = true;
        bool_str = "true";
        bool_len = 4;
    } else if (c == 'f') {
        bool_val = false;
        bool_str = "false";
        bool_len = 5;
    } else {
        json__error(decoder, "parse boolean: value does not start with 't' not 'f'");
        return false;
    }

    for (s32 j = 0; j < bool_len; j++) {
        if (j > 0) c = json__read(decoder);
        if (c != bool_str[j]) {
            json__errorf(decoder, "parse boolean: expected \"%s\" but found unexpected character", bool_str);
            return false;
        }
    }

    if (dst) *dst = bool_val;
    return true;
}

// Errors out if anything else that "null".
b8 json__parse_null(json_decoder *decoder, void **dst) {
    c8 c;
    c8 *null_str = "null";
    json__back(decoder);
    for (s32 j = 0; j < 4; j++) {
        c = json__read(decoder);
        if (c != null_str[j]) {
            json__errorf(decoder, "parse null: expected \"%s\" but found unexpected character", null_str);
            return false;
        }
    }

    if (dst) *dst = NULL;
    return true;
}

b8 json__parse_array_field(json_decoder *decoder, json_field_spec *field) {
    c8 c = json__char(decoder);
    if (c != '[') {
        json__error(decoder, "parse array field: missing opening bracket");
        return false;
    }

    if (!(field->spec.kind & JSON_ARRAY)) {
        json__error(decoder, "parse array field: did not expect array");
        return false;
    }
    // @Cleanup: error and return instead
    assert(field->spec.callback.array_fn);
    assert(field->spec.target.array);

    *field->spec.target.array = field->spec.callback.array_fn(decoder);
    return true;
}

// @Improvement: only pass the json_value_spec
b8 json__parse_object_field(json_decoder *decoder, json_field_spec *field) {
    c8 c = json__char(decoder);
    if (c != '{') {
        json__error(decoder, "parse object field: missing opening bracket");
        return false;
    }

    // @Cleanup: error and return instead
    assert(field->spec.kind & JSON_OBJECT);
    assert(field->spec.callback.object_fn);
    assert(field->spec.target.object);

    *field->spec.target.object = field->spec.callback.object_fn(decoder);
    return true;
}

//
// `i` should point to the opening bracket.
//
// @Improvement: handle additional fields
// @Bug: check for unexpected types
// @Bug: handle end-of-string
//
b8 json__parse_object(json_decoder *decoder, json_field_spec *fields, s32 n_fields) {
    c8 c = json__char(decoder);
    if (c != '{') {
        json__error(decoder, "parse object: missing opening bracket");
        return false;
    }

    c = json__read(decoder);

    // State machine, represents what we expect next.
    // First item should be a key.
    s32 comma = 0, colon = 1, key = 2, value = 3;
    s32 state = key;

    b8 ok;

    c8 *field_name;
    // @Improvement: store `dst` directly to avoid the `if (field) ...` checks.
    json_field_spec *field;

    while(c != '}') {
        if (json__is_whitespace(c)) {
            c = json__read(decoder);
            continue;
        }

        if (state == comma) {

            if (c != ',') {
                json__error(decoder, "parse object: expected ','");
                return false;
            }
            state = key;
                
        } else if (state == colon) {

            if (c != ':') {
                json__error(decoder, "parse object: expected ':'");
                return false;
            }
            state = value;

        } else if (state == key) {

            if (c != '"') {
                json__error(decoder, "parse object: expected '\"'");
                return false;
            }
            ok = json__parse_string(decoder, &field_name);
            if (ok) {
                s32 field_idx = json__find_field(field_name, fields, n_fields);
                if (field_idx >= 0) {
                    field = &fields[field_idx];
                } else {
                    field = NULL;
                }
            }

            state = colon;

        } else if (state == value) {

            switch (c) {
                case '"':
                    if (field) {
						if (!json__check_type(decoder, JSON_STRING, field->spec.kind)) return false;
                        ok = json__parse_string(decoder, field->spec.target.string);
                    } else {
                        ok = json__parse_string(decoder, NULL);
                    }
                    break;

                case '-': case '0':
                case '1': case '2': case '3':
                case '4': case '5': case '6':
                case '7': case '8': case '9':
                    if (field) {
                        if (field->spec.kind & JSON_INTEGER) {
							if (!json__check_type(decoder, JSON_INTEGER, field->spec.kind)) return false;
                            ok = json__parse_number(decoder, field->spec.target.integer);
                        } else {
							if (!json__check_type(decoder, JSON_FLOAT, field->spec.kind)) return false;
                            ok = json__parse_number(decoder, field->spec.target.real);
                        }
                    } else {
                        ok = json__parse_number(decoder, NULL);
                    }
                    break;

                case 't': case 'f':
                    if (field) {
						if (!json__check_type(decoder, JSON_BOOLEAN, field->spec.kind)) return false;
                        ok = json__parse_boolean(decoder, field->spec.target.boolean);
                    } else {
                        ok = json__parse_boolean(decoder, NULL);
                    }
                    break;

                case 'n':
                    if (field) {
						if (!json__check_type(decoder, JSON_OBJECT | JSON_ARRAY, field->spec.kind)) return false;
						// @Bug: if it's an array, handle properly and set lenght to 0
                        ok = json__parse_null(decoder, field->spec.target.object);
                    } else {
                        ok = json__parse_null(decoder, NULL);
                    }
                    break;

                case '{':
                    // @Improvement: handle `dst == NULL` in `parse_compound_field` and
                    // factorize objects and arrays.
                    if (field) {
						if (!json__check_type(decoder, JSON_OBJECT, field->spec.kind)) return false;
                        ok = json__parse_object_field(decoder, field);
                    } else {
                        // Skip the object (parse it but don't store it anywhere).
                        ok = json__parse_object(decoder, NULL, 0);
                    }
                    break;

                case '[':
                    if (field) {
						if (!json__check_type(decoder, JSON_ARRAY, field->spec.kind)) return false;
                        ok = json__parse_array_field(decoder, field);
                    } else {
                        // Skip the array (parse it but don't store it anywhere).
                        ok = json__parse_array(decoder, NULL);
                    }
                    break;

                default:
                    json__error(decoder, "parse object: invalid value");
                    return false;
            }

            state = comma;
        }

        // If any parse_XXX failed...
        if (!ok) {
            return false;
        }
        c = json__read(decoder);
    }

    if (state != comma) {
        json__error(decoder, "parse object: unexpected end of object");
        return false;
    }

    // Return the index of the first character after the closing bracket. 
    return true;
}

//
// `i` should point to the opening bracket.
//
// @Improvement: avoid having to `if (array) ...` in every case. 
// @Bug: check for unexpected types
// @Bug: handle end-of-string
//
b8 json__parse_array(json_decoder *decoder, json_array_spec *array) {
    c8 c = json__char(decoder);
    if (c != '[') {
        json__error(decoder, "parse array: missing opening bracket");
        return false;
    }

    c = json__read(decoder);

    // We expect values and commas to alternate, so we keep
    // track of that here. First item should be a value.
    s32 value = 0, comma = 1;
    s32 next = value;

    b8 ok;

    s32 len = 0;
    s32 cap = 10;
    // @Improvement: free array on error
    void *items = NULL, *item_ptr;
    json_field_spec item_field; // Used for objects and arrays in the array.

    while(c != ']') {
        if (json__is_whitespace(c)) {
            c = json__read(decoder);
            continue;
        }

        {
            // Allocate and grow the destination array.

            if (array && next == value) {

                if (!items) {
                    // @Improvement: allow custom allocator
                    items = memory_alloc(array->item_size * cap);
                }

                // Grow the array - double the capacity everytime.
                if (len >= cap) {
                    s32 new_cap = 2 * cap;
                    void *new_items = memory_alloc(array->item_size * new_cap);
                    memory_copy(new_items, array, array->item_size * len);
                    free(items);
                    items = new_items;
                    cap   = new_cap;
                }

                // Points to the last spot in the array.
                item_ptr = items + array->item_size * len++;
            }
        }

        if (next == value) {

            switch (c) {
                case '"':
                    if (array) {
						if (!json__check_type(decoder, JSON_STRING, array->spec.kind)) return false;
                        ok = json__parse_string(decoder, item_ptr);
                    } else {
                        ok = json__parse_string(decoder, NULL);
                    }
                    break;

                case '-': case '0':
                case '1': case '2': case '3':
                case '4': case '5': case '6':
                case '7': case '8': case '9':
                    if (array) {
						if (!json__check_type(decoder, JSON_INTEGER | JSON_FLOAT, array->spec.kind)) return false;
                        ok = json__parse_number(decoder, (void*) item_ptr);
                    } else {
                        ok = json__parse_number(decoder, NULL);
                    }
                    break;

                case 't': case 'f':
                    if (array) {
						if (!json__check_type(decoder, JSON_BOOLEAN, array->spec.kind)) return false;
                        ok = json__parse_boolean(decoder, (b32*) item_ptr);
                    } else {
                        ok = json__parse_boolean(decoder, NULL);
                    }
                    break;

                case 'n':
                    if (array) {
						if (!json__check_type(decoder, JSON_OBJECT | JSON_ARRAY, array->spec.kind)) return false;
						// @Bug: if it's an array, handle properly and set lenght to 0
                        ok = json__parse_null(decoder, (void**) item_ptr);
                    } else {
                        ok = json__parse_null(decoder, NULL);
                    }
                    break;

                case '{':
                    // @Improvement: factorize objects and arrays
                    if (array) {
						if (!json__check_type(decoder, JSON_OBJECT, array->spec.kind)) return false;
                        item_field.spec.kind = array->spec.kind;
                        item_field.spec.callback.object_fn = array->spec.callback.object_fn;
                        item_field.spec.target.object      = (void**) item_ptr;
                        ok = json__parse_object_field(decoder, &item_field);
                    } else {
                        ok = json__parse_object(decoder, NULL, 0);
                    }
                    break;

                case '[':
                    if (array) {
						if (!json__check_type(decoder, JSON_ARRAY, array->spec.kind)) return false;
                        item_field.spec.kind = array->spec.kind;
                        item_field.spec.callback.array_fn = array->spec.callback.array_fn;
                        item_field.spec.target.array      = (json_array*) item_ptr;
                        ok = json__parse_array_field(decoder, &item_field);
                    } else {
                        ok = json__parse_array(decoder, NULL);
                    }
                    break;

                default:
                    json__error(decoder, "parse array: invalid value");
                    return false;
            }

            next = comma;

        } else {

            if (c != ',') {
                json__error(decoder, "parse array: expected ','");
                return false;
            }
            next = value;

        }

        // If any json__parse_XXX failed...
        if (!ok) {
            return false;
        }
        c = json__read(decoder);
    }

    // We should end with a value
    if (next == value && len > 0) {
        json__error(decoder, "parse array: unexpected end of array");
        return false;
    }

    if (array && array->array) {
        // @Improvement: allocate an array of the right size, and free the temporary one.
        array->array->data = items;
        array->array->length = len;
    }

    // Return the index of the first character after the closing bracket.
    return true;
}

//
// Reusable json_array_fn
//

json_array json_decode_array_of_integer(json_decoder *decoder) {
	json_array integers = { .length=0, .data=NULL };
	json_array_spec array = {
		.item_size=sizeof(s32),
		.array=&integers,
		.spec={ .kind=JSON_INTEGER },
	};
    // @Improvement: free array on error
	if (!json_parse_array(decoder, &array)) integers.length = -1;
	return integers;
}

json_array json_decode_array_of_float(json_decoder *decoder) {
	json_array floats = { .length=0, .data=NULL };
	json_array_spec array = {
		.item_size=sizeof(f32),
		.array=&floats,
		.spec={ .kind=JSON_FLOAT },
	};
    // @Improvement: free array on error
	if (!json_parse_array(decoder, &array)) floats.length = -1;
	return floats;
}

json_array json_decode_array_of_string(json_decoder *decoder) {
	json_array strings = { .length=0, .data=NULL };
	json_array_spec array = {
		.item_size=sizeof(c8*),
		.array=&strings,
		.spec={ .kind=JSON_STRING },
	};
    // @Improvement: free array on error
	if (!json_parse_array(decoder, &array)) strings.length = -1;
	return strings;
}

c8 json__read(json_decoder *decoder) {
    return decoder->data[++decoder->cursor];
}

void json__back(json_decoder *decoder) {
    decoder->cursor--;
}

c8 json__char(json_decoder *decoder) {
    return decoder->data[decoder->cursor];
}

b8 json__check_type(json_decoder *decoder, json_value_type expected, json_value_type spec) {
    if (!(expected & spec)) {
        json__errorf(decoder, "unexpected type found: expected one of %d but got %d", expected, spec);
        return false;
    }
    return true;
}

// @Improvement: make verbosity configurable
void json__error(json_decoder *decoder, c8 *msg) {
    printf("json: %s, at %d\n", msg, decoder->cursor);

    const c8 *data = decoder->data;

    s32 line = decoder->cursor;
    while (line > 0 && data[line] != '\n') line--;

    // Print previous line
    if (line > 0) {
        line++;
        s32 previous_line = line - 2;
        while (previous_line > 0 && data[previous_line] != '\n') previous_line--;
        if (previous_line > 0) {
			printf("...\n");
			previous_line++;
		}

        while (data[previous_line] != '\n') {
            if (data[previous_line] == '\t') {
                printf("  ");
            } else {
                printf("%c", data[previous_line]);
            }
            previous_line++;
        }
        printf("\n");
    }

    // Print current line, and the arrow below
    s32 idx = 0, len;
    while (data[line] != '\n' && data[line] != '\0') {
        if (line == decoder->cursor) len = idx;
        if (data[line] == '\t') {
            printf("  ");
            idx += 2;
        } else {
            printf("%c", data[line]);
            idx += 1;
        }
        line++;
    }
    printf("\n");

    for (s32 i = 0; i < len; i++) {
        printf(" ");
    }
    printf("^\n");

    // Print the following line
    if (data[line] == '\n') {
        line++;
        while (data[line] != '\n' && data[line] != '\0') {
            if (data[line] == '\t') {
                printf("  ");
            } else {
                printf("%c", data[line]);
            }
            line++;
        }
        printf("\n");
		if (data[line] == '\n') printf("...\n");
    }
}

void json__errorf(json_decoder *decoder, const c8 *fmt, ...) {
    c8 msg[1024];
    va_list arglist;
    va_start(arglist, fmt);
    vsprintf(msg, fmt, arglist);
    va_end(arglist);
    json__error(decoder, msg);
}


#endif // __robin_c_json
