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
            std::auto_ptr <CryptoPP::HashTransformation> tmp;
            switch (method) {
                case cryptopp::HASH_METHOD::MD5_METHOD:
                    tmp.reset(new CryptoPP::Weak1::MD5);
                    break;
                case cryptopp::HASH_METHOD::SHA1_METHOD:
                    tmp.reset(new CryptoPP::SHA1);
                    break;
                case cryptopp::HASH_METHOD::SHA256_METHOD:
                    tmp.reset(new CryptoPP::SHA256);
                    break;
                case cryptopp::HASH_METHOD::SHA512_METHOD:
                    tmp.reset(new CryptoPP::SHA512);
                    break;
                default:
                    tmp.reset(new CryptoPP::Weak1::MD5);
                    break;
            }
            std::string dst;
            CryptoPP::StringSource(str, true, new CryptoPP::HashFilter(*tmp, new CryptoPP::HexEncoder(new CryptoPP::StringSink(dst), false)));
            return dst;
        }

    public:

        std::string encode(const std::string & plaintext) {
            std::string ciphertext;
            CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption Encryptor(m_key, CryptoPP::AES::DEFAULT_KEYLENGTH);
            CryptoPP::StringSource(plaintext, true, new CryptoPP::StreamTransformationFilter(Encryptor, new CryptoPP::StringSink(ciphertext))
                    );
            return cryptopp::enhex(ciphertext);
        }

        std::string decode(const std::string & ciphertext) {
            std::string plaintext;
            CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption Decryptor(m_key, CryptoPP::AES::DEFAULT_KEYLENGTH);
            CryptoPP::StringSource(cryptopp::dehex(ciphertext), true,
                    new CryptoPP::StreamTransformationFilter(Decryptor, new CryptoPP::StringSink(plaintext))
                    );
            return plaintext;
        }

        static std::string enhex(const std::string & str) {
            std::string encoded;
            CryptoPP::StringSource ss(str, true, new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded), false)
                    );
            return encoded;
        }

        static std::string dehex(const std::string & str) {
            std::string encoded;
            CryptoPP::StringSource ss(str, true, new CryptoPP::HexDecoder(new CryptoPP::StringSink(encoded))
                    );
            return encoded;
        }

        static std::string enbase64(const std::string & str) {
            std::string encoded;
            CryptoPP::StringSource ss(str, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encoded))
                    );
            return encoded;
        }

        static std::string debase64(const std::string & str) {
            std::string encoded;
            CryptoPP::StringSource ss(str, true, new CryptoPP::Base64Decoder(new CryptoPP::StringSink(encoded))
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
            std::string encoded;
            CryptoPP::StringSource ss(str, true, new CryptoPP::Base32Encoder(new CryptoPP::StringSink(encoded), false)
                    );
            return encoded;
        }

        static std::string debase32(const std::string & str) {
            std::string encoded;
            CryptoPP::StringSource ss(str, true, new CryptoPP::Base32Decoder(new CryptoPP::StringSink(encoded))
                    );
            return encoded;
        }
    };
}
#endif /* CRYPTOPP_HPP */


