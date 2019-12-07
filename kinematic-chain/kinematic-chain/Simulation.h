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
#include "ObstacleCollection.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class Simulation
{
public:
	unique_ptr<Vector4[]> parametrizationTable;
	unique_ptr<int[]> ffTable;

	unique_ptr<Vector4[]> parametrizationTableTmp;
	unique_ptr<int[]> ffTableTmp;
	int N;
	Vector4 color;

	Robot robot;
	ObstacleCollection Obstacles;

	void Init();
	void UpdateParametrization();

	float paused;
	float time;
	float animationTime;
	void Animate();
	void UpdateAnimation(float dt);

	void FindPath();
	void ClearFloodTable();
	void RunFlood(int aStart, int bStart, int aEnd, int bEnd);
	void FloodStep(int a, int b, int val, queue<int>& qA, queue<int>& qB);
	bool RetrievePath(int aEnd, int bEnd);
	bool RetrievePathStep(int a, int b, int val, vector<pair<int, int>>& angle);
	void DrawFlood();

	int NormalizeAngle(int angle);
};

