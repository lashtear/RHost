#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "externs.h"
#include "sha1.h"

void SHA1_Init(SHA1_CONTEXT *p)
{
    p->H[0] = 0x67452301;
    p->H[1] = 0xEFCDAB89;
    p->H[2] = 0x98BADCFE;
    p->H[3] = 0x10325476;
    p->H[4] = 0xC3D2E1F0;
    p->nTotal = 0;
    p->nblock = 0;
}

#ifdef WIN32
#define ROTL(d,n) _lrotl(d,n)
#else // WIN32
#define ROTL(d,n) (((d) << (n)) | ((d) >> (32-(n))))
#endif // WIN32

#define Ch(x,y,z)      (((x) & (y)) ^ (~(x) & (z)))
#define Parity(x,y,z)  ((x) ^ (y) ^ (z))
#define Maj(x,y,z)     (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

static void SHA1_HashBlock(SHA1_CONTEXT *p)
{
    int t, j;
    UINT32 W[80], a, b, c, d, e, T;

    // Prepare Message Schedule, {W sub t}.
    //
    for (t = 0, j = 0; t <= 15; t++, j += 4)
    {
        W[t] = (p->block[j  ] << 24)
             | (p->block[j+1] << 16)
             | (p->block[j+2] <<  8)
             | (p->block[j+3]      );
    }
    for (t = 16; t <= 79; t++)
    {
        W[t] = ROTL(W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16], 1);
    }

    a = p->H[0];
    b = p->H[1];
    c = p->H[2];
    d = p->H[3];
    e = p->H[4];

    for (t =  0; t <= 19; t++)
    {
        T = ROTL(a,5) + Ch(b,c,d) + e + 0x5A827999 + W[t];
        e = d;
        d = c;
        c = ROTL(b,30);
        b = a;
        a = T;
    }
    for (t = 20; t <= 39; t++)
    {
        T = ROTL(a,5) + Parity(b,c,d) + e + 0x6ED9EBA1 + W[t];
        e = d;
        d = c;
        c = ROTL(b,30);
        b = a;
        a = T;
    }
    for (t = 40; t <= 59; t++)
    {
        T = ROTL(a,5) + Maj(b,c,d) + e + 0x8F1BBCDC + W[t];
        e = d;
        d = c;
        c = ROTL(b,30);
        b = a;
        a = T;
    }
    for (t = 60; t <= 79; t++)
    {
        T = ROTL(a,5) + Parity(b,c,d) + e + 0xCA62C1D6 + W[t];
        e = d;
        d = c;
        c = ROTL(b,30);
        b = a;
        a = T;
    }

    p->H[0] += a;
    p->H[1] += b;
    p->H[2] += c;
    p->H[3] += d;
    p->H[4] += e;
}

void SHA1_Compute(SHA1_CONTEXT *p, size_t n, const char *buf)
{
    size_t m;
    while (n)
    {
        m = sizeof(p->block) - p->nblock;
        if (n < m)
        {
            m = n;
        }
        memcpy(p->block + p->nblock, buf, m);
        buf += m;
        n -= m;
        p->nblock += m;
        p->nTotal += m;

        if (p->nblock == sizeof(p->block))
        {
            SHA1_HashBlock(p);
            p->nblock = 0;
        }
    }
}

void SHA1_Final(SHA1_CONTEXT *p)
{
    p->block[p->nblock++] = 0x80;
    if (sizeof(p->block) - sizeof(UINT64) < p->nblock)
    {
        memset(p->block + p->nblock, 0, sizeof(p->block) - p->nblock);
        SHA1_HashBlock(p);
        memset(p->block, 0, sizeof(p->block) - sizeof(UINT64));
    }
    else
    {
        memset(p->block + p->nblock, 0, sizeof(p->block) - p->nblock - sizeof(UINT64));
    }
    p->nTotal *= 8;

    p->block[sizeof(p->block) - 8] = (UINT8)((p->nTotal >> 56) & 0xFF);
    p->block[sizeof(p->block) - 7] = (UINT8)((p->nTotal >> 48) & 0xFF);
    p->block[sizeof(p->block) - 6] = (UINT8)((p->nTotal >> 40) & 0xFF);
    p->block[sizeof(p->block) - 5] = (UINT8)((p->nTotal >> 32) & 0xFF);
    p->block[sizeof(p->block) - 4] = (UINT8)((p->nTotal >> 24) & 0xFF);
    p->block[sizeof(p->block) - 3] = (UINT8)((p->nTotal >> 16) & 0xFF);
    p->block[sizeof(p->block) - 2] = (UINT8)((p->nTotal >>  8) & 0xFF);
    p->block[sizeof(p->block) - 1] = (UINT8)((p->nTotal      ) & 0xFF);
    SHA1_HashBlock(p);
}

#if 0

//#define TEST_STRING "abc"
#define TEST_STRING "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
int main(int argc, char *argv[])
{
    char buffer[] = TEST_STRING;
    int i;

    SHA1_CONTEXT shac;
    SHA1_Init(&shac);
    SHA1_Compute(&shac, strlen(TEST_STRING), buffer);
    SHA1_Final(&shac);

    for (i = 0; i < 5; i++)
    {
        printf("%08X", shac.H[i]);
    }
    return 1;
}

#endif
