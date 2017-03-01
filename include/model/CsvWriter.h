#ifndef CSVWRITER_H
#define	CSVWRITER_H

#include <iostream>
#include <fstream>
#include <opencv2/core/core.hpp>
//#include "Utils.h"

using namespace std;


class CsvWriter {
public:
  CsvWriter(const string &path, const string &separator = " ");
  ~CsvWriter();
  void writeXYZ(const vector<cv::Point3f> &list_points3d);
  void writeUVXYZ(const vector<cv::Point3f> &list_points3d, const vector<cv::Point2f> &list_points2d, const cv::Mat &descriptors);

private:
  ofstream _file;
  string _separator;
  bool _isFirstTerm;
};

#endif
