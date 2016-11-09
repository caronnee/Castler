/*
 * Mesh.cpp
 *
 *  Created on: Apr 9, 2014
 *      Author: edgar
 */

#include "model/Mesh.h"
#include "model/CsvReader.h"
#include "typedefs.h"
#include "debug.h"
#include "Mics.h"

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
	return _vertices[pos]._vertex;
}

const float * Mesh::Diffuse() const
{
	return materialDiffuse;
}

/** Load a CSV with *.ply format **/
void Mesh::load(const std::string path, bool repairNormals)
{
	Clear();
	// Create the reader
	CsvReader csvReader(path);

	// Read from .ply file
	std::vector<PolyIndices> shapes;
	cv::Point3f centerOffset;
	csvReader.readPLY(_vertices, shapes, centerOffset);

	// Update mesh attributes
	for (int i = 0; i < shapes.size(); i++)
	{
		PolyIndices &poly = shapes[i];
		Point3f normal = GetNormal(poly[0], poly[1],poly[2]);
		for (int iPoly = 0; iPoly < poly.size(); iPoly++)
		{
			int &vertex = poly[iPoly];
			cv::Point3f & nrm = _vertices[vertex]._normal;
			if (nrm.dot(nrm) != 0)
			{
				int n = _vertices.size();
				_vertices.push_back(_vertices[vertex]);
				vertex = n;
			}
			_vertices[vertex]._normal = normal;

		}
		AddTriangle(poly[0], poly[1], poly[2]);
		if (poly.size() == 4)
		{
			AddTriangle(poly[0], poly[2], poly[3]);
		}
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

const int * Mesh::Indices() const
{
	return _indices.data();
}

void Mesh::AddTriangle(int a, int b, int c)
{
	int index = _indices.size() / 3;

	// lets hope this works
	_indices.push_back(a);
	_indices.push_back(b);
	_indices.push_back(c);
}

void Mesh::GetBB(cv::Point3f & bottomLeft, cv::Point3f& upRight)
{
	bottomLeft = _vertices[0]._vertex;
	upRight = _vertices[0]._vertex;
	for (int i = 1; i < _vertices.size(); i++)
	{
		///X
		if (bottomLeft.x > _vertices[i]._vertex.x)
			bottomLeft.x = _vertices[i]._vertex.x;
		else if (upRight.x < _vertices[i]._vertex.x)
			upRight.x = _vertices[i]._vertex.x;

		///y
		if (bottomLeft.y > _vertices[i]._vertex.y)
			bottomLeft.y = _vertices[i]._vertex.y;
		else if (upRight.y < _vertices[i]._vertex.y)
			upRight.y = _vertices[i]._vertex.y;

		///z
		if (bottomLeft.z > _vertices[i]._vertex.z)
			bottomLeft.z = _vertices[i]._vertex.z;
		else if (upRight.z < _vertices[i]._vertex.z)
			upRight.z = _vertices[i]._vertex.z;

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
	AddTriangle(6, 5, 7);
	AddTriangle(6, 2, 5);
	AddTriangle(2, 0, 3);
	AddTriangle(2, 3, 5);

	AddTriangle(1, 6, 7);
	AddTriangle(1, 4, 7);
	AddTriangle(1, 2, 6);
	AddTriangle(0, 1, 2);

	AddTriangle(3, 4, 5);
	AddTriangle(3, 5, 7);
	AddTriangle(0, 1, 4);
	AddTriangle(0, 4, 3);
}

void Mesh::AddVertex(float x, float y, float z)
{
	Vertex vertex;
	cv::Point3f p(x, y, z);
	vertex._vertex = p;
	_vertices.push_back(vertex);
}

void Mesh::Clear()
{
	_vertices.clear();
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

void Mesh::GetIndices(std::vector<int>& indices)
{
	indices.insert(indices.end(), _indices.begin(), _indices.end());
}

cv::Point3f Mesh::GetNormal(int a,int b, int c) const
{
	const cv::Point3f& first = getVertex(a);
	const cv::Point3f& second = getVertex(b);
	const cv::Point3f& third = getVertex(c);
	cv::Point3f dir1 = second - first;
	cv::Point3f dir2 = third - first;

	cv::Point3f ret = dir2.cross(dir1);
	return ret;
}

void Mesh::GetVertexNormal(std::vector<float>& vertexNormal)
{
	for (int i = 0; i < _vertices.size(); i++)
	{
		cv::Point3f& point = _vertices[i]._vertex;
		vertexNormal.push_back(point.x);
		vertexNormal.push_back(point.y);
		vertexNormal.push_back(point.z);
		cv::Point3f& normal = _vertices[i]._normal;
		vertexNormal.push_back(normal.x);
		vertexNormal.push_back(normal.y);
		vertexNormal.push_back(normal.z);
	}
}
