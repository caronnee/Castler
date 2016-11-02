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


 // --------------------------------------------------- //
 //                 TRIANGLE CLASS                      //
 // --------------------------------------------------- //

class Triangle {
public:

	explicit Triangle(int id, cv::Point3f V0, cv::Point3f V1, cv::Point3f V2);
	virtual ~Triangle();

	cv::Point3f getV0() const { return v0_; }
	cv::Point3f getV1() const { return v1_; }
	cv::Point3f getV2() const { return v2_; }

private:
	/** The identifier number of the triangle */
	int id_;
	/** The three vertices that defines the triangle */
	cv::Point3f v0_, v1_, v2_;
};


// --------------------------------------------------- //
//                     RAY CLASS                       //
// --------------------------------------------------- //

class Ray {
public:

	explicit Ray(cv::Point3f P0, cv::Point3f P1);
	virtual ~Ray();

	cv::Point3f getP0() { return p0_; }
	cv::Point3f getP1() { return p1_; }

private:
	/** The two points that defines the ray */
	cv::Point3f p0_, p1_;
};

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
	const cv::Point3f * Vertices();
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
	std::vector<cv::Point3f> _vertices;

	/* The list of normals */
	std::vector<cv::Point3f> _normals;

	/* The list of normals */
	std::vector<int> _indices;

public:
	cv::Point3f GetNormal(int i) const;
	void GetVertexNormal(std::vector<float>& vertexNormal);
};

#endif /* OBJECTMESH_H_ */
