/*
  Implementation de Blake2s
  Credit: Alexey NAZAROV
*/


//blake2s.c
#include <stdint.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "blake2s.h"

#define BLAKE2S_BLOCKBYTES 64
#define BLAKE2S_ERRCHECK 0
#define BLAKE2S_STREAM 1
#define BLAKE2S_UNALIGNED 0
#define BLAKE2S_64BIT 0
#define NATIVE_LITTLE_ENDIAN 1

#define BLAKE2S_OUTLEN 32

typedef struct {
    uint32_t h[8];
    uint32_t t[2];
    uint8_t buf[64];
    size_t buflen;
    uint8_t last_node;
    uint32_t f[2];
} blake2s_ctx_t;

// SHA-256 context and response buffer
static blake2s_ctx_t ctx;

static const uint32_t blake2s_IV[8] = {
    0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
    0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

static const uint8_t blake2s_sigma[10][8] = {
    { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef },
    { 0xea, 0x48, 0x9f, 0xd6, 0x1c, 0x02, 0xb7, 0x53 },
    { 0xb8, 0xc0, 0x52, 0xfd, 0xae, 0x36, 0x71, 0x94 },
    { 0x79, 0x31, 0xdc, 0xbe, 0x26, 0x5a, 0x40, 0xf8 },
    { 0x90, 0x57, 0x24, 0xaf, 0xe1, 0xbc, 0x68, 0x3d },
    { 0x2c, 0x6a, 0x0b, 0x83, 0x4d, 0x75, 0xfe, 0x19 },
    { 0xc5, 0x1f, 0xed, 0x4a, 0x07, 0x63, 0x92, 0x8b },
    { 0xdb, 0x7e, 0xc1, 0x39, 0x50, 0xf4, 0x86, 0x2a },
    { 0x6f, 0xe9, 0xb3, 0x08, 0xc2, 0xd7, 0x14, 0xa5 },
    { 0xa2, 0x84, 0x76, 0x15, 0xfb, 0x9e, 0x3c, 0xd0 },
};

static inline uint32_t load32(const void *src) {
    uint32_t w;
    memcpy(&w, src, sizeof(w));
    return w;
}

static inline void store32(void *dst, uint32_t w) {
    memcpy(dst, &w, sizeof(w));
}

static inline uint32_t rotr32(const uint32_t w, const unsigned c) {
    return (w >> c) | (w << (32 - c));
}

static void secure_zero_memory(void *v, size_t n) {
    memset(v, 0, n);
}

static void blake2s_set_lastnode(blake2s_ctx_t *S) {
    S->f[1] = (uint32_t)-1;
}

static void blake2s_set_lastblock(blake2s_ctx_t *S) {
    if (S->last_node) blake2s_set_lastnode(S);
    S->f[0] = (uint32_t)-1;
}

static void blake2s_increment_counter(blake2s_ctx_t *S, const size_t inc) {
    S->t[0] += inc;
}

static void blake2s_set_IV(uint32_t *buf) {
    memcpy(buf, blake2s_IV, sizeof(blake2s_IV));
}

static void blake2s_init(blake2s_ctx_t *S) {
    memset(S, 0, sizeof(blake2s_ctx_t));
    blake2s_set_IV(S->h);
    S->h[0] ^= (1UL << 24) | (1UL << 16) | BLAKE2S_OUTLEN;
}

static void blake2s_round(size_t r, const uint32_t m[16], uint32_t v[16]) {
    for (size_t i = 0; i < 8; i++) {
        uint32_t *a = &v[(i + (i / 4) * 0) % 4];
        uint32_t *b = &v[(i + (i / 4) * 1) % 4 + 4];
        uint32_t *c = &v[(i + (i / 4) * 2) % 4 + 8];
        uint32_t *d = &v[(i + (i / 4) * 3) % 4 + 12];
        const uint8_t sigma = blake2s_sigma[r][i];
        uint32_t m1 = m[sigma >> 4], m2 = m[sigma & 0xf];
        *a += *b + m1;
        *d = rotr32(*d ^ *a, 16);
        *c += *d;
        *b = rotr32(*b ^ *c, 12);
        *a += *b + m2;
        *d = rotr32(*d ^ *a, 8);
        *c += *d;
        *b = rotr32(*b ^ *c, 7);
    }
}

static void blake2s_compress(blake2s_ctx_t *S, const uint8_t in[BLAKE2S_BLOCKBYTES]) {
    uint32_t m[16];
    memcpy(m, in, sizeof(m));
    uint32_t v[16];
    memcpy(v, S->h, sizeof(uint32_t) * 8);
    blake2s_set_IV(&v[8]);
    v[12] ^= S->t[0];
    v[13] ^= S->t[1];
    v[14] ^= S->f[0];
    v[15] ^= S->f[1];
    for (size_t r = 0; r < 10; r++)
        blake2s_round(r, m, v);
    for (size_t i = 0; i < 8; i++)
        S->h[i] ^= v[i] ^ v[i + 8];
}

static void blake2s_update(blake2s_ctx_t *S, const uint8_t *data, uint16_t len) {
    size_t left = S->buflen;
    size_t fill = BLAKE2S_BLOCKBYTES - left;
    if (len > fill) {
        S->buflen = 0;
        memcpy(S->buf + left, data, fill);
        blake2s_increment_counter(S, BLAKE2S_BLOCKBYTES);
        blake2s_compress(S, S->buf);
        data += fill;
        len -= fill;
        while (len > BLAKE2S_BLOCKBYTES) {
            blake2s_increment_counter(S, BLAKE2S_BLOCKBYTES);
            blake2s_compress(S, data);
            data += BLAKE2S_BLOCKBYTES;
            len -= BLAKE2S_BLOCKBYTES;
        }
    }
    memcpy(S->buf + S->buflen, data, len);
    S->buflen += len;
}

static void blake2s_final(blake2s_ctx_t *S, uint8_t *digest) {
    blake2s_increment_counter(S, S->buflen);
    blake2s_set_lastblock(S); // Directly set last block without checking
    memset(S->buf + S->buflen, 0, BLAKE2S_BLOCKBYTES - S->buflen);
    blake2s_compress(S, S->buf);
    memcpy(digest, S->h, BLAKE2S_OUTLEN);
    secure_zero_memory(S, sizeof(blake2s_ctx_t));
}

//------------------------------------------------------
// Compute Blake2s
//------------------------------------------------------
// ecrit dans le ptr digest le hash du contenu de data
// data 128o, digest 32o
void compute_blake2s(uint8_t * data, uint8_t * digest) {
    // Hash computation
    blake2s_init(&ctx);
    blake2s_update(&ctx, data, 128);
    blake2s_final(&ctx, digest);
}
