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

typedef void(*SerializePoint)(FILE * file, cv::Point3f&point);
typedef void(*SerializeArrayPoint)(FILE * file, std::vector<Vertex> &point);
typedef void(*SerializeArraySize)(FILE * file, std::vector<int> &point);
typedef void(*SerializeDesc)(FILE * file, PositionDesc& desc);

struct SerializeCallback
{
	SerializePoint serializePoint;
	SerializeArrayPoint serializeArrayPoint;
	SerializeArraySize serializeArrayInt;
	SerializeDesc serializeDesc;
};

class Mesh
{
	PositionDesc _desc;
	float _materialDiffuse[3];

	void LoadCastle(const char * name);

public:
	 
	Mesh();
	~Mesh();

	void Save(const char * name);
	void Serialize(FILE * file, SerializeCallback context);
	const cv::Point3f& getVertex(int pos) const;
	int getNumVertices() const { return _vertices.size(); }
	const float * Diffuse()const;
	void Load(const std::string path_file, bool repairNormals = true);
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

	void GetIndices(std::vector<int>& indices);
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
