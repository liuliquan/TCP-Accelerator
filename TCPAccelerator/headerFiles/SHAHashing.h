//
// Created by root on 10/19/17.
//
#ifndef SHA1_H
#define SHA1_H

#ifndef structIncluded
#include "types.h"
#endif

#define SHA1_BLOCK_SIZE 20              // SHA1 outputs a 20 byte digest

typedef unsigned char BYTE;             // 8-bit byte
typedef unsigned int  WORD;             // 32-bit word, change to "long" for 16-bit machines

typedef struct {
    BYTE data[64];
    WORD datalen;
    unsigned long long bitlen;
    WORD state[5];
    WORD k[4];
} SHA1_CTX;

//hashing functions
void sha1Init(SHA1_CTX *ctx);
void sha1Update(SHA1_CTX *ctx, const BYTE *data, size_t len);
void sha1Final(SHA1_CTX *ctx, BYTE *hash);

#endif