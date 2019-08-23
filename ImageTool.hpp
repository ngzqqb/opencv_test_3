#pragma once

#include <memory>
#include <vector>

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <opencv2/opencv.hpp>

namespace sstd {

    double evalAngle(const QString &);
    cv::Mat rotateExternImage(const cv::Mat &,double,int=64);
    cv::Mat rotateExternImage(const QString &, double, int = 64);
    bool saveImage(const cv::Mat & arg, const QString & argFileName);

}/*namespace sstd*/

