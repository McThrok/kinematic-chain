#pragma once
#include <d3d11.h>
#include <vector>
#include <math.h> 
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <algorithm>

#include "Graphics/Vertex.h"

namespace sm = DirectX::SimpleMath;
namespace dx = DirectX;

class Arm
{
public:
	float length;
	float startAngle;
	sm::Vector2 startPosition;
};

class Obsticle
{
public:
	sm::Vector2 position;
	sm::Vector2 size;
	sm::Vector3 color;
};

class Simulation
{
public:
	std::vector<Obsticle> obsitcles;

	Arm arm1;
	Arm arm2;

	void Init();


};

