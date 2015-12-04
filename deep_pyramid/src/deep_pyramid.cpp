#include <sstream>
#include <math.h>
#include <assert.h>
#include <time.h>

#include "deep_pyramid.h"
#include "nms.h"

using namespace cv;
using namespace std;
using namespace caffe;

string DeepPyramid::to_string(int i)
{
    ostringstream stm ;
    stm << i ;
    return stm.str() ;
}

Rect DeepPyramid::getRectByNorm5Pixel_ARTICLE(Point p)
{
    Point center=p*centerConformity;
    //high-left
    Point hl;
    int x,y;
    x=center.x-boxSideConformity;
    y=center.y-boxSideConformity;
    hl.x=x>0 ? x : 0;
    hl.y=y>0 ? y : 0;

    //down-right
    Point dr;
    x=center.x+boxSideConformity;
    y=center.y+boxSideConformity;
    dr.x=x<sideNetInputSquare ? x : sideNetInputSquare-1;
    dr.y=y<sideNetInputSquare ? y : sideNetInputSquare-1;

    return Rect(hl,dr);
}

Rect DeepPyramid::getRectByNorm5Rect_ARTICLE(Rect r)
{
    Point hl=Point(r.x,r.y);
    Point dr=Point(r.x+r.width,r.y+r.height);
    Rect r1=getRectByNorm5Pixel_ARTICLE(hl);
    Rect r2=getRectByNorm5Pixel_ARTICLE(dr);
    Point RectHL=Point(r1.x,r1.y);
    Point RectDR=Point(r2.x+r2.width, r2.y+r2.height);

    return Rect(RectHL, RectDR);
}

Rect DeepPyramid::getRectByNorm5Rect(Rect r)
{
    Blob<float>* input_layer = net_->input_blobs()[0];
    Blob<float>* output_layer = net_->output_blobs()[0];
    double scale=input_layer->width()/(double)output_layer->width();

    Rect rect;
    rect.x=r.x*scale;
    rect.y=r.y*scale;
    rect.width=r.width*scale;
    rect.height=r.height*scale;

    return rect;
}

Rect DeepPyramid::getNorm5RectByOriginal_ARTICLE(Rect originalRect)
{
    Point hl=Point(originalRect.x,originalRect.y);
    Point dr=Point(originalRect.x+originalRect.width,originalRect.y+originalRect.height);
    Point norm5HL=(hl+Point(boxSideConformity,boxSideConformity))*(1/(double)centerConformity);
    Point norm5DR=(dr-Point(boxSideConformity,boxSideConformity))*(1/(double)centerConformity);
    Rect r;
    if(norm5HL.x<norm5DR.x)
    {
        r.x=norm5HL.x;
        r.width=norm5DR.x-norm5HL.x;
    }
    else
    {
        r.x=(rand()%2)==0 ? norm5DR.x : norm5HL.x;
        r.width=1;
    }
    if(norm5HL.y<norm5DR.y)
    {
        r.y=norm5HL.y;
        r.height=norm5DR.y-norm5HL.y;
    }
    else
    {
        r.y=(rand()%2)==0 ? norm5DR.y : norm5HL.y;
        r.height=1;
    }
    return r;
}

Rect DeepPyramid::getNorm5RectByOriginal(Rect originalRect)
{
    Blob<float>* input_layer = net_->input_blobs()[0];
    Blob<float>* output_layer = net_->output_blobs()[0];
    double scale=input_layer->width()/(double)output_layer->width();

    Rect r;
    r.x=originalRect.x/scale;
    r.y=originalRect.y/scale;
    r.width=originalRect.width/scale;
    r.height=originalRect.height/scale;

    return r;
}

Rect DeepPyramid::boundingBoxAtLevel(int i, Rect originalRect)
{
    double longSide;
    if(originalImg.cols>originalImg.rows)
    {
        longSide=originalImg.cols;
    }
    else
    {
        longSide=originalImg.rows;
    }
    double scale = (sideNetInputSquare/pow(2, (num_levels-1-i)/2.0))/longSide;
    originalRect.x*=scale;
    originalRect.y*=scale;
    originalRect.height*=scale;
    originalRect.width*=scale;
    return originalRect;
}

void DeepPyramid::createMax5PyramidTest()
{
    createMax5Pyramid();
}
void DeepPyramid::showNorm5Pyramid()
{
    for(unsigned int i=0;i<norm5.size();i++)
        for(unsigned int j=0;j<norm5[i].size();j++)
        {
            imshow("norm5| "+to_string(i)+" | "+to_string(j), norm5[i][j]);
            waitKey(0);
            destroyWindow("norm5| "+to_string(i)+" | "+to_string(j));
        }
}

void DeepPyramid::setImg(Mat img)
{
    img.copyTo(originalImg);
}

void DeepPyramid::showImagePyramid()
{
    for(int i=0;i<num_levels;i++)
    {
        imshow("Image level: "+to_string(i+1),imagePyramid[i]);
        waitKey(0);
        destroyWindow("Image level: "+to_string(i+1));
    }
}

//Image Pyramid
//
Size DeepPyramid::calculateLevelPyramidImageSize(int i)
{
    Size levelPyramidImageSize;
    if(originalImg.rows<=originalImg.cols)
    {
        levelPyramidImageSize.width=sideNetInputSquare/pow(2,(num_levels-1-i)/2.0);
        levelPyramidImageSize.height=levelPyramidImageSize.width*(originalImg.rows/((double)originalImg.cols));
    }
    else
    {
        levelPyramidImageSize.height=sideNetInputSquare/pow(2,(num_levels-1-i)/2.0);
        levelPyramidImageSize.width=levelPyramidImageSize.height*(originalImg.cols/((double)originalImg.rows));
    }

    return levelPyramidImageSize;
}

Mat DeepPyramid::createLevelPyramidImage(int i)
{
    Mat levelImg(sideNetInputSquare,sideNetInputSquare,CV_8UC3,Scalar(0,0,0));
    Size pictureSize=calculateLevelPyramidImageSize(i);
    Mat resizedImg;
    resize(originalImg, resizedImg, pictureSize);
    resizedImg.copyTo(levelImg(Rect(Point(0,0),pictureSize)));

    return levelImg;
}

void DeepPyramid::createImagePyramid()
{
    imagePyramid.clear();
    for(int i=0;i<num_levels;i++)
    {
        imagePyramid.push_back(createLevelPyramidImage(i));
    }
    cout<<"Create image pyramid..."<<endl;
    cout<<"Status: Success!"<<endl;
}
//
////

//NeuralNet
//
void DeepPyramid::calculateNet()
{
    net_->ForwardPrefilled();
}

Mat DeepPyramid::convertToFloat(Mat img)
{
    Mat img_float;
    if (img.channels() == 3)
    {
        img.convertTo(img_float, CV_32FC3);
    }
    else
    {
        if(img.channels()==1)
        {
            img.convertTo(img_float, CV_32FC1);
        }
    }

    return img_float;
}

void DeepPyramid::fillNeuralNetInput(int i)
{
    Mat img=imagePyramid[i];
    Blob<float>* input_layer = net_->input_blobs()[0];
    input_layer->Reshape(1, num_channels,
                         sideNetInputSquare, sideNetInputSquare);
    net_->Reshape();

    std::vector<Mat> input_channels;
    int width = input_layer->width();
    int height = input_layer->height();
    float* input_data = input_layer->mutable_cpu_data();
    for (int i = 0; i < input_layer->channels(); ++i) {
        Mat channel(height, width, CV_32FC1, input_data);
        input_channels.push_back(channel);
        input_data += width * height;
    }

    Mat img_float=convertToFloat(img);
    split(img_float, input_channels);
}

void DeepPyramid::calculateNetAtLevel(int i)
{
    const clock_t begin_time = clock();
    cout<<"Calculate level: "+to_string(i+1)+" ..."<<endl;
    fillNeuralNetInput(i);
    calculateNet();
    cout << "Time:"<<float( clock () - begin_time ) /  CLOCKS_PER_SEC<<" s."<<endl;
    cout<<"Status: Success!"<<endl;

}

std::vector<Mat> DeepPyramid::wrapNetOutputLayer()
{
    Blob<float>* output_layer = net_->output_blobs()[0];
    const float* begin = output_layer->cpu_data();
    float* data=new float[output_layer->height()*output_layer->width()];

    std::vector<Mat> max5Level;

    for(int k=0;k<output_layer->channels();k++)
    {
        for(int i=0;i<output_layer->height()*output_layer->width();i++)
        {
            data[i]=begin[i+output_layer->height()*output_layer->width()*k];
        }
        Mat conv(output_layer->height(),output_layer->width(), CV_32FC1, data);
        max5Level.push_back(conv.clone());
    }

    return max5Level;
}
//
////

//Max5
//
std::vector<Mat> DeepPyramid::createLevelPyramidMax5(int i)
{
    calculateNetAtLevel(i);
    return wrapNetOutputLayer();
}
void DeepPyramid::createMax5Pyramid()
{
    for(int i=0;i<num_levels;i++)
    {
        max5.push_back(createLevelPyramidMax5(i));
    }
}
//
////

//Norm5
//
Mat uniteMats(std::vector<Mat> m)
{
    Mat unite(1,0,CV_32FC1);
    for(unsigned int i=0;i<m.size();i++)
    {
        unite.push_back(m[i].reshape(1,1));
    }

    return unite;
}

void calculateMeanAndDeviationValue(std::vector<Mat> level, float& meanValue, float& deviationValue)
{
    Mat unite=uniteMats(level);
    Mat mean, deviation;
    meanStdDev(unite,mean,deviation);
    meanValue=mean.at<double>(0,0);
    deviationValue=deviation.at<double>(0,0);
}

std::vector<Mat> DeepPyramid::createLevelPyramidNorm5(int i)
{
    float mean,deviation;
    calculateMeanAndDeviationValue(max5[i],mean,deviation);
    vector<Mat> norm5;
    for(unsigned int j=0;j<max5[i].size();j++)
    {
        norm5.push_back((max5[i][j]-mean)/deviation);
    }

    return norm5;
}

void DeepPyramid::createNorm5Pyramid()
{
    for(unsigned int i=0;i<max5.size();i++)
    {
        norm5.push_back(createLevelPyramidNorm5(i));
    }
}
//
////

//Root-Filter sliding window
//
void DeepPyramid::getFeatureVector(int levelIndx, Point position, Size size, Mat& feature)
{
    for(unsigned int k=0;k<norm5[levelIndx].size();k++)
    {
        Mat m;
        norm5[levelIndx][k](Rect(position, size)).copyTo(m);
        for(int h=0;h<size.height;h++)
        {
            for(int w=0;w<size.width;w++)
            {
                feature.at<float>(0,w+h*size.width+k*size.height*size.width)=m.at<float>(h, w);
            }
        }
    }
}

Mat DeepPyramid::getFeatureVector(Rect rect, Size size)
{
    int bestLevel=chooseLevel(size, rect);

    Rect imagePyramidBounigBox=boundingBoxAtLevel(bestLevel, rect);
    Rect objectRect=getNorm5RectByOriginal(imagePyramidBounigBox);
    vector<Mat> norm5Resized;
    for(int j=0;j<256;j++)
    {
        Mat m;
        Mat objectMat=norm5[bestLevel][j];
        resize(objectMat(objectRect), m, size);
        norm5Resized.push_back(m);
    }

    cv::Mat feature(1, size.height*size.width*norm5.size(),CV_32FC1);
    for(int w=0;w<size.width;w++)
    {
        for(int h=0;h<size.height;h++)
        {
            for(unsigned int k=0;k<norm5.size();k++)
            {
                int featureIndex=w+h*size.width+k*size.height*size.width;
                feature.at<float>(0,featureIndex)=norm5Resized[k].at<float>(h,w);
            }
        }
    }

    return feature;
}

void DeepPyramid::rootFilterAtLevel(int rootFilterIndx, int levelIndx, int stride)
{
    Size filterSize=rootFilterSize[rootFilterIndx];
    CvSVM* filterSVM=rootFilterSVM[rootFilterIndx];
    int stepWidth, stepHeight;
    cout<<"here!"<<endl;
    stepWidth=((norm5[levelIndx][0].cols/pow(2,(num_levels-1-levelIndx)/2.0)-filterSize.width)/stride)+1;
    stepHeight=((norm5[levelIndx][0].rows/pow(2,(num_levels-1-levelIndx)/2.0)-filterSize.height)/stride)+1;
    int detectedObjectCount=0;

    for(int w=0;w<stepWidth;w+=stride)
        for(int h=0;h<stepHeight;h+=stride)
        {

            Point p(stride*w,stride*h);
            cv::Mat feature(1,filterSize.height*filterSize.width*norm5[levelIndx].size(),CV_32FC1);
            getFeatureVector(levelIndx,p,filterSize, feature);

            int predict;
            predict=filterSVM->predict(feature);

            if(predict==OBJECT)
            {
                ObjectBox object;
                object.confidence=std::fabs(filterSVM->predict(feature,true));
                object.level=levelIndx;
                object.norm5Box=Rect(p,filterSize);
                allObjects.push_back(object);
                detectedObjectCount++;
            }
        }
    cout<<"Count of detected objects: "<<detectedObjectCount<<endl;
}

void DeepPyramid::rootFilterConvolution()
{
    for(unsigned int i=0;i<rootFilterSize.size();i++)
        for(unsigned int j=0;j<norm5.size();j++)
        {
            cout<<"Convolution with SVM level "+to_string(j+1)+"..."<<endl;
            const clock_t begin_time = clock();
            rootFilterAtLevel(i,j,1);
            cout << "Time:"<<float( clock () - begin_time ) /  CLOCKS_PER_SEC<<" s."<<endl;
            cout<<"Status: Success!"<<endl;
        }
}
//
////

//Rectangle
//
void DeepPyramid::calculateImagePyramidRectangle()
{
    for(unsigned int i=0;i<allObjects.size();i++)
    {
        allObjects[i].pyramidImageBox=getRectByNorm5Rect(allObjects[i].norm5Box);
    }
}

void DeepPyramid::calculateOriginalRectangle()
{
    calculateImagePyramidRectangle();
    float* scales=new float[num_levels];
    double longSide=(originalImg.cols>originalImg.rows) ? originalImg.cols : originalImg.rows;
    for(int i=0;i<num_levels;i++)
    {
        scales[i]=longSide/(sideNetInputSquare/pow(2, (num_levels-1-i)/2.0));
    }
    for(unsigned int i=0;i<allObjects.size();i++)
    {
        Rect original;
        Rect pyramid=allObjects[i].pyramidImageBox;
        int level=allObjects[i].level;
        original.x=pyramid.x*scales[level];
        original.y=pyramid.y*scales[level];
        original.width=pyramid.width*scales[level];
        original.height=pyramid.height*scales[level];
        allObjects[i].originalImageBox=original;
    }
}
double IOU(Rect r1,Rect r2)
{
    Rect runion= r1 & r2;

    return (double)runion.area()/(r1.area()+r2.area()-runion.area());
}

void DeepPyramid::groupOriginalRectangle()
{
    detectedObjects=nms_avg(allObjects,0.2,0.7);
}

void DeepPyramid::drawObjects()
{
    originalImg.copyTo(originalImgWithObjects);
    for(unsigned int i=0;i<detectedObjects.size();i++)
    {
        rectangle(originalImgWithObjects, detectedObjects[i].originalImageBox, config.objectRectangleColor);
    }
}

vector<ObjectBox> DeepPyramid::detect(Mat img)
{
    clear();
    setImg(img);
    createImagePyramid();
    createMax5Pyramid();
    createNorm5Pyramid();
    cout<<"filter"<<endl;
    rootFilterConvolution();
    cout<<"group rectangle"<<endl;
    calculateOriginalRectangle();
    groupOriginalRectangle();
    cout<<"boundbox regressor: TODO"<<endl;
    cout<<"Object count:"<<detectedObjects.size()<<endl;
    cout<<"draw"<<endl;
    drawObjects();
    return detectedObjects;
}

void DeepPyramid::clear()
{
    imagePyramid.clear();
    max5.clear();
    norm5.clear();
    detectedObjects.clear();
    meanValue.clear();
    deviationValue.clear();
    allObjects.clear();
    detectedObjects.clear();
}

void DeepPyramid::addRootFilter(Size filterSize, CvSVM* classifier)
{
    rootFilterSize.push_back(filterSize);
    rootFilterSVM.push_back(classifier);
}

Mat DeepPyramid::getImageWithObjects()
{
    return originalImgWithObjects;
}

void DeepPyramid::calculateToNorm5(Mat image)
{
    clear();
    setImg(image);
    createImagePyramid();
    createMax5Pyramid();
    createNorm5Pyramid();
}
int DeepPyramid::getNumLevel()
{
    return num_levels;
}
cv::Size DeepPyramid::originalImageSize()
{
    return Size(originalImg.cols,originalImg.rows);
}

int DeepPyramid::norm5SideLength()
{
    Blob<float>* output_layer = net_->output_blobs()[0];
    int length=output_layer->width();
    return length;
}

int DeepPyramid::chooseLevel(Size filterSize, Rect boundBox)
{
    vector<double> f;
    for(int i=0;i<num_levels;i++)
    {
        Rect r=getNorm5RectByOriginal(boundingBoxAtLevel(i,boundBox));

        f.push_back(abs(filterSize.width-r.width)+abs(r.height-filterSize.height));
    }
    int bestLevel=distance(f.begin(), min_element(f.begin(), f.end()));

    return bestLevel;
}

Mat DeepPyramid::getNorm5(int level, int channel)
{
    return norm5[level][channel];
}
void DeepPyramid::clearFilter()
{
    for(unsigned int i=0;i<rootFilterSVM.size();i++)
    {
        delete rootFilterSVM[i];
    }
    rootFilterSize.clear();
    rootFilterSVM.clear();
}

DeepPyramid::DeepPyramid(std::string detector_config, DeepPyramidMode mode) : config(detector_config, mode)
{
#ifdef CPU_ONLY
    Caffe::set_mode(Caffe::CPU);
#else
    Caffe::set_mode(Caffe::GPU);
#endif

    net_.reset(new Net<float>(config.model_file, caffe::TEST));
    net_->CopyTrainedLayersFrom(config.trained_net_file);
    Blob<float>* input_layer = net_->input_blobs()[0];
    num_channels = input_layer->channels();
    assert(input_layer->width()==input_layer->height());
    sideNetInputSquare=input_layer->width();

    num_levels=config.numLevels;

    centerConformity=16;
    boxSideConformity=81;

    if(mode==DeepPyramidMode::DETECT || mode==DeepPyramidMode::TEST)
    {
        CvSVM* classifier=new CvSVM();
        classifier->load(config.svm_trained_file.c_str());
        addRootFilter(config.filterSize,classifier);
    }
}


DeepPyramidConfiguration::DeepPyramidConfiguration(string deep_pyramid_config, DeepPyramidMode _mode)
{
    mode=_mode;

    FileStorage config(deep_pyramid_config, FileStorage::READ);

    config["NeuralNetwork-configuration"]>>model_file;
    config["NeuralNetwork-trained-model"]>>trained_net_file;
    config["NumberOfLevel"]>>numLevels;

    if(mode==DeepPyramidMode::DETECT || mode==DeepPyramidMode::TEST)
    {
        config["ObjectColor"]>>objectRectangleColor;

        config["SVM"]>>svm_trained_file;
        config["Filter-size"]>>filterSize;
    }

    config.release();
}
