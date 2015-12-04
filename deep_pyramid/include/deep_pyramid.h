#ifndef __DEEP_PYRAMID_H__
#define __DEEP_PYRAMID_H__

#include <iostream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>
#include <caffe/caffe.hpp>
#include <caffe/common.hpp>

// FIX: cтранное использование директивы условной компиляции
#if 1
#include <stdio.h>
#define TIMER_START(name) int64 t_##name = getTickCount()
#define TIMER_END(name) printf("TIMER_" #name ":\t%6.2fms\n", \
    1000.f * ((getTickCount() - t_##name) / getTickFrequency()))
#else
#define TIMER_START(name)
#define TIMER_END(name)
#endif

#define OBJECT 1
#define NOT_OBJECT -1

// FIX: переместить в отдельный модуль
class ObjectBox
{
public:
    double confidence;
    int level;
    cv::Rect norm5Box;
    cv::Rect pyramidImageBox;
    cv::Rect originalImageBox;
    bool operator<(const ObjectBox &object) const
    {
        return confidence<object.confidence;
    }
};

enum DeepPyramidMode
{
    DETECT,
    TRAIN,
    TEST
};

// FIX: переместить в отдельный модуль
class DeepPyramidConfiguration
{
public:
    DeepPyramidMode mode;

    std::string model_file;
    std::string trained_net_file;

    int numLevels;

    cv::Scalar objectRectangleColor;

    std::string svm_trained_file;
    cv::Size filterSize;

    DeepPyramidConfiguration(){};
    DeepPyramidConfiguration(std::string deep_peramid_config, DeepPyramidMode mode);
};

// FIX: переместить в отдельный модуль
class DeepPyramid
{
public:
    int getNorm5ChannelsCount()
    {
        return norm5[0].size();
    }

    DeepPyramidConfiguration config;
    void addRootFilter(cv::Size filterSize, CvSVM* classifier);
    void setImg(cv::Mat &img);
    void detect(cv::Mat &img, std::vector<ObjectBox>&);
    DeepPyramid(int num_levels, const std::string& model_file,
                const std::string& trained_file);
    cv::Mat getImageWithObjects();
    void calculateToNorm5(cv::Mat &image);
    int getNumLevel();
    void getFeatureVector(int levelIndx, cv::Point position, cv::Size size, cv::Mat& feature);
    cv::Size originalImageSize();
    int norm5SideLength();
    
    //Rectangle transform
    cv::Rect boundingBoxAtLevel(int i, cv::Rect originalRect);
    cv::Rect getRectByNorm5Rect(cv::Rect rect);
    cv::Rect getNorm5RectByOriginal(cv::Rect originalRect);
    int chooseLevel(cv::Size filterSize, cv::Rect boundBox);
    cv::Mat getNorm5(int level, int channel);
    void clearFilter();
    cv::Mat getFeatureVector(cv::Rect rect, cv::Size size);

    std::string to_string(int i);
    DeepPyramid(std::string detector_config, DeepPyramidMode mode);

private:
    cv::Mat originalImg;
    cv::Mat originalImgWithObjects;
    int num_levels;
    std::vector<cv::Mat> imagePyramid;
    std::vector< std::vector<cv::Mat> > max5;
    std::vector< std::vector<cv::Mat> > norm5;
    std::vector<float> meanValue;
    std::vector<float> deviationValue;
    std::vector<cv::Size> rootFilterSize;
    std::vector<CvSVM*> rootFilterSVM;
    std::vector<ObjectBox> allObjects;
    std::vector<ObjectBox> detectedObjects;

    void drawObjects();
    caffe::shared_ptr<caffe::Net<float> > net_;
    int sideNetInputSquare;
    int num_channels;

    void showImagePyramid();
    void createMax5PyramidTest();
    void showNorm5Pyramid();
    
    //Image Pyramid
    cv::Size calculateLevelPyramidImageSize(int i);
    cv::Mat createLevelPyramidImage(int i);
    void createImagePyramid();

    //NeuralNet
    cv::Mat convertToFloat(cv::Mat img);
    void fillNeuralNetInput(int i);
    void calculateNet();
    void calculateNetAtLevel(int i);
    std::vector<cv::Mat> wrapNetOutputLayer();

    //Max5
    std::vector<cv::Mat> createLevelPyramidMax5(int i);
    void createMax5Pyramid();

    //Norm5
    std::vector<cv::Mat> createLevelPyramidNorm5(int i);
    void createNorm5Pyramid();

    //Root-Filter sliding window
    void rootFilterAtLevel(int rootFilterIndx, int levelIndx, int stride);
    void rootFilterConvolution();

    //Rectangle
    void calculateImagePyramidRectangle();
    void calculateOriginalRectangle();
    void groupOriginalRectangle();

    //Rectangle transform ARTICLE
    cv::Rect getRectByNorm5Pixel_ARTICLE(cv::Point point);
    cv::Rect getRectByNorm5Rect_ARTICLE(cv::Rect rect);
    cv::Rect getNorm5RectByOriginal_ARTICLE(cv::Rect originalRect);
    int centerConformity;
    int boxSideConformity;
    void clear();

};

// FIX: переместить в утилиты
double IOU(cv::Rect r1,cv::Rect r2);
#endif // DEEP_PYRAMID_H
