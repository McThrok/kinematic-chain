#pragma once
#include <d3d11.h>
#include <vector>
#include <math.h> 
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <algorithm>
#include <random>
#include <queue>
#include <memory>

#include "Graphics/Vertex.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;


class RobotState
{
public:
	bool isEmpty;
	float armAngle1;
	float armAngle2;
	Matrix armMatrix1;
	Matrix armMatrix2;
};

class Arm
{
public:
	float length;
	float startAngle;
	float startAngleAlt;
	float endAngle;
	float endAngleAlt;
	bool* useAltStart;
	bool* useAltEnd;

	float GetAngle(bool start) { return start ? *useAltStart ? startAngleAlt : startAngle : *useAltEnd ? endAngleAlt : endAngle; }
};

class Robot
{
public:
	Arm arm1;
	Arm arm2;

	vector<pair<int, int>>angle;

	bool useAltStart;
	bool useAltEnd;

	void Init();
	RobotState GetState(float animationProgress);
	float InterpolateAngle(float animationProgress, bool firstArm);
	float NormalizeAngle(float angle);
	bool SetPosition(Vector2 position, bool start);

};

