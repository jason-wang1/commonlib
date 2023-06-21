#pragma once
#include <stdint.h>
#include <string>
#include <vector>

// crypto各种算法
// aes算法比正常的多校验位：原始密码的长度%16
class Crypto
{
public:
    static std::string aes_encrypt(const std::string &sPlainPassword);
    static std::string aes_decrypt(const std::string &sCryptoPassword);

    static bool aes_encrypt(const std::string &sData, std::string &sResult);
    static bool aes_decrypt(const std::string &sData, std::string &sResult);
    static bool aes_encrypt(const char *pBegin, const char *pEnd, std::string &sResult);
    static bool aes_decrypt(const char *pBegin, const char *pEnd, std::string &sResult);

    typedef void (*block_encrypt_func)(void *ctx, const void *input, void *output);
    typedef void (*block_decrypt_func)(void *ctx, const void *input, void *output);

    static bool cbc_encrypt(const char *pBegin, const char *pEnd, block_encrypt_func encrypt_func, int iBlockSize, void *ctx, std::string &sResult);
    static bool cbc_decrypt(const char *pBegin, const char *pEnd, block_decrypt_func decrypt_func, int iBlockSize, void *ctx, std::string &sResult);

    static bool cbc_encrypt(const std::string &sData, block_encrypt_func encrypt_func, int iBlockSize, void *ctx, std::string &sResult);
    static bool cbc_decrypt(const std::string &sData, block_decrypt_func decrypt_func, int iBlockSize, void *ctx, std::string &sResult);

    static std::string toHex(const std::string &s);
    static std::string fromHex(const std::string &s);
};
