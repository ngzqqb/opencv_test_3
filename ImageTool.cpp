﻿#include <cassert>
#include <string>
#include <numeric>
#include "ImageTool.hpp"

namespace sstd {

    namespace private_eval_angle {

        class LineItem : public QLineF {
        public:
            double thisLineLength/*线段长度*/;
            double thisAngle;

            inline LineItem(const QLineF & arg) :
                QLineF{ arg },
                thisLineLength{ arg.length() }{
            }

            inline void fullConstruct() {
                thisAngle = std::atan2(this->dy(), this->dx());
                if (thisAngle < 0) {
                    thisAngle += 3.1415927;
                }
                thisAngle = (180 / 3.141592654) * thisAngle;
            }

            inline LineItem() :
                QLineF{ {1,0},{0,0} },
                thisLineLength{ 0 },
                thisAngle{ 0 }{
            }

            inline LineItem(const LineItem &) = delete;
            inline LineItem(LineItem &&) = delete;
            inline LineItem&operator=(const LineItem &) = delete;
            inline LineItem&operator=(LineItem &&) = delete;
        };

        /* 预处理 */
        inline cv::Mat toGray(const cv::Mat & arg) {

            if ((arg.cols < 64) || (arg.rows < 64)) {/* 图片太小 */
                return {};
            }

            cv::Mat varEvalImage;

            /*重新调整大小，仅仅计算角度没有必要太大*/
            cv::resize(arg,
                varEvalImage,
                cv::Size{ arg.cols / 4 ,arg.rows / 4 },
                cv::INTER_CUBIC);

            if constexpr(false){/* 对图像进行锐化 */
                const static cv::Mat varKernel = []()->cv::Mat {
                    cv::Mat varAns(3, 3, CV_32F, cv::Scalar(0));
                    varAns.at<float>(1, 1) = 5.0;
                    varAns.at<float>(0, 1) = -1.0;
                    varAns.at<float>(1, 0) = -1.0;
                    varAns.at<float>(1, 2) = -1.0;
                    varAns.at<float>(2, 1) = -1.0;
                    return std::move(varAns);
                }();
                cv::filter2D(varEvalImage, 
                    varEvalImage, 
                    varEvalImage.depth(), 
                    varKernel);
            } else {
                cv::GaussianBlur(varEvalImage, varEvalImage, cv::Size(5, 5), 3, 3);
            }

            /*计算Yellow颜色的蒙版*/
            cv::Mat varIsYellow;

            {
                cv::Mat varHSVImage;

                /* 计算hsv图像 */
                cv::cvtColor(varEvalImage, varHSVImage, cv::COLOR_BGR2HSV);

                {/*H: 0-180，S： 0-255，V：0-255 */
                    /* https://www.jianshu.com/p/7361652e15b8 */
                    cv::Mat varHSV[3];
                    cv::split(varHSVImage, varHSV);

                    const auto varH = varHSV[0];
                    auto varS = varHSV[1];
                    auto varV = varHSV[2];

                    /* 橙色 或 黄色 */
                    varIsYellow = (
                        (varS > 15) &
                        (varV > 15) &
                        (varH > 20) &/* yellow : 23 to 40 */
                        (varH < 43));
                }

            }

            /* 将黄色替换为白色 */
            varEvalImage.setTo(cv::Scalar(255, 255, 255), varIsYellow);

            {/* 直方图均衡化 */
                cv::Mat varBGR[3];
                cv::split(varEvalImage, varBGR);
                cv::equalizeHist(varBGR[0], varBGR[0]);
                cv::equalizeHist(varBGR[1], varBGR[1]);
                cv::equalizeHist(varBGR[2], varBGR[2]);
                cv::merge(varBGR, 3, varEvalImage);
            }

            /* 将BGR转为Gray */
            cv::cvtColor(varEvalImage, varEvalImage, cv::COLOR_BGR2GRAY);

            {/* 二值化 */
                const auto varAllSize =
                    static_cast<std::int64_t>(varEvalImage.cols) * varEvalImage.rows;
                std::vector< std::uint8_t > varItems;
                varItems.reserve(static_cast<std::size_t>(varAllSize));
                auto varPtr = varEvalImage.data;
                for (int varI = 0; varI < varEvalImage.rows; ++varI) {
                    auto varRow = varPtr;
                    for (int varJ = 0; varJ < varEvalImage.cols; ++varJ) {
                        varItems.push_back(varRow[varJ]);
                    }
                    varPtr += varEvalImage.step;
                }
                auto varPos = varItems.begin() + static_cast<int>(0.05 + varItems.size()*0.1);
                std::nth_element(
                    varItems.begin(),
                    varPos,
                    varItems.end());
                const int varSplitValue = (*varPos != 255u) ? (*varPos) : 254u;
                cv::threshold(varEvalImage,
                    varEvalImage,
                    varSplitValue, 255,
                    cv::THRESH_BINARY_INV);
            }

            if constexpr(true) {/*形态学滤波*/
                const auto varC2 = cv::getStructuringElement(cv::MORPH_ELLIPSE, { 3,3 });// cv::Mat(5, 5, CV_8UC1, cv::Scalar(1));
                const auto varC1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, { 3,3 });
        
                    cv::erode(varEvalImage,
                        varEvalImage,
                        varC2,
                        { -1,-1 }, 1);
                    cv::dilate(varEvalImage,
                        varEvalImage,
                        varC1,
                        { -1,-1 }, 2);
                    cv::erode(varEvalImage,
                        varEvalImage,
                        varC2,
                        { -1,-1 }, 2);
            }

            return std::move(varEvalImage);

        }

        inline cv::Mat loadImage(const QString & arg) {
            auto const varImageName = arg.toLocal8Bit();
            auto varBGRColor = cv::imread({ varImageName.data(),
                                static_cast<std::size_t>(varImageName.size()) }, cv::IMREAD_COLOR);
            return toGray(varBGRColor);
        }

        /* https://blog.csdn.net/wsp_1138886114/article/details/83793331 */
        inline cv::Mat histogramImage(const cv::Mat & arg) {
            cv::Mat varAns;
            if constexpr (false) {
                auto varCLAHE = cv::createCLAHE(40.0, { 8,8 });
                varCLAHE->apply(arg, varAns);
            } else {
                cv::equalizeHist(arg, varAns);
            }
            return std::move(varAns);
        }

        inline cv::Mat cannyImage(const cv::Mat & arg) {
            cv::Mat varAns;
            cv::Canny(arg, varAns, 86, 180);
            return std::move(varAns);
        }

        inline QImage toQImage(const cv::Mat & arg) {
            if (arg.empty()) {
                return{};
            }
            const auto varImageWidth = arg.cols;
            const auto varImageHeight = arg.rows;

            assert(arg.channels() == 1);
            auto varImageWrap = std::make_unique< cv::Mat>();
            arg.convertTo(*varImageWrap, CV_8UC1);
            assert(varImageWidth == varImageWrap->cols);
            assert(varImageHeight == varImageWrap->rows);
            return QImage{
                varImageWrap->data,
                        varImageWidth,
                        varImageHeight,
                        static_cast<int>(varImageWrap->step),
                        QImage::Format_Grayscale8,
                        [](void * arg) { delete reinterpret_cast<cv::Mat *>(arg); },
                varImageWrap.release() };
        }

        inline std::vector< std::shared_ptr<LineItem> > houghLinesP(const cv::Mat & arg) {
            auto varLessLength = [](const auto & l, const auto & r) {
                return l->thisLineLength > r->thisLineLength;
            };
            std::vector< std::shared_ptr<LineItem> > varAns;
            {/*霍夫曼找直线...*/
                std::vector<cv::Vec4i> varLines;
                cv::HoughLinesP(arg,
                    varLines,
                    1.05/*半径分辨率*/,
                    CV_PI / 180/*角度分辨率*/,
                    30/*直线点数阈值*/,
                    120/*直线长度阈值*/,
                    16/*最大距离*/);
                varAns.reserve(varLines.size());
                for (const auto & varI : varLines) {
                    varAns.push_back(std::make_shared<LineItem>(QLineF{
                        QPoint{ varI[0],varI[1] },
                        QPoint{ varI[2],varI[3] } }));
                }
            }
            constexpr std::size_t varMaxEleSize = 128;
            if (varAns.size() > varMaxEleSize) {/* 最多保留前 varMaxEleSize 项 */
                std::nth_element(varAns.begin(),
                    varAns.begin() + varMaxEleSize,
                    varAns.end(),
                    varLessLength);
                varAns.resize(varMaxEleSize);
            }
            /* 从大到小排序 */
            std::sort(varAns.begin(), varAns.end(), varLessLength);
            /* 完整构造 */
            for (auto & varI : varAns) {
                varI->fullConstruct();
            }
            return std::move(varAns);
        }

    }/**/

    cv::Mat loadImage(const QString & arg) {
        namespace ps = private_eval_angle;
        return ps::loadImage(arg);
    }

    double evalAngle(const QString & arg) try {
        namespace ps = private_eval_angle;
        std::vector< std::shared_ptr<ps::LineItem> > varLines;
        {/**/
            cv::Mat varCannyImage;
            {/**/
                cv::Mat varHistogramImage;
                {/*  */
                    cv::Mat varSourceImage;
                    varSourceImage = ps::loadImage(arg);
                    varHistogramImage = ps::histogramImage(varSourceImage);
                }
                varCannyImage = ps::cannyImage(varHistogramImage);
            }
            varLines = ps::houghLinesP(varCannyImage);
            if constexpr (false) {/* 测试绘制霍夫曼直线 */
                QImage varImage{ arg };
                {
                    QPainter varPainter{ &varImage };
                    for (const auto & varI : varLines) {
                        varPainter.setPen(QPen{ QColor(230,std::rand() & 255,std::rand() & 127),2 });
                        varPainter.drawLine(*varI);
                    }
                }
                varImage.save(arg + QStringLiteral(".bmp"));
            }
        }

        if (varLines.empty()) {/*没有找到符合要求的直线 .... */
            return 0;
        }

        if (varLines.size() == 1) {/*只找到一条符合要求的直线 ... */
            return varLines[0]->thisAngle;
        }

        { /* 统计角度高峰 ...  */
            class CountItem {
            public:
                std::list< std::shared_ptr<ps::LineItem > > items;
                double mean{ 0 };
                void evalMean(double arg) {
                    mean = 0;
                    if (items.empty()) {
                        return;
                    }
                    const auto varRate = 1.0 / items.size();
                    for (const auto & varI : items) {
                        auto varJudge = arg - varI->thisAngle;
                        if (varJudge > 45) {/* 179 - 0 */
                            mean = std::fma(varRate, varI->thisAngle + 180, mean);
                        } else if (varJudge < -45) { /* 0 - 180 */
                            mean = std::fma(varRate, varI->thisAngle - 180, mean);
                        } else {
                            mean = std::fma(varRate, varI->thisAngle, mean);
                        }
                    }
                }
            };
            constexpr int varAngleStep = 3;
            static_assert(varAngleStep < 30);
            constexpr int varArrayCount = int((180. / varAngleStep));
            std::array< CountItem, varArrayCount > varAngleCount;
            for (const auto & varI : varLines) {/* 每一个角度加入 angle +/- varAngleStep 三个柱状图 */

                const int varIndex = std::min(varArrayCount - 1,
                    std::max(0, int(varI->thisAngle / varAngleStep)));

                int varNextIndex = varIndex + 1;
                int varBeforeIndex = varIndex - 1;

                if (varBeforeIndex < 0) {
                    varBeforeIndex = varArrayCount - 1;
                }

                if (varNextIndex >= varArrayCount) {
                    varNextIndex = 0;
                }

                varAngleCount[varIndex].items.push_back(varI);
                varAngleCount[varNextIndex].items.push_back(varI);
                varAngleCount[varBeforeIndex].items.push_back(varI);

            }

            {
                int varIndex = 0;
                for (auto & varI : varAngleCount) {
                    varI.evalMean((varIndex + 0.5) * varAngleStep);
                    ++varIndex;
                }
            }

            auto varElement = std::max_element(varAngleCount.begin(), varAngleCount.end(),
                [](const auto & l, const auto & r) {/* size 大的 mean 小的 */
                if (l.items.size() < r.items.size()) {
                    return true;
                }
                if (l.items.size() > r.items.size()) {
                    return false;
                }
                return l.mean > r.mean;
            });

            return varElement->mean;
        }

    } catch (...) {
        return 0;
    }

    cv::Mat rotateExternImage(
        const QString & argImageInput,
        double argAngle,
        int argMargin) try {
        auto const varImageName = argImageInput.toLocal8Bit();
        return rotateExternImage(
            cv::imread({ varImageName.data(), static_cast<std::size_t>(varImageName.size()) }),
            argAngle,
            argMargin);
    } catch (const std::exception & e) {
        std::cout << e.what() << std::endl;
        return {};
    } catch (...) {
        std::cout << "unknow exception ." << std::endl;
        return {};
    }

    cv::Mat rotateExternImage(
        const cv::Mat & argImageInput,
        double argAngle,
        int argMargin) try {
        cv::Mat argImage;
        cv::copyMakeBorder(argImageInput, argImage,
            argMargin, argMargin, argMargin, argMargin,
            cv::BORDER_CONSTANT,
            cv::Scalar(255, 255, 255));
        if constexpr (false) {
            cv::copyMakeBorder(argImage, argImage,
                1, 1, 1, 1,
                cv::BORDER_CONSTANT,
                cv::Scalar(2, 2, 2));
        }
        if (argImageInput.empty()) {
            return std::move(argImage);
        }
        cv::Point2f varCenter(
            (argImage.cols - 1) / 2.0,
            (argImage.rows - 1) / 2.0);
        cv::Mat varRotateMatrix =
            cv::getRotationMatrix2D(varCenter, argAngle, 1.0);
        cv::Rect2f varBoundBox = cv::RotatedRect(cv::Point2f(),
            argImage.size(),
            argAngle).boundingRect2f();
        varRotateMatrix.at<double>(0, 2) +=
            varBoundBox.width / 2.0 - argImage.cols / 2.0;
        varRotateMatrix.at<double>(1, 2) +=
            varBoundBox.height / 2.0 - argImage.rows / 2.0;
        cv::Mat varRotatedImage;
        cv::warpAffine(argImage,
            varRotatedImage/*dst*/,
            varRotateMatrix,
            varBoundBox.size(),
            cv::INTER_CUBIC,
            cv::BORDER_CONSTANT,
            cv::Scalar(255, 255, 255));
        return std::move(varRotatedImage);
        /* https://stackoverflow.com/questions/22041699/rotate-an-image-without-cropping-in-opencv-in-c */
    } catch (const std::exception & e) {
        std::cout << e.what() << std::endl;
        return {};
    } catch (...) {
        std::cout << "unknow exception ." << std::endl;
        return {};
    }

    bool saveImage(const cv::Mat & arg, const QString & argFileName) {
        const auto varImageName = argFileName.toLocal8Bit();
        const cv::String varCVFileName{ varImageName.data(),
                    static_cast<std::size_t>(varImageName.size()) };
        return cv::imwrite(varCVFileName, arg);
    }

}/*namespace sstd*/

// https://github.com/ngzHappy/oct2
// https://github.com/ngzHappy/oct2/tree/master/opencv_data/opencv_pca

