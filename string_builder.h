#ifndef __robin_c_string_builder
#define __robin_c_string_builder


#include "c.h"


#ifndef X_STRING_BUFFER_SIZE 
#define X_STRING_BUFFER_SIZE (1024)
#endif


//
// Declarations
//


typedef struct string_buffer string_buffer;
typedef struct string_builder string_builder;


external s32 string_length(c8 *str);
external b32 string_equal (c8 *s1, c8 *s2);

external string_builder* string_make_builder(void);
external void            string_free_builder(string_builder *builder);
external s32             string_write_n     (string_builder *builder, c8 *str, s32 n);
external s32             string_write       (string_builder *builder, c8 *str);
external s32             string_write_char  (string_builder *builder, c8 c);
external void            string_copy_builder(string_builder *builder, c8 *dst);
external c8*             string_builder_to_c(string_builder *builder);


//
// Definitions
//


struct string_buffer {
    c8            data[X_STRING_BUFFER_SIZE];
    string_buffer *next;
};

struct string_builder {
    s32           len;
    string_buffer buffer;
    string_buffer *current;
};


s32 string_length(c8 *s) {
    s32 len = 0;
    while(s[len] != 0) len++;
    return len;
}

b32 string_equal(c8 *s1, c8 *s2) {
    s32 l1 = string_length(s1);
    s32 l2 = string_length(s2);
    if (l1 != l2) {
        return false;
    }
    for (s32 i = 0; i < l1; i++) {
        if (s1[i] != s2[i]) return false;
    }
    return true;
}

string_builder* string_make_builder(void) {
    string_builder *builder = struct_alloc(string_builder);
    builder->len             = 0;
    builder->buffer.next     = NULL;
    builder->current         = NULL;
    return builder;
}

void string_free_builder(string_builder *builder) {
    string_buffer *current = builder->buffer.next, *next = NULL;
    while (current) {
        next = current->next;
        free(current);
        current = next;
    }
    free(builder);
}

s32 string_write_n(string_builder *builder, c8 *src, s32 n) {
    string_buffer *buffer = builder->current;
    if (!buffer) {
        buffer = &builder->buffer;
    }

    s32 global_cursor = builder->len;
    s32 buffer_cursor;
    s32 total_written_len = 0;
    s32 write_len;

    while (total_written_len < n) {
        buffer_cursor = global_cursor % X_STRING_BUFFER_SIZE;
        if (buffer_cursor == 0 && global_cursor > 0) {
            /*
            If the current buffer is full, allocate a new one.
            This is expected to occur every iterations except the first one.
            */
            if (buffer->next == NULL) {
                buffer->next = struct_alloc(string_buffer);
                buffer->next->next = NULL;
            }
            buffer = buffer->next;
            builder->current = buffer;
        }

        write_len = n - total_written_len;
        if (buffer_cursor + write_len > X_STRING_BUFFER_SIZE) {
            write_len = X_STRING_BUFFER_SIZE - buffer_cursor;
        }
        memory_copy(buffer->data + buffer_cursor, src + total_written_len, write_len);

        global_cursor += write_len;
        total_written_len += write_len;
    }

    assert(total_written_len == n);

    builder->len = global_cursor;
    return total_written_len;
}

s32 string_write(string_builder *builder, c8 *str) {
    s32 len = string_length(str);
    return string_write_n(builder, str, len);
}

s32 string_write_char(string_builder *builder, c8 c) {
    string_buffer *buffer = builder->current;
    if (!buffer) {
        buffer = &builder->buffer;
    }

    s32 global_cursor = builder->len;
    s32 buffer_cursor = global_cursor % X_STRING_BUFFER_SIZE;
    if (buffer_cursor == 0 && global_cursor > 0) {
        /*
         * The current buffer is full, allocate a new one.
         */
        if (buffer->next == NULL) {
            buffer->next = struct_alloc(string_buffer);
            buffer->next->next = NULL;
        }
        buffer = buffer->next;
        builder->current = buffer;
    }

    buffer->data[buffer_cursor] = c;
    builder->len += 1;
    return 1;
}

void string_copy_builder(string_builder *builder, c8 *dst) {
    string_buffer *buffer = &builder->buffer;
    s32 write_index = 0, write_len;
    s32 to_write = builder->len;

    while (to_write > 0) {
        assert(buffer != NULL);

        write_len = to_write;
        if (write_len > X_STRING_BUFFER_SIZE) {
            write_len = X_STRING_BUFFER_SIZE;
        }
        memory_copy(dst + write_index, buffer->data, write_len);

        write_index += write_len;
        to_write    -= write_len;

        buffer = buffer->next;
    }

    dst[write_index] = 0;
}

c8* string_builder_to_c(string_builder *builder) {
    c8 *string = array_alloc(builder->len + 1, c8);
    string_copy_builder(builder, string);
    return string;
}

#endif // __robin_c_string_builder
