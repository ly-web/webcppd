#ifndef CRYPTOPP_HPP
#define CRYPTOPP_HPP

#include <string>
#include <memory>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/base64.h>
#include <cryptopp/base32.h>
#include <cryptopp/sha.h>
#include <cryptopp/md5.h>


namespace webcppd {

    class cryptopp {
    public:

        cryptopp(const std::string &key)
        : m_key(0), m_iv(0) {
            m_key = new unsigned char[CryptoPP::AES::DEFAULT_KEYLENGTH]();
            m_iv = new unsigned char[CryptoPP::AES::BLOCKSIZE]();

            size_t n = std::min(key.size(), static_cast<size_t> (CryptoPP::AES::DEFAULT_KEYLENGTH));
            for (size_t i = 0; i < n; ++i) {
                m_key[i] = key[i];
            }
        }

        virtual ~cryptopp() {
            delete[] m_key;
            delete[] m_iv;
        }

    private:
        unsigned char *m_key;
        unsigned char *m_iv;

        enum HASH_METHOD {
            MD5_METHOD, SHA1_METHOD, SHA256_METHOD, SHA512_METHOD
        };

        static std::string hash(const std::string& str, const cryptopp::HASH_METHOD& method) {
            using namespace CryptoPP;
            std::auto_ptr <HashTransformation> tmp;
            switch (method) {
                case cryptopp::HASH_METHOD::MD5_METHOD:
                    tmp.reset(new Weak1::MD5);
                    break;
                case cryptopp::HASH_METHOD::SHA1_METHOD:
                    tmp.reset(new SHA1);
                    break;
                case cryptopp::HASH_METHOD::SHA256_METHOD:
                    tmp.reset(new SHA256);
                    break;
                case cryptopp::HASH_METHOD::SHA512_METHOD:
                    tmp.reset(new SHA512);
                    break;
                default:
                    tmp.reset(new Weak1::MD5);
                    break;
            }
            std::string dst;
            StringSource(str, true, new HashFilter(*tmp, new HexEncoder(new StringSink(dst), false)));
            return dst;
        }

    public:

        std::string encode(const std::string & plaintext) {
            using namespace CryptoPP;
            std::string ciphertext;
            ECB_Mode<AES>::Encryption Encryptor(m_key, AES::DEFAULT_KEYLENGTH);
            StringSource(plaintext, true, new StreamTransformationFilter(Encryptor, new StringSink(ciphertext))
                    );
            return cryptopp::enhex(ciphertext);
        }

        std::string decode(const std::string & ciphertext) {
            using namespace CryptoPP;
            std::string plaintext;
            ECB_Mode<AES>::Decryption Decryptor(m_key, AES::DEFAULT_KEYLENGTH);
            StringSource(cryptopp::dehex(ciphertext), true,
                    new StreamTransformationFilter(Decryptor, new StringSink(plaintext))
                    );
            return plaintext;
        }

        static std::string enhex(const std::string & str) {
            using namespace CryptoPP;
            std::string encoded;
            StringSource ss(str, true, new HexEncoder(new StringSink(encoded), false)
                    );
            return encoded;
        }

        static std::string dehex(const std::string & str) {
            using namespace CryptoPP;
            std::string encoded;
            StringSource ss(str, true, new HexDecoder(new StringSink(encoded))
                    );
            return encoded;
        }

        static std::string enbase64(const std::string & str) {
            using namespace CryptoPP;
            std::string encoded;
            StringSource ss(str, true, new Base64Encoder(new StringSink(encoded))
                    );
            return encoded;
        }

        static std::string debase64(const std::string & str) {
            using namespace CryptoPP;
            std::string encoded;
            StringSource ss(str, true, new Base64Decoder(new StringSink(encoded))
                    );
            return encoded;
        }

        static std::string sha256(const std::string & str) {
            return cryptopp::hash(str, cryptopp::HASH_METHOD::SHA256_METHOD);
        }

        static std::string sha512(const std::string & str) {
            return cryptopp::hash(str, cryptopp::HASH_METHOD::SHA512_METHOD);
        }

        static std::string md5(const std::string & str) {
            return cryptopp::hash(str, cryptopp::HASH_METHOD::MD5_METHOD);
        }

        static std::string sha1(const std::string & str) {
            return cryptopp::hash(str, cryptopp::HASH_METHOD::SHA1_METHOD);
        }

        static std::string enbase32(const std::string & str) {
            using namespace CryptoPP;
            std::string encoded;
            StringSource ss(str, true, new Base32Encoder(new StringSink(encoded), false)
                    );
            return encoded;
        }

        static std::string debase32(const std::string & str) {
            using namespace CryptoPP;
            std::string encoded;
            StringSource ss(str, true, new Base32Decoder(new StringSink(encoded))
                    );
            return encoded;
        }
    };
}
#endif /* CRYPTOPP_HPP */


