/*
 * Mesh.cpp
 *
 *  Created on: Apr 9, 2014
 *      Author: edgar
 */

#include "model/Mesh.h"
#include "model/CsvReader.h"
#include "typedefs.h"

// --------------------------------------------------- //
//                   TRIANGLE CLASS                    //
// --------------------------------------------------- //

/**  The custom constructor of the Triangle Class */
Triangle::Triangle(int id, cv::Point3f V0, cv::Point3f V1, cv::Point3f V2)
{
  id_ = id; v0_ = V0; v1_ = V1; v2_ = V2;
}

/**  The default destructor of the Class */
Triangle::~Triangle()
{
  // TODO Auto-generated destructor stub
}


// --------------------------------------------------- //
//                     RAY CLASS                       //
// --------------------------------------------------- //

/**  The custom constructor of the Ray Class */
Ray::Ray(cv::Point3f P0, cv::Point3f P1) {
  p0_ = P0; p1_ = P1;
}

/**  The default destructor of the Class */
Ray::~Ray()
{
  // TODO Auto-generated destructor stub
}


// --------------------------------------------------- //
//                 OBJECT MESH CLASS                   //
// --------------------------------------------------- //

/** The default constructor of the ObjectMesh Class */
Mesh::Mesh() : list_vertex_(0) , list_triangles_(0)
{
  id_ = 0;
  num_vertexs_ = 0;
  num_triangles_ = 0;
}

/** The default destructor of the ObjectMesh Class */
Mesh::~Mesh()
{
  // TODO Auto-generated destructor stub
}


/** Load a CSV with *.ply format **/
void Mesh::load(const std::string path)
{

  // Create the reader
  CsvReader csvReader(path);

  // Clear previous data
  list_vertex_.clear();
  list_triangles_.clear();

  // Read from .ply file
  csvReader.readPLY(list_vertex_, list_triangles_);

  // Update mesh attributes
  num_vertexs_ = (int)list_vertex_.size();
  num_triangles_ = (int)list_triangles_.size();

}

void Mesh::AddTriangle(int a, int b, int c)
{
	PolyIndices poly;
	poly.push_back(a);
	poly.push_back(b);
	poly.push_back(c);
	list_triangles_.push_back(poly);
}

void Mesh::ConvertToBB()
{
	cv::Point3f bottomLeft = list_vertex_[0];
	cv::Point3f upRight = list_vertex_[0];
	for (int i = 1; i < num_vertexs_; i++)
	{
		///X
		if (bottomLeft.x > list_vertex_[i].x)
			bottomLeft.x = list_vertex_[i].x;
		else if (upRight.x < list_vertex_[i].x)
			upRight.x = list_vertex_[i].x;

		///y
		if (bottomLeft.y > list_vertex_[i].y)
			bottomLeft.y = list_vertex_[i].y;
		else if (upRight.y < list_vertex_[i].y)
			upRight.y = list_vertex_[i].y;

		///z
		if (bottomLeft.z > list_vertex_[i].z)
			bottomLeft.z = list_vertex_[i].z;
		else if (upRight.z < list_vertex_[i].z)
			upRight.z = list_vertex_[i].z;

	}
	// clear all
	Clear();
	AddVertex(bottomLeft.x, bottomLeft.y, bottomLeft.z);
	AddVertex(bottomLeft.x, bottomLeft.y, upRight.z);
	AddVertex(bottomLeft.x, upRight.y, bottomLeft.z);
	AddVertex(bottomLeft.x, upRight.y, upRight.z);
	AddVertex(upRight.x, bottomLeft.y, bottomLeft.z);
	AddVertex(upRight.x, bottomLeft.y, upRight.z);
	AddVertex(upRight.x, upRight.y, bottomLeft.z);
	AddVertex(upRight.x, upRight.y, upRight.z);

	//add faces
	AddTriangle(6,5,7);
	AddTriangle(6,2,5);
	AddTriangle(2,0,3);
	AddTriangle(2,3,5);

	AddTriangle(1,6,7);
	AddTriangle(1,4,7);
	AddTriangle(1,2,6);
	AddTriangle(0,1,2);

	AddTriangle(3,4,5);
	AddTriangle(3,5,7);
	AddTriangle(0,1,4);
	AddTriangle(0,4,3);
}

void Mesh::AddVertex(float x, float y, float z)
{
	cv::Point3f p(x, y, z);
	list_vertex_.push_back(p);
	num_vertexs_++;
}

void Mesh::Clear()
{
	list_vertex_.clear();
	list_triangles_.clear();
	num_triangles_ = 0;
	num_vertexs_ = 0;
}