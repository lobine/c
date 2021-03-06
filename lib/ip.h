#ifndef __robin_c_ip
#define __robin_c_ip


#include "c.h"


typedef u32               ipv4;
typedef union ipv4_b      ipv4_b;
typedef struct ipv4_range ipv4_range;

external ipv4       ipv4_parse_address(const c8 *s);
external ipv4_b     ipv4_bytes(ipv4 address);
external ipv4_range ipv4_parse_range(const c8 *s);
external ipv4       ipv4_min_in_range(ipv4_range range);
external ipv4       ipv4_max_in_range(ipv4_range range);


struct ipv4_range {
    ipv4 address;
    ipv4 mask;
    ipv4 wildcard;
};

union ipv4_b {
    u8 bytes[4];
    struct {
        u8 a;
        u8 b;
        u8 c;
        u8 d;
    } abcd;
};

ipv4 ipv4_parse_address(const c8 *s) {
    s32 len = 0;
    while (s[len] != 0) len++;

    u8 bytes[4] = {0, 0, 0, 0};
    s32 b = 0;
    for (s32 i = 0; i < len; ++i) {
        c8 c = s[i];
        if ('0' <= c && c <= '9') {
            bytes[b] *= 10;
            bytes[b] += c - '0';
        } else if (c == '.') {
            if (b >= 3) return 0; // @Error
            b++;
        }  else {
            return 0; // @Error
        }
    }

    ipv4 result;
    result = bytes[0];
    result <<= 8;
    result += bytes[1];
    result <<= 8;
    result += bytes[2];
    result <<= 8;
    result += bytes[3];
    return result;
}

ipv4_b ipv4_bytes(ipv4 address) {
    ipv4_b result;
    result.abcd.a = (address & 0xFF000000) >> 3 * 8;
    result.abcd.b = (address & 0x00FF0000) >> 2 * 8;
    result.abcd.c = (address & 0x0000FF00) >> 1 * 8;
    result.abcd.d = (address & 0x000000FF);
    return result;
}

ipv4_range ipv4_parse_range(const c8 *s) {
    ipv4_range result = { .address = 0, .mask = 0, .wildcard = 0 };

    s32 len = 0;
    while (s[len] != 0) len++;

    u8 bytes[5] = {0, 0, 0, 0, 32};
    s32 b = 0;
    for (s32 i = 0; i < len; ++i) {
        c8 c = s[i];
        if ('0' <= c && c <= '9') {
            bytes[b] *= 10;
            bytes[b] += c - '0';
        } else if (c == '.') {
            if (b >= 3) return result; // @Error
            b++;
        } else if (c == '/' && b == 3) {
            bytes[4] = 0;
            b++;
        } else {
            return result; // @Error
        }
    }

    result.address = bytes[0];
    result.address <<= 8;
    result.address += bytes[1];
    result.address <<= 8;
    result.address += bytes[2];
    result.address <<= 8;
    result.address += bytes[3];

    if (bytes[4] == 32) result.wildcard = 0;
    else result.wildcard = 0xFFFFFFFF >> bytes[4];

    result.mask     = ~result.wildcard;

    return result;
}

ipv4 ipv4_min_in_range(ipv4_range range) {
    return range.address & range.mask;
}

ipv4 ipv4_max_in_range(ipv4_range range) {
    return range.address | range.wildcard;
}


#endif // __robin_c_ip
