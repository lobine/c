#ifndef __robin_c_string
#define __robin_c_string


//
// Declarations
//


typedef struct string string;


external string string_make(c8 *val);
external string string_slice(string s, s32 start, s32 end);
external b8     string_equal(string s1, string s2);

external b8   string_is_alpha_char(c8 c);
external b8   string_is_digit_char(c8 c);


//
// Definitions
//


struct string {
    u32  length;
    c8  *data;
};


inline string string_slice(string s, s32 start, s32 end) {
    return (string){
        .length = end - start,
        .data   = s.data + start,
    };
}

inline string string_make(c8 *val) {
    s32 length = 0;
    while (val[length++] != '\0');
    return (string){
        .length = length,
        .data   = val,
    };
}

b8 string_equal(string s1, string s2) {
    if (s1.length != s2.length) return false;
    s32 index = s1.length;
    c8 *c1 = s1.data, *c2 = s2.data;
    while (*c1++ == *c2++ && index-- > 0);
    return index == 0;
}

inline b8 string_is_alpha_char(c8 c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

inline b8 string_is_digit_char(c8 c) {
    return '0' <= c && c <= '9';
}


#endif // __robin_c_string
