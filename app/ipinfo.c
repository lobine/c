#include "ip.h"

int main(s32 argc, c8 *argv[]) {
    if (argc != 2) {
        return 1;
    }

    ipv4_range range = ipv4_parse_range(argv[1]);
    if (!range.address) {
        return 1;
    }

    ipv4 min = ipv4_min_in_range(range);
    ipv4 max = ipv4_max_in_range(range);

    printf("IP Range: %s\n", argv[1]);

    printf("Mask: ");
    ipv4_bytes mask_bytes = ipv4_to_bytes(range.mask);
    for (s32 i = 0; i < 4; ++i) printf("%u.", mask_bytes.bytes[i]);
    printf("\n");

    printf("Wildcard: ");
    ipv4_bytes wild_bytes = ipv4_to_bytes(range.wildcard);
    for (s32 i = 0; i < 4; ++i) printf("%u.", wild_bytes.bytes[i]);
    printf("\n");

    printf("Host count: %u\n", range.wildcard + 1);

    printf("Min address: ");
    ipv4_bytes min_bytes = ipv4_to_bytes(min);
    for (s32 i = 0; i < 4; ++i) printf("%u.", min_bytes.bytes[i]);
    printf("\n");
    printf("Max address: ");
    ipv4_bytes max_bytes = ipv4_to_bytes(max);
    for (s32 i = 0; i < 4; ++i) printf("%u.", max_bytes.bytes[i]);
    printf("\n");

    return 0;
}
