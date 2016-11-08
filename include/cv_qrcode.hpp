#ifndef CV_QRCODE_HPP
#define CV_QRCODE_HPP

#include <string>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <Poco/Random.h>
#include <qrencode.h>

namespace webcppd {

    class cv_qrcode {
    public:

        cv_qrcode(const std::string& text, int width = 180, int version = 2)
        : version(version), width(width), height(width), image(), successful(false) {
            QRcode * qr = QRcode_encodeString(text.c_str(),
                    this->version,
                    QR_ECLEVEL_L,
                    QR_MODE_8,
                    1);
            if (qr) {
                Poco::Random rng;
                cv::Mat src(qr->width, qr->width, CV_8UC3, cv::Scalar::all(255));
                uchar tmp;
                int color[3];
                color[0] = rng.next(255), color[1] = rng.next(255), color[2] = rng.next(255);
                for (int i = 0; i < qr->width; ++i) {
                    for (int j = 0; j < qr->width; ++j) {
                        tmp = (qr->data[i * qr->width + j]);
                        if (tmp & 0x01) {
                            src.at<cv::Vec3b>(i, j)[0] = color[0]; //rng.next(255);
                            src.at<cv::Vec3b>(i, j)[1] = color[1]; //rng.next(255);
                            src.at<cv::Vec3b>(i, j)[2] = color[2]; //rng.next(255);
                        }
                    }
                }
                QRcode_free(qr);

                CvSize dstSize;
                dstSize.width = this->width, dstSize.height = this->height;
                IplImage srcp = src;
                IplImage* dstp = cvCreateImage(dstSize, srcp.depth, srcp.nChannels);
                cvResize(&srcp, dstp, CV_INTER_AREA);
                cv::Mat tmpMat(dstp);

                int top, bottom, left, right;
                double border = 0.5;
                top = (int) (border * src.rows);
                bottom = (int) (border * src.rows);
                left = (int) (border * src.cols);
                right = (int) (border * src.cols);
                cv::Scalar borderColor = cv::Scalar::all(255);
                this->image = tmpMat;
                cv::copyMakeBorder(tmpMat, this->image, top, bottom, left, right, cv::BORDER_ISOLATED, borderColor);
                this->successful = true;
            }
        }
        cv_qrcode() = delete;
        cv_qrcode(const cv_qrcode& orig) = delete;
        virtual ~cv_qrcode() = default;

        bool created() {
            return this->successful;
        }

        friend std::ostream & operator<<(std::ostream& out, const cv_qrcode& x) {
            std::vector<uchar> result;
            cv::imencode(".png", x.image, result);
            for (auto & item : result) {
                out << item;
            }
            return out;
        }
    private:
        int version, width, height;
        cv::Mat image;
        bool successful;
    };
}

#endif /* CV_QRCODE_HPP */

