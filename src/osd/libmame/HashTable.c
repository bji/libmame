/** **************************************************************************
 * HashTable.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/


#include "HashTable.h"

namespace Hash
{
#if 0
    /* To fix emacs indentation */
}
#endif

/* static */
unsigned int Util::Hash(const char *key, int length)
{
    uint32_t a, b, c;

    const unsigned char *k = (const unsigned char *) key;

    a = b = c = 0xdeadbeef + ((uint32_t) length);

#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

    static uint32_t endianness_indicator = 0x1;
    if (((unsigned char *) &endianness_indicator)[0]) {
        while (length > 12) {
            a += k[0];
            a += ((uint32_t) k[1]) << 8;
            a += ((uint32_t) k[2]) << 16;
            a += ((uint32_t) k[3]) << 24;
            b += k[4];
            b += ((uint32_t) k[5]) << 8;
            b += ((uint32_t) k[6]) << 16;
            b += ((uint32_t) k[7]) << 24;
            c += k[8];
            c += ((uint32_t) k[9]) << 8;
            c += ((uint32_t) k[10]) << 16;
            c += ((uint32_t) k[11]) << 24;
            a -= c; a ^= rot(c, 4);  c += b;
            b -= a; b ^= rot(a, 6);  a += c;
            c -= b; c ^= rot(b, 8);  b += a;
            a -= c; a ^= rot(c, 16);  c += b;
            b -= a; b ^= rot(a, 19);  a += c;
            c -= b; c ^= rot(b, 4);  b += a;
            length -= 12;
            k += 12;
        }
        
        switch(length) {
        case 12: c += ((uint32_t) k[11]) << 24;
        case 11: c += ((uint32_t) k[10]) << 16;
        case 10: c += ((uint32_t) k[9]) << 8;
        case 9 : c += k[8];
        case 8 : b += ((uint32_t) k[7]) << 24;
        case 7 : b += ((uint32_t) k[6]) << 16;
        case 6 : b += ((uint32_t) k[5]) << 8;
        case 5 : b += k[4];
        case 4 : a += ((uint32_t) k[3]) << 24;
        case 3 : a += ((uint32_t) k[2]) << 16;
        case 2 : a += ((uint32_t) k[1]) << 8;
        case 1 : a += k[0];
        case 0 : break;
        }
    }
    else {
        while (length > 12) {
            a += ((uint32_t) k[0]) << 24;
            a += ((uint32_t) k[1]) << 16;
            a += ((uint32_t) k[2]) << 8;
            a += ((uint32_t) k[3]);
            b += ((uint32_t) k[4]) << 24;
            b += ((uint32_t) k[5]) << 16;
            b += ((uint32_t) k[6]) << 8;
            b += ((uint32_t) k[7]);
            c += ((uint32_t) k[8]) << 24;
            c += ((uint32_t) k[9]) << 16;
            c += ((uint32_t) k[10]) << 8;
            c += ((uint32_t) k[11]);
            a -= c; a ^= rot(c, 4);  c += b;
            b -= a; b ^= rot(a, 6);  a += c;
            c -= b; c ^= rot(b, 8);  b += a;
            a -= c; a ^= rot(c, 16);  c += b;
            b -= a; b ^= rot(a, 19);  a += c;
            c -= b; c ^= rot(b, 4);  b += a;
            length -= 12;
            k += 12;
        }

        switch(length) {
        case 12: c += k[11];
        case 11: c += ((uint32_t) k[10]) << 8;
        case 10: c += ((uint32_t) k[9]) << 16;
        case 9 : c += ((uint32_t) k[8]) << 24;
        case 8 : b += k[7];
        case 7 : b += ((uint32_t) k[6]) << 8;
        case 6 : b += ((uint32_t) k[5]) << 16;
        case 5 : b += ((uint32_t) k[4]) << 24;
        case 4 : a += k[3];
        case 3 : a += ((uint32_t) k[2]) << 8;
        case 2 : a += ((uint32_t) k[1]) << 16;
        case 1 : a += ((uint32_t) k[0]) << 24;
        case 0 : break;
        }
    }
    
    c ^= b; c -= rot(b, 14);
    a ^= c; a -= rot(c, 11);
    b ^= a; b -= rot(a, 25);
    c ^= b; c -= rot(b, 16);
    a ^= c; a -= rot(c, 4);
    b ^= a; b -= rot(a, 14);
    c ^= b; c -= rot(b, 24);

    return c;
}


const int Util::hashTableSizesG[16] =
{
    53, 769, 3079, 12289, 49157, 98317, 196613, 393241, 786433, 1572869,
    3145739, 6291469, 12582917, 25165843, 50331653, 100663319
};



} /* namespace Hash */
