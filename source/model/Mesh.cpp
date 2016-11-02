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
Mesh::Mesh() : _vertices(0) 
{
  id_ = 0;
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
	return _vertices[pos];
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
  _vertices.clear();
  _normals.clear();

  // Read from .ply file
  std::vector<PolyIndices> shapes;
  cv::Point3f centerOffset;
  csvReader.readPLY(_vertices, shapes, centerOffset);
  for (int i = 0; i < _vertices.size(); i++)
  {
	  _normals.push_back(cv::Point3f(0, 0, 0));
  }
  // Update mesh attributes

  for (int i = 0; i < shapes.size(); i++)
  {
	  PolyIndices &poly = shapes[i];
	  AddTriangle(poly[0], poly[1], poly[2]);
	  if (poly.size() == 4)
	  {
		  AddTriangle(poly[0], poly[2], poly[3]);
	  }
  }
  for (int i = 0; i < _normals.size(); i++)
  {
	  _normals[i] /= _normals[i].dot(_normals[i]);
  }
  //cv::Point3f leftbottom, rightup;
  //GetBB(leftbottom, rightup);
  //cv::Point3f cent = (leftbottom + rightup) / 2;
  //cent.z = 0;
  //for (int i = 0; i < list_vertex_.size(); i++)
  //{
	 // list_vertex_[i] -= leftbottom; // leftbotom will now be at 0,0 at some point
	 // list_vertex_[i] += cent;
  //}
}

int Mesh::NIndices()const
{
	return _indices.size();
}

int Mesh::NVertices() const
{
	return _vertices.size();
}

const cv::Point3f * Mesh::Vertices()
{
	return _vertices.data();
}

const int * Mesh::Indices() const
{
	return _indices.data();
}

void Mesh::AddTriangle(int a, int b, int c)
{
	int triangles = Triangles();
	// lets hope this works
	_indices.push_back(a);
	_indices.push_back(b);
	_indices.push_back(c);
	cv::Point3f normal = GetNormal(triangles);
	_normals[a] += normal;
	_normals[b] += normal;
	_normals[c] += normal;
}

void Mesh::GetBB(cv::Point3f & bottomLeft, cv::Point3f& upRight)
{
	bottomLeft = _vertices[0];
	upRight = _vertices[0];
	for (int i = 1; i < _vertices.size(); i++)
	{
		///X
		if (bottomLeft.x > _vertices[i].x)
			bottomLeft.x = _vertices[i].x;
		else if (upRight.x < _vertices[i].x)
			upRight.x = _vertices[i].x;

		///y
		if (bottomLeft.y > _vertices[i].y)
			bottomLeft.y = _vertices[i].y;
		else if (upRight.y < _vertices[i].y)
			upRight.y = _vertices[i].y;

		///z
		if (bottomLeft.z > _vertices[i].z)
			bottomLeft.z = _vertices[i].z;
		else if (upRight.z < _vertices[i].z)
			upRight.z = _vertices[i].z;

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
	_vertices.push_back(p);
	_normals.push_back(cv::Point3f(0, 0, 0));
}

void Mesh::Clear()
{
	_vertices.clear();
	_normals.clear();
	_indices.clear();
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

void Mesh::GetVertexNormal(std::vector<float>& vertexNormal)
{
	for (int i = 0; i < _vertices.size(); i++)
	{
		cv::Point3f& point = _vertices[i];
		vertexNormal.push_back(point.x);
		vertexNormal.push_back(point.y);
		vertexNormal.push_back(point.z);
		cv::Point3f& normal = _normals[i];
		vertexNormal.push_back(normal.x);
		vertexNormal.push_back(normal.y);
		vertexNormal.push_back(normal.z);
	}
}
