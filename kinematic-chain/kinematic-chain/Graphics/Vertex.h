#pragma once
#include <DirectXMath.h>

struct VertexPT
{
	VertexPT() {}
	VertexPT(float x, float y, float z, float u, float v)
		: pos(x, y, z), texCoord(u, v) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 texCoord;
};

struct VertexPN
{
	VertexPN() {}

	VertexPN(DirectX::XMFLOAT3 _pos, DirectX::XMFLOAT3 _normal)
		: pos(_pos), normal(_normal) {}

	VertexPN(float x, float y, float z, float nx, float ny, float nz)
		: pos(x, y, z), normal(nx, ny, nz) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
};

struct VertexP
{
	VertexP() {}

	VertexP(DirectX::XMFLOAT3 _pos) : pos(_pos) {}

	VertexP(float x, float y, float z) : pos(x, y, z) {}

	DirectX::XMFLOAT3 pos;
};