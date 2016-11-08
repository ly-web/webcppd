#ifndef VALIDATION_HPP
#define VALIDATION_HPP

#include <string>
#include <Poco/SingletonHolder.h>
#include <Poco/RegularExpression.h>

namespace webcppd {

    class validation {
    public:

        bool match(const std::string& str, const std::string& pattern) {
            return Poco::RegularExpression(pattern).match(str);
        }
        std::string INTEGER = "^-?[1-9]\\d*$"
                , POSITIVE_INTEGER = "^[1-9]\\d*$"
                , NEGATIVE_INTEGER = "^-[1-9]\\d*$"
                , NUMBER = "^([+-]?)\\d*\\.?\\d+$"
                , POSITIVE_NUMBER = "^[1-9]\\d*|0$"
                , NEGATINE_NUMBER = "^-[1-9]\\d*|0$"
                , FLOAT = "^([+-]?)\\d*\\.\\d+$"
                , POSTTIVE_FLOAT = "^[1-9]\\d*.\\d*|0.\\d*[1-9]\\d*$"
                , NEGATIVE_FLOAT = "^-([1-9]\\d*.\\d*|0.\\d*[1-9]\\d*)$"
                , UNPOSITIVE_FLOAT = "^[1-9]\\d*.\\d*|0.\\d*[1-9]\\d*|0?.0+|0$"
                , UNNEGATIVE_FLOAT = "^(-([1-9]\\d*.\\d*|0.\\d*[1-9]\\d*))|0?.0+|0$"
                , EMAIL = "^\\w+((-\\w+)|(\\.\\w+))*\\@[A-Za-z0-9]+((\\.|-)[A-Za-z0-9]+)*\\.[A-Za-z0-9]+$"
                , COLOR = "^[a-fA-F0-9]{6}$"
                , URL = "^http[s]?:\\/\\/([\\w-]+\\.)+[\\w-]+([\\w-./?%&=]*)?$"
                , CHINESE = "^[\u4E00-\u9FA5\uF900-\uFA2D]+$"
                , ASCII = "^[\\x00-\\xFF]+$"
                , ZIPCODE = "^\\d{6}$"
                , MOBILE = "^(13|15)[0-9]{9}$"
                , IP4 = "^(25[0-5]|2[0-4]\\d|[0-1]\\d{2}|[1-9]?\\d)\\.(25[0-5]|2[0-4]\\d|[0-1]\\d{2}|[1-9]?\\d)\\.(25[0-5]|2[0-4]\\d|[0-1]\\d{2}|[1-9]?\\d)\\.(25[0-5]|2[0-4]\\d|[0-1]\\d{2}|[1-9]?\\d)$"
                , NOTEMPTY = "^\\S+$"
                , PICTURE = "(.*)\\.(jpg|bmp|gif|ico|pcx|jpeg|tif|png|raw|tga)$"
                , RAR = "(.*)\\.(rar|zip|7zip|tgz)$"
                , DATE = "^((((1[6-9]|[2-9]\\d)\\d{2})-(0?[13578]|1[02])-(0?[1-9]|[12]\\d|3[01]))|(((1[6-9]|[2-9]\\d)\\d{2})-(0?[13456789]|1[012])-(0?[1-9]|[12]\\d|30))|(((1[6-9]|[2-9]\\d)\\d{2})-0?2-(0?[1-9]|1\\d|2[0-8]))|(((1[6-9]|[2-9]\\d)(0[48]|[2468][048]|[13579][26])|((16|[2468][048]|[3579][26])00))-0?2-29-)) (20|21|22|23|[0-1]?\\d):[0-5]?\\d:[0-5]?\\d$"
                , QQ = "^[1-9]*[1-9][0-9]*$"
                , TEL = "^(([0\\+]\\d{2,3}-)?(0\\d{2,3})-)?(\\d{7,8})(-(\\d{3,}))?$"
                , LETTER = "^[A-Za-z]+$"
                , LETTER_U = "^[A-Z]+$"
                , LETTER_L = "^[a-z]+$"
                , IDCARD = "^(\\d{15}$|^\\d{18}$|^\\d{17}(\\d|X|x))$";



    public:

        static validation& instance() {
            static Poco::SingletonHolder<validation> validator;
            return *validator.get();
        }




    };

}

#endif /* VALIDATION_HPP */

