#include "model/CsvReader.h"

// Converts a given string to an integer
int StringToInt(const std::string &Text)
{
	std::istringstream ss(Text);
	int result;
	return ss >> result ? result : 0;
}

float StringToFloat(const std::string &Text)
{
	std::istringstream ss(Text);
	float result;
	return ss >> result ? result : 0.0f;
}

/** The default constructor of the CSV reader Class */
CsvReader::CsvReader(const string &path, const char &separator){
    _file.open(path.c_str(), ifstream::in);
    _separator = separator;
}

/* Read a plane text file with .ply format */
void CsvReader::readPLY(vector<Vertex> &list_vertex, vector<vector<int> > &list_triangles, Point3f& centerOffset)
{
    std::string line, tmp_str, n;
    int num_vertex = 0, num_triangles = 0;
    int count = 0;
    bool end_header = false;
    bool end_vertex = false;

    // Read the whole *.ply file
    while (getline(_file, line)) {
    stringstream liness(line);

    // read header
    if(!end_header)
    {
        getline(liness, tmp_str, _separator);
        if( tmp_str == "element" )
        {
            getline(liness, tmp_str, _separator);
            getline(liness, n);
            if(tmp_str == "vertex") num_vertex = StringToInt(n);
            if(tmp_str == "face") num_triangles = StringToInt(n);
        }
        if(tmp_str == "end_header") end_header = true;
    }

    // read file content
    else if(end_header)
    {
         // read vertex and add into 'list_vertex'
         if(!end_vertex && count < num_vertex)
         {
             string x, y, z;
             getline(liness, x, _separator);
             getline(liness, y, _separator);
             getline(liness, z);

             cv::Point3f tmp_p;
             tmp_p.x = (float)StringToFloat(x);
             tmp_p.y = (float)StringToFloat(y);
             tmp_p.z = (float)StringToFloat(z);
			 Vertex vert;
			 vert._vertex = tmp_p;
             list_vertex.push_back(vert);

             count++;
             if(count == num_vertex)
             {
                 count = 0;
                 end_vertex = !end_vertex;
             }
         }
         // read faces and add into 'list_triangles'
         else if(end_vertex  && count < num_triangles)
         {
             string num_pts_per_face, id0, id1, id2;
             getline(liness, num_pts_per_face, _separator);
			 int vertices = StringToInt(num_pts_per_face);
             getline(liness, id0, _separator);
             getline(liness, id1, _separator);
			 if (vertices == 3)
			 {
				 getline(liness, id2);
			 }
			 else
			 {
				 getline(liness, id2, _separator);
			 }

             std::vector<int> tmp_triangle(3);
             tmp_triangle[0] = StringToInt(id0);
             tmp_triangle[1] = StringToInt(id1);
			 tmp_triangle[2] = StringToInt(id2);
			 if (vertices == 4)
			 {
				 getline(liness, id2);
				 tmp_triangle.push_back(StringToInt(id2));
			 }
             list_triangles.push_back(tmp_triangle);

             count++;
      }
		 cv::Point3f leftBottom=list_vertex[0]._vertex, rightUp=list_vertex[0]._vertex;
		 for (int i = 1; i < list_vertex.size(); i++)
		 {
			 cv::Point3f& testPoint = list_vertex[i]._vertex;
			 if (leftBottom.x > testPoint.x)
				 leftBottom.x = testPoint.x;
			 if (leftBottom.y > testPoint.y)
				 leftBottom.y = testPoint.y;
			 if (leftBottom.z > testPoint.z)
				 leftBottom.z = testPoint.z;

			 if (rightUp.x > testPoint.x)
				 rightUp.x = testPoint.x;
			 if (rightUp.y > testPoint.y)
				 rightUp.y = testPoint.y;
			 if (rightUp.z > testPoint.z)
				 rightUp.z = testPoint.z;
		 }
		 centerOffset = (rightUp + leftBottom) / 2;
    }
  }
}
