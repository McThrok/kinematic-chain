#include "Simulation.h"


void Simulation::Init()
{
	robot.Init();
	Obstacles.Init();

	float paused = false;
	float animationTime = 2;

	N = 360;
	color = Vector4(1, 1, 1, 1);

	parametrizationTable = make_unique<Vector4[]>(N * N);
	ffTable = make_unique<int[]>(N * N);
}
void Simulation::UpdateParametrization()
{
	Vector2 v1(0, 0);

	for (int i = 0; i < N; i++)
	{
		Matrix t1 = Matrix::CreateTranslation(Vector3(robot.arm1.length, 0, 0));
		Matrix r1 = Matrix::CreateRotationY(XMConvertToRadians(-i));

		Vector3 w2 = XMVector3TransformCoord(Vector3(robot.arm1.length, 0, 0), r1);
		Vector2 v2(w2.x, w2.z);

		for (int j = 0; j < N; j++)
		{
			Matrix r2 = Matrix::CreateRotationY(XMConvertToRadians(-j));
			Matrix m = r2 * t1 * r1;

			Vector3 w3 = XMVector3TransformCoord(Vector3(robot.arm2.length, 0, 0), m);
			Vector2 v3(w3.x, w3.z);


			Vector4 color = this->color;
			if (Obstacles.CheckSegment(v1, v2, color))
				Obstacles.CheckSegment(v2, v3, color);

			parametrizationTable.get()[i * N + j] = color;
		}
	}
}

void Simulation::ClearFloodTable()
{
	int* ff = ffTable.get();

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			int idx = i * N + j;
			if (parametrizationTable.get()[idx] != this->color)
				ff[idx] = -1;//blocked
			else
				ff[idx] = 1000000;
		}
}
void Simulation::RunFlood(int aStart, int bStart, int aEnd, int bEnd)
{
	//aStart = 10;
	//bStart = 10;
	//aEnd = 12;
	//bEnd = 10;
	int* ff = ffTable.get();

	queue<int> qA;
	queue<int> qB;

	qA.push(aStart);
	qB.push(bStart);
	ff[aStart * N + bStart] = 0;

	while (!qA.empty())
	{
		int a = qA.front(); qA.pop();
		int b = qB.front(); qB.pop();
		if (a == aEnd && b == bEnd)
			break;

		int val = ff[a * N + b];

		FloodStep(a + 1, b, val, qA, qB);
		FloodStep(a - 1, b, val, qA, qB);
		FloodStep(a, b + 1, val, qA, qB);
		FloodStep(a, b - 1, val, qA, qB);
	}

}
void Simulation::FloodStep(int a, int b, int val, queue<int>& qA, queue<int>& qB)
{
	a = NormalizeAngle(a);
	b = NormalizeAngle(b);
	int* ff = ffTable.get();
	int idx = a * N + b;

	if (ff[idx] != -1 && ff[idx] > val + 1)
	{
		ff[idx] = val + 1;
		qA.push(a);
		qB.push(b);
	}

}
bool Simulation::RetrievePathStep(int a, int b, int val, vector<pair<int, int>>& angle)
{
	a = NormalizeAngle(a);
	b = NormalizeAngle(b);
	int* ff = ffTable.get();
	int idx = a * N + b;

	if (ff[idx] != -1 && ff[idx] < val)
	{
		angle.push_back(make_pair(a, b));
		return true;
	}
	return false;
}
bool Simulation::RetrievePath(int aEnd, int bEnd)
{
	robot.angle.clear();

	int a = aEnd;
	int b = bEnd;
	robot.angle.push_back(make_pair(a, b));

	int* ff = ffTable.get();

	while (true)
	{
		int val = ff[a * N + b];

		if (val == 0)
			return true;

		if (RetrievePathStep(a + 1, b, val, robot.angle)) a += 1;
		else if (RetrievePathStep(a - 1, b, val, robot.angle)) a -= 1;
		else if (RetrievePathStep(a, b + 1, val, robot.angle)) b += 1;
		else if (RetrievePathStep(a, b - 1, val, robot.angle)) b -= 1;
		else return false;

		a = NormalizeAngle(a);
		b = NormalizeAngle(b);
	}

	return true;

}
bool Simulation::FindPath()
{
	UpdateParametrization();
	ClearFloodTable();

	int aStart = NormalizeAngle(static_cast<int>(robot.arm1.GetAngle(true)));
	int bStart = NormalizeAngle(static_cast<int>(robot.arm2.GetAngle(true)));

	int aEnd = NormalizeAngle(static_cast<int>(robot.arm1.GetAngle(false)));
	int bEnd = NormalizeAngle(static_cast<int>(robot.arm2.GetAngle(false)));

	RunFlood(aStart, bStart, aEnd, bEnd);

	return RetrievePath(aEnd, bEnd);
}

void Simulation::Animate()
{
	time = 0;
	paused = true;
}
void Simulation::UpdateAnimation(float dt)
{
	if (time > animationTime)
		paused = true;

	if (paused)
		return;

	time += dt;
}

int Simulation::NormalizeAngle(int angle)
{
	while (angle < 0) angle += 360;
	while (angle >= 360) angle -= 360;

	return angle;
}