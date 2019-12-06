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
	Matrix GetWorldMatrix(bool start) { return Matrix::CreateScale(length, 1, 1) * Matrix::CreateRotationY(-XMConvertToRadians(GetAngle(start))); }
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
	unique_ptr<Vector4[]> parametrizationTable;
	unique_ptr<int[]> ffTable;
	int N;
	Vector4 color;

	int selectedIdx;
	bool useAltStart;
	bool useAltEnd;
	Arm arm1;
	Arm arm2;

	void Init();
	void Update();

	bool SetPosition(Vector2 position, bool start);
	bool CheckSegment(Vector2 v1, Vector2 v2, Vector4& color);
	bool SegmentIntersect(Vector2 p1, Vector2 q1, Vector2 p2, Vector2 q2);
	bool OnSegment(Vector2 p, Vector2 q, Vector2 r);
	int CheckOrientation(Vector2 p, Vector2 q, Vector2 r);

	void Select(int x, int y);
	void DeleteSelected();

	bool FindPath(vector<int>& angle1, vector<int>& angle2);
	void ClearFloodTable();
	void RunFlood(int aStart, int bStart, int aEnd, int bEnd);
	void FloodStep(int a, int b, int val, queue<int>& qA, queue<int>& qB);
	bool RetrievePath(int aEnd, int bEnd, vector<int>& angle1, vector<int>& angle2);
	bool RetrievePathStep(int a, int b, int val, vector<int>& angle1, vector<int>& angle2);

	mt19937 gen{ 0 };
	vector<Obsticle> obsitcles;
	void AddObsticle(Vector2 p1, Vector2 p2);
	Vector4 GetRandomColor();
	float GetRandomFloat(float min, float max);

	int NormalizeAngle(int angle);
};

