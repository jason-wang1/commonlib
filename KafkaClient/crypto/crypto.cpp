#include "crypto.h"
#include <stdint.h>
#include <string.h>
#include "aes.h"

static const std::string g_aeskey = Crypto::fromHex("23544452656469732d2d3e3230323123");

static const char g_hexcode[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

std::string Crypto::aes_encrypt(const std::string &sPlainPassword)
{
    std::string sResult;
    aes_encrypt(sPlainPassword, sResult);
    return toHex(sResult);
}

std::string Crypto::aes_decrypt(const std::string &sCryptoPassword)
{
    std::string sResult;
    aes_decrypt(fromHex(sCryptoPassword), sResult);
    return sResult;
}

bool Crypto::aes_encrypt(const std::string &sData, std::string &sResult)
{
    return aes_encrypt(sData.c_str(), sData.c_str() + sData.size(), sResult);
}

bool Crypto::aes_decrypt(const std::string &sData, std::string &sResult)
{
    return aes_decrypt(sData.c_str(), sData.c_str() + sData.size(), sResult);
}

bool Crypto::aes_encrypt(const char *pBegin, const char *pEnd, std::string &sResult)
{
    aes_context ctx;
    if (aes_set_key2(&ctx, (unsigned char *)g_aeskey.c_str(), g_aeskey.size() * 8) != 0)
    {
        return false;
    }

    return Crypto::cbc_encrypt(pBegin, pEnd, (Crypto::block_encrypt_func)::aes_encrypt2, 16, &ctx, sResult);
}

bool Crypto::aes_decrypt(const char *pBegin, const char *pEnd, std::string &sResult)
{
    aes_context ctx;
    if (aes_set_key2(&ctx, (unsigned char *)g_aeskey.c_str(), g_aeskey.size() * 8) != 0)
    {
        return false;
    }

    return Crypto::cbc_decrypt(pBegin, pEnd, (Crypto::block_decrypt_func)::aes_decrypt2, 16, &ctx, sResult);
}

bool Crypto::cbc_encrypt(const char *pBegin, const char *pEnd, block_encrypt_func encrypt_func, int iBlockSize, void *ctx, std::string &sResult)
{
    unsigned char buf[32] = {0};
    if (iBlockSize < 8 || iBlockSize > (int)sizeof(buf))
    {
        return false;
    }

    int iDataSize = pEnd - pBegin;
    int rem = iDataSize % iBlockSize;
    if (rem == 0)
    {
        sResult.reserve(sResult.size() + iDataSize);
    }
    else
    {
        sResult.reserve(sResult.size() + iDataSize - rem + iBlockSize + 1);
    }

    for (const char *r = pBegin; r < pEnd; r += iBlockSize)
    {
        int len = std::min(iBlockSize, (int)(pEnd - r));
        for (int i = 0; i < len; ++i)
        {
            buf[i] = *(r + i) ^ buf[i];
        }
        encrypt_func(ctx, buf, buf);
        sResult.append(buf, buf + iBlockSize);
    }
    if (rem != 0)
    {
        sResult.append(1, rem);
    }
    return true;
}

bool Crypto::cbc_decrypt(const char *pBegin, const char *pEnd, block_decrypt_func decrypt_func, int iBlockSize, void *ctx, std::string &sResult)
{
    unsigned char buf[32] = {0};
    if (iBlockSize < 8 || iBlockSize > (int)sizeof(buf))
    {
        return false;
    }

    int iDataSize = pEnd - pBegin;
    int rem = iDataSize % iBlockSize;
    if (rem == 0)
    {
        sResult.reserve(sResult.size() + iDataSize);
    }
    else if (rem == 1)
    {
        if (iDataSize < iBlockSize + 1)
        {
            return false;
        }
        rem = *(pEnd - 1);
        if (rem <= 0 || rem >= iBlockSize)
        {
            return false;
        }
        sResult.reserve(sResult.size() + iDataSize - iBlockSize - 1 + rem);
        --pEnd;
    }
    else
    {
        return false;
    }

    for (const char *r = pBegin; r < pEnd; r += iBlockSize)
    {
        decrypt_func(ctx, r, buf);
        if (r != pBegin)
        {
            for (int i = 0; i < iBlockSize; ++i)
            {
                buf[i] ^= *(r - iBlockSize + i);
            }
        }
        if (rem != 0 && r + iBlockSize >= pEnd)
        {
            sResult.append(buf, buf + rem);
        }
        else
        {
            sResult.append(buf, buf + iBlockSize);
        }
    }
    return true;
}

bool Crypto::cbc_encrypt(const std::string &sData, block_encrypt_func encrypt_func, int iBlockSize, void *ctx, std::string &sResult)
{
    return cbc_encrypt(sData.c_str(), sData.c_str() + sData.size(), encrypt_func, iBlockSize, ctx, sResult);
}

bool Crypto::cbc_decrypt(const std::string &sData, block_decrypt_func decrypt_func, int iBlockSize, void *ctx, std::string &sResult)
{
    return cbc_decrypt(sData.c_str(), sData.c_str() + sData.size(), decrypt_func, iBlockSize, ctx, sResult);
}

std::string Crypto::toHex(const std::string &s)
{
    std::string r;
    r.resize(s.size() * 2);
    for (std::string::size_type i = 0; i < s.size(); ++i)
    {
        uint8_t c = s[i];
        r[i * 2] = g_hexcode[c >> 4];
        r[i * 2 + 1] = g_hexcode[c & 0xf];
    }
    return r;
}

std::string Crypto::fromHex(const std::string &s)
{
    std::string r;
    std::string::size_type n = s.size() / 2;
    r.resize(n);
    for (std::string::size_type i = 0; i < n; ++i)
    {
        uint8_t hi = s[i * 2];
        uint8_t lo = s[i * 2 + 1];
        hi = hi >= 'a' ? (hi - 'a' + 10) : (hi >= 'A' ? (hi - 'A' + 10) : hi - '0');
        lo = lo >= 'a' ? (lo - 'a' + 10) : (lo >= 'A' ? (lo - 'A' + 10) : lo - '0');
        r[i] = (hi << 4) | lo;
    }
    return r;
}
