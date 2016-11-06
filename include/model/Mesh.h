/*
 * Mesh.h
 *
 *  Created on: Apr 9, 2014
 *      Author: edgar
 * Modified by caronnee
 */

#ifndef MESH_H_
#define MESH_H_

#include <iostream>
#include <opencv2/core/core.hpp>
#include "CsvReader.h"
#include "typedefs.h" 

 // --------------------------------------------------- //
 //                 TRIANGLE CLASS                      //
 // --------------------------------------------------- //

struct PositionDesc
{
	float _zPos, _xPos, _yPos, _elevation, _azimuth;
};
// --------------------------------------------------- //
//                OBJECT MESH CLASS                    //
// --------------------------------------------------- //

class Mesh
{
	PositionDesc _desc;
	float materialDiffuse[3];
public:
	 
	Mesh();
	virtual ~Mesh();

	const cv::Point3f& getVertex(int pos) const;
	int getNumVertices() const { return _vertices.size(); }
	const float * Diffuse()const;
	void load(const std::string path_file, bool repairNormals = true);
	int NIndices()const;
	int NVertices()const;
	const int * Indices() const;
	void AddTriangle(int a, int b, int c);
	void GetBB(cv::Point3f & leftBottom, cv::Point3f& rightUpper);

	// adds vertex to a mesh
	void AddVertex(float x, float y, float z);

	// empty
	void Clear();
	int Triangles();
	const cv::Point3f& getTriangleVertex(int i, int j) const;

private:
	/** The identification number of the mesh */
	int id_;

	/* The list of triangles of the mesh */
	std::vector<Vertex> _vertices;

	/* The list of normals */
	std::vector<int> _indices;

public:
	cv::Point3f GetNormal(int a, int b, int c) const;
	void GetVertexNormal(std::vector<float>& vertexNormal);
};

#endif /* OBJECTMESH_H_ */
