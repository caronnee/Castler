#ifndef CSVREADER_H
#define	CSVREADER_H

#include <iostream>
#include <fstream>
#include <opencv2/core/core.hpp>
//#include "Utils.h"
#include<set>

using namespace std;


struct Vertex
{
	cv::Point3f _vertex;
	cv::Point3f _normal;
};

class CsvReader {
public:
  /**
  * The default constructor of the CSV reader Class.
  * The default separator is ' ' (empty space)
  *
  * @param path - The path of the file to read
  * @param separator - The separator character between words per line
  * @return
  */
  CsvReader(const string &path, const char &separator = ' ');

  /**
  * Read a plane text file with .ply format
  *
  * @param list_vertex - The container of the vertices list of the mesh
  * @param list_triangle - The container of the triangles list of the mesh
  * @return
  */
  void readPLY(vector<Vertex> &list_vertex, vector<vector<int> > &list_triangles, cv::Point3f& centerOffset);

private:
  /** The current stream file for the reader */
  ifstream _file;
  /** The separator character between words for each line */
  char _separator;
};

#endif
