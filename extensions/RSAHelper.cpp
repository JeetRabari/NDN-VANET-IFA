#include "RSAHelper.hpp"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("RSAHelper");

RSAHelper::RSAHelper(int key_len)
{
    unsigned long e = 3;
    keyLen = key_len;
    keypair = RSA_generate_key (key_len, e, NULL, NULL);

    
    pri = BIO_new (BIO_s_mem ());
    pub = BIO_new (BIO_s_mem ());

    PEM_write_bio_RSAPrivateKey (pri, keypair, NULL, NULL, 0, NULL, NULL);
    PEM_write_bio_RSAPublicKey (pub, keypair);

    privateKeyRSA = PEM_read_bio_RSAPrivateKey (pri, NULL, 0, NULL);
    publicKeyRSA = PEM_read_bio_RSAPublicKey (pub, NULL, 0 ,NULL);

    PEM_write_bio_RSAPrivateKey (pri, keypair, NULL, NULL, 0, NULL, NULL);
    PEM_write_bio_RSAPublicKey (pub, keypair);

    pri_len = BIO_pending (pri);
    pub_len = BIO_pending (pub);

    pri_key = (char *) malloc (pri_len + 1);
    pub_key = (char *) malloc (pub_len + 1);

    BIO_read (pri, pri_key, pri_len);
    BIO_read (pub, pub_key, pub_len);

    pri_key[pri_len] = '\0';
    pub_key[pub_len] = '\0';

    privateKeyRSAStr.assign (pri_key, pri_len + 1);
    publicKeyRSAStr.assign (pub_key, pub_len + 1);
}

RSAHelper::~RSAHelper ()
{
    RSA_free (keypair);
    BIO_free_all (pri);
    BIO_free_all (pub);
    free (pri_key);
    free (pub_key);
}

int
RSAHelper::encrypt (const char *plainText, char *cipherText)
{
    int retVal;
    //cipherText = (char *)malloc (RSA_size (keypair));
    retVal = RSA_private_encrypt (strlen(plainText)+1, (unsigned char*)plainText, (unsigned char*)cipherText,
                                privateKeyRSA, RSA_PKCS1_PADDING);
    return (retVal);
}

bool
RSAHelper::decrypt (char *cipherText, char *plainText, int c_len)
{
    int retVal;

    //plainText = (char *)malloc (c_len);
    retVal = RSA_public_decrypt (c_len, (unsigned char*)cipherText, (unsigned char*)plainText,
                                publicKeyRSA, RSA_PKCS1_PADDING);

    return (retVal != -1);
}

std::string
RSAHelper::encrypt (std::string plainText)
{
    char* enc = (char *)malloc (keyLen/8);
    int err = encrypt (plainText.c_str(), enc);

    if(err == -1)
    {
        char *err = (char *) malloc (130);
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        NS_LOG_INFO ("Enc Err: " << err);
        return "";
    } 
    return std::string (enc, err);
}

std::string 
RSAHelper::decrypt (std::string cipherText, std::string publicKey)
{
    BIO *pub = BIO_new ( BIO_s_mem() );
    BIO_write (pub, publicKey.c_str(), publicKey.length());

    RSA* publicK = PEM_read_bio_RSAPublicKey (pub, NULL, 0 ,NULL);

    char* decrypted = (char *)malloc (cipherText.length()*sizeof(char));

    int err = RSA_public_decrypt (cipherText.length(), (unsigned char*)cipherText.c_str(), (unsigned char*)decrypted
                        ,publicK, RSA_PKCS1_PADDING);

    if(err == -1) 
    {
        char *err = (char *) malloc (130);
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        NS_LOG_INFO ("Decrypt Error: " << err);   
        return "";
    }

    std::string retVal (decrypted, strlen(decrypted));

    return retVal;
}

void
RSAHelper::test()
{
    std::string msg = "helloJeet";

    std::string cT = encrypt (msg.c_str());

    std::cout << "Enc Str Len:" << cT.length() << std::endl;

    std::string decrypted = decrypt (cT, this->publicKeyRSAStr);

    std::cout << decrypted << std::endl;
}