#ifndef __NMS_H__
#define __NMS_H__

#include "deep_pyramid.h"

// FIX: обернуть в отдельный класс
// результат можно сразу записать в исходное множество (см. ниже)
// void nms_max(std::vector<ObjectBox> &objects, double threshold);
// или возвращать через аргументы
// void nms_max(const std::vector<ObjectBox> &objects, double threshold, 
//              std::vector<ObjectBox> &detectedObjects);
void nms_max(std::vector<ObjectBox> &objects, double threshold);
void nms_avg(std::vector<ObjectBox> &objects, double box_threshold, double confidence_threshold);

#endif
