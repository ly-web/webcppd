#ifndef CV_CAPTCHA_HPP
#define CV_CAPTCHA_HPP

#include <string>
#include <ostream>
#include <array>
#include <vector>
#include <Poco/String.h>
#include <Poco/Random.h>
#include <opencv2/opencv.hpp>


namespace webcppd {

    class cv_captcha {
    public:
        cv_captcha(bool lined = false, bool ellipsed = false, bool slated = false);
        cv_captcha(const cv_captcha& orig) = delete;
        virtual ~cv_captcha() = default;

    public:
        int get_size();
        std::string get_content();
        void create();
        void add_line();
        void add_ellipse();
        void add_slate();
        void set_line_number(int n);
        void set_ellipse_number(int n);
        void set_slate_number(int n);
        void write_to_file(const std::string& filepath);
        int get_width();
        int get_height();
        friend std::ostream& operator<<(std::ostream& out, const cv_captcha& x);

    private:
        int n, line_number, ellipse_number, slate_number;
        std::string content;
        Poco::Random rng;
        cv::Mat matrix;
        bool lined, ellipsed, slated;
        const std::string all_char;
        const std::array<int, 9> font_face;
    private:
        std::string get_random_char(int n = 1);
        int get_random_font_face();
        void draw_random_line(int n = 8);
        void draw_random_ellipse(int n = 8);
        void draw_random_slate(int n = 300);
        cv::Scalar random_color();
        template<typename T>
        T uniform(T a, T b);
    };

    cv_captcha::cv_captcha(bool lined, bool ellipsed, bool slated) :
    n(6)
    , line_number(10)
    , ellipse_number(10)
    , slate_number(300),
    content()
    , rng()
    , matrix(80, 200, CV_8UC3, cv::Scalar::all(255))
    , lined(lined)
    , ellipsed(ellipsed)
    , slated(slated)
    , all_char("0123456789abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ")
    , font_face({0, 1, 2, 3, 4, 5, 6, 7, 16}) {

    }

    inline std::string cv_captcha::get_random_char(int n) {
        std::string result;
        unsigned int p = 0;
        std::string::size_type size = cv_captcha::all_char.size();
        for (int i = 0; i < n; ++i) {
            p = this->rng.next(size);
            result.push_back(cv_captcha::all_char[p]);
        }

        return result;
    }

    inline int cv_captcha::get_random_font_face() {
        return cv_captcha::font_face[this->rng.next(cv_captcha::font_face.size())];
    }

    inline int cv_captcha::get_size() {
        return this->n;
    }

    inline std::string cv_captcha::get_content() {
        return this->content;
    }

    inline void cv_captcha::add_ellipse() {
        this->ellipsed = true;
    }

    inline void cv_captcha::add_line() {
        this->lined = true;
    }

    inline void cv_captcha::add_slate() {
        this->slated = true;
    }

    inline void cv_captcha::set_line_number(int n) {
        this->line_number = n;
    }

    inline void cv_captcha::set_ellipse_number(int n) {
        this->ellipse_number = n;
    }

    inline void cv_captcha::set_slate_number(int n) {
        this->slate_number = n;
    }

    inline void cv_captcha::create() {
        this->content.clear();
        int p = 0;
        for (int i = 0; i<this->n; ++i) {
            int font_face = this->get_random_font_face();
            double font_scale = 1.0;
            int thickness = 2;
            std::string text = this->get_random_char(1);
            this->content.append(Poco::toLower(text));
            int baseline = 0;
            cv::Size text_size = cv::getTextSize(text, font_face,
                    font_scale, thickness, &baseline);

            p += (i == 0 ? text_size.width / 2 : text_size.width + 12);
            cv::Point text_org(p, (this->matrix.rows + text_size.height) / 2 + (this->rng.nextBool() ? -5 : 5));


            cv::putText(this->matrix, text, text_org, font_face, font_scale,
                    this->random_color(), thickness, CV_AA);
        }

        if (this->lined)
            this->draw_random_line(this->line_number);
        if (this->ellipsed)
            this->draw_random_ellipse(this->ellipse_number);
        if (this->slated)
            this->draw_random_slate(this->slate_number);


    }

    inline void cv_captcha::write_to_file(const std::string& filepath) {
        cv::imwrite(filepath, this->matrix);
    }

    inline void cv_captcha::draw_random_line(int n) {
        cv::Point pt1, pt2;

        for (int i = 0; i < n; i++) {
            pt1.x = this->rng.next(this->matrix.cols);
            pt1.y = this->rng.next(this->matrix.rows);
            pt2.x = this->rng.next(this->matrix.cols);
            pt2.y = this->rng.next(this->matrix.rows);

            cv::line(this->matrix, pt1, pt2, this->random_color(), this->uniform(1, 2), 1);
        }

    }

    inline void cv_captcha::draw_random_ellipse(int n) {
        for (int i = 0; i < n; ++i) {
            cv::ellipse(this->matrix,
                    cv::Point(this->rng.next(this->matrix.cols), this->rng.next(this->matrix.cols)),
                    cv::Size(this->rng.next(this->matrix.rows), this->rng.next(this->matrix.rows)),
                    this->uniform(0, 360),
                    0,
                    360,
                    this->random_color(),
                    1,
                    2);
        }
    }

    inline void cv_captcha::draw_random_slate(int n) {
        int i, j;
        for (int k = 0; k < n; ++k) {
            i = this->rng.next(this->matrix.cols);
            j = this->rng.next(this->matrix.rows);

            if (this->matrix.channels() == 1) {
                this->matrix.at<uchar>(j, i) = this->rng.next(255);
            } else {
                this->matrix.at<cv::Vec3b>(j, i)[0] = this->rng.next(255);
                this->matrix.at<cv::Vec3b>(j, i)[1] = this->rng.next(255);
                this->matrix.at<cv::Vec3b>(j, i)[2] = this->rng.next(255);
            }
        }
    }

    inline cv::Scalar cv_captcha::random_color() {
        return cv::Scalar(this->rng.next(255), this->rng.next(255), this->rng.next(255));
    }

    template<typename T>
    inline T cv_captcha::uniform(T a, T b) {
        return a == b ? a : (T) (this->rng.next(b - a) + a);
    }

    inline int cv_captcha::get_width() {
        return this->matrix.cols;
    }

    inline int cv_captcha::get_height() {
        return this->matrix.rows;
    }

    std::ostream& operator<<(std::ostream& out, const cv_captcha & x) {
        std::vector<uchar> result;
        cv::imencode(".png", x.matrix, result);
        for (auto& item : result) {
            out << item;
        }
        return out;
    }

}

#endif /* CV_CAPTCHA_HPP */

