#pragma once

#include <string>

#include <GL/gl.h>

struct ps2glMeshHeader
{
	int nStrips;
};

struct ps2glStripHeader
{
	int nVertices;
	int nNormals;
	int nUVs;
	int nRGBAs;
};

struct ps2glMeshVertex
{
	float x;
	float y;
	float z;
};

struct ps2glMeshNormal
{
	float nx;
	float ny;
	float nz;
};

struct ps2glMeshRGBA
{
	float r;
	float g;
	float b;
	float a;
};

struct ps2glMeshUV
{
	float u;
	float v;
};

class mesh
{
public:
	mesh();

	mesh(const std::string& path);

protected:
	GLint list;
};