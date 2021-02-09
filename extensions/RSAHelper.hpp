#ifndef RSA_HELPER_H
#define RSA_HELPER_H


#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <bits/stdc++.h>
#include "openssl/err.h"

class RSAHelper {
    public:
        RSAHelper(int key_len);

        ~RSAHelper();

        int encrypt (const char * plainText, char* cipherText);

        bool decrypt (char * cipherText, char* plainText, int c_len);

        std::string decrypt (std::string cipherText, std::string publicKey);

        std::string encrypt (std::string plainText);

        void test();

        std::string publicKeyRSAStr;
        std::string privateKeyRSAStr;
    private:
        int keyLen;
        RSA *keypair;
        RSA *privateKeyRSA;
        RSA *publicKeyRSA;
        BIO *pri;
        BIO *pub;
        size_t pri_len;
        size_t pub_len;
        char *pri_key;
        char *pub_key; 
};

#endif