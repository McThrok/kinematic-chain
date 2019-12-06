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
#include "Robot.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class Obstacle
{
public:
	Vector2 position;
	Vector2 size;
	Vector4 color;
	Matrix GetWorldMatrix() { return Matrix::CreateScale(size.x, 1, size.y) * Matrix::CreateTranslation(position.x, 0, position.y); }
};

class ObstacleCollection
{
public:
	int selectedIdx;

	void Init();

	mt19937 gen{ 0 };
	vector<Obstacle> obstacles;
	void AddObstacle(Vector2 p1, Vector2 p2);
	Vector4 GetRandomColor();
	float GetRandomFloat(float min, float max);

	bool CheckSegment(Vector2 v1, Vector2 v2, Vector4& color);
	bool SegmentIntersect(Vector2 p1, Vector2 q1, Vector2 p2, Vector2 q2);
	bool OnSegment(Vector2 p, Vector2 q, Vector2 r);
	int CheckOrientation(Vector2 p, Vector2 q, Vector2 r);

	void Select(int x, int y);
	void DeleteSelected();
};

