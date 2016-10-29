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
Mesh::Mesh() : list_vertex_(0) 
{
  id_ = 0;
  num_vertexs_ = 0;
  num_triangles_ = 0;
  memset(&_desc, 0, sizeof(_desc));

  materialDiffuse[0] = 0.33;
  materialDiffuse[1] = 0.83;
  materialDiffuse[2] = 0.01;
}

/** The default destructor of the ObjectMesh Class */
Mesh::~Mesh()
{
  // TODO Auto-generated destructor stub
}


const cv::Point3f& Mesh::getVertex(int pos) const
{
	return list_vertex_[pos];
}

const float * Mesh::Diffuse() const
{
	return materialDiffuse;
}

/** Load a CSV with *.ply format **/
void Mesh::load(const std::string path, bool repairNormals)
{
  // Create the reader
  CsvReader csvReader(path);

  // Clear previous data
  list_vertex_.clear();

  // Read from .ply file
  std::vector<PolyIndices> shapes;
  csvReader.readPLY(list_vertex_, shapes);

  // Update mesh attributes
  num_vertexs_ = (int)list_vertex_.size();
  num_triangles_ = (int)shapes.size();

  for (int i = 0; i < shapes.size(); i++)
  {
	  PolyIndices &poly = shapes[i];
	  AddTriangle(poly[0], poly[1], poly[2]);
  }
}

int Mesh::NIndices()const
{
	return _indices.size();
}

int Mesh::NVertices() const
{
	return list_vertex_.size();
}

const cv::Point3f * Mesh::Vertices()
{
	return list_vertex_.data();
}

const int * Mesh::Indices() const
{
	return _indices.data();
}

void Mesh::AddTriangle(int a, int b, int c)
{
	// lets hope this works
	_indices.push_back(a);
	_indices.push_back(b);
	_indices.push_back(c);
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
	_normals.clear();
	num_triangles_ = 0;
	num_vertexs_ = 0;
}

int Mesh::Triangles()
{
	return _indices.size() / 3;
}

const cv::Point3f& Mesh::getTriangleVertex(int i, int j) const
{
	int index = _indices[i * 3 + j];
	return getVertex(index);
}

cv::Point3f Mesh::GetNormal(int i) const
{
	const cv::Point3f& first = getTriangleVertex(i, 0);
	const cv::Point3f& second = getTriangleVertex(i, 1);
	const cv::Point3f& third = getTriangleVertex(i, 2);
	cv::Point3f dir1 = second - first;
	cv::Point3f dir2 = third - first;

	cv::Point3f ret = dir1.cross(dir2);
	return ret;
}
