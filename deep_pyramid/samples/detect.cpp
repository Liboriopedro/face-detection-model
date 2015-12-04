#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <caffe/caffe.hpp>
#include <caffe/common.hpp>

#include "deep_pyramid.h"

using namespace std;
using namespace cv;
using namespace caffe;

int main(int argc, char *argv[])
{
    // FIX: проверка аргументов командной строки на корректность
    // правильный пример обработки аргументов
    // https://github.com/ITLab-Vision/ITLab-Vision-deeplab/blob/movie-maker/extra/apps/movie_maker/samples/video_maker.cpp
    string config_file=argv[1];
    DeepPyramid pyramid(config_file, DeepPyramidMode::DETECT);
    
    // FIX: вот этот блок кода вообще к чему?
    // {
    CvSVMParams params;
    params.svm_type    = CvSVM::C_SVC;
    params.kernel_type = CvSVM::POLY;
    params.degree=3;
    params.term_crit   = cvTermCriteria(CV_TERMCRIT_ITER, 500, 1e-6);    
    FileStorage file("svmoar.xml", FileStorage::WRITE);
    file<<"Scalar"<<Scalar(0, 255, 0);
    file.release();
    // }
    Mat image;
    string image_file=argv[2];
    image=imread(image_file);

    pyramid.detect(image);

    // FIX: название выходного файла необходимо вынести в аргументы
    imwrite(image_file+"_res.jpg", pyramid.getImageWithObjects());

    return 0;
}
