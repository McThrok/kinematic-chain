#pragma once
#include <d3d11.h>
#include <vector>
#include <math.h> 
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <algorithm>
#include <random>

#include "Graphics/Vertex.h"

using namespace std;;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class Arm
{
public:
	float length;
	float angle;
	Matrix GetWorldMatrix() { return Matrix::CreateScale(length, 1, 1) * Matrix::CreateRotationY(-XMConvertToRadians(angle)); }
};

class Obsticle
{
public:
	Vector2 position;
	Vector2 size;
	Vector4 color;
	Matrix GetWorldMatrix() { return Matrix::CreateScale(size.x, 1, size.y) * Matrix::CreateTranslation(position.x, 0, position.y); }
};

class Simulation
{
public:
	mt19937 gen{ 0 };
	vector<Obsticle> obsitcles;
	void AddObsticle(Vector2 p1, Vector2 p2);
	Vector4 GetRandomColor();
	float GetRandomFloat(float min, float max);

	bool editMode;
	Arm arm1;
	Arm arm2;

	void Init();


};

