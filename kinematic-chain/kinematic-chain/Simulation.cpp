#include "Simulation.h"


void Simulation::Init()
{
	robot.Init();
	Obstacles.Init();

	paused = true;
	animationTime = 2;

	N = 360;
	color = Vector4(0, 0, 0, 1);

	parametrizationTable = make_unique<Vector4[]>(N * N);
	ffTable = make_unique<int[]>(N * N);

	parametrizationTableTmp = make_unique<Vector4[]>(N * N);
	ffTableTmp = make_unique<int[]>(N * N);

	UpdateParametrization();
}

void Simulation::UpdateParametrization()
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			int idx = CheckPosition(i, j);
			Vector4 color = idx == -1 ? this->color : Obstacles.obstacles[idx].color;

			parametrizationTable.get()[i * N + j] = color;
			ffTable.get()[i * N + j] = color == this->color ? 1000000 : -1;
		}

	}
}

int Simulation::CheckPosition(float angle1, float angle2)
{
	Vector2 v1(0, 0);

	Matrix t1 = Matrix::CreateTranslation(Vector3(robot.arm1.length, 0, 0));
	Matrix r1 = Matrix::CreateRotationY(XMConvertToRadians(-angle1));

	Vector3 w2 = XMVector3TransformCoord(Vector3(robot.arm1.length, 0, 0), r1);
	Vector2 v2(w2.x, w2.z);

	Matrix r2 = Matrix::CreateRotationY(XMConvertToRadians(-angle2));
	Matrix m = r2 * t1 * r1;

	Vector3 w3 = XMVector3TransformCoord(Vector3(robot.arm2.length, 0, 0), m);
	Vector2 v3(w3.x, w3.z);

	int result = Obstacles.CheckSegment(v1, v2);
	if (result == -1)
		result = Obstacles.CheckSegment(v2, v3);

	return result;
}

void Simulation::ClearFloodTable()
{
	int* ff = ffTable.get();
	int* fft = ffTableTmp.get();

	for (int i = 0; i < N * N; i++)
		fft[i] = ff[i];
}
void Simulation::RunFlood(int aStart, int bStart, int aEnd, int bEnd)
{
	int* fft = ffTableTmp.get();

	queue<int> qA;
	queue<int> qB;

	qA.push(aStart);
	qB.push(bStart);
	fft[aStart * N + bStart] = 0;

	while (!qA.empty())
	{
		int a = qA.front(); qA.pop();
		int b = qB.front(); qB.pop();
		if (a == aEnd && b == bEnd)
			break;

		int val = fft[a * N + b];

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
	int* fft = ffTableTmp.get();
	int idx = a * N + b;

	if (fft[idx] != -1 && fft[idx] > val + 1)
	{
		fft[idx] = val + 1;
		qA.push(a);
		qB.push(b);
	}

}
bool Simulation::RetrievePathStep(int a, int b, int val, vector<pair<int, int>>& angle)
{
	a = NormalizeAngle(a);
	b = NormalizeAngle(b);
	int* fft = ffTableTmp.get();
	int idx = a * N + b;

	if (fft[idx] != -1 && fft[idx] < val)
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

	int* fft = ffTableTmp.get();

	while (true)
	{
		int val = fft[a * N + b];

		if (val == 0)
			break;

		if (RetrievePathStep(a + 1, b, val, robot.angle)) a += 1;
		else if (RetrievePathStep(a - 1, b, val, robot.angle)) a -= 1;
		else if (RetrievePathStep(a, b + 1, val, robot.angle)) b += 1;
		else if (RetrievePathStep(a, b - 1, val, robot.angle)) b -= 1;
		else { robot.angle.clear(); return false; }

		a = NormalizeAngle(a);
		b = NormalizeAngle(b);
	}

	reverse(robot.angle.begin(), robot.angle.end());

	return true;

}
void Simulation::FindPath()
{
	ClearFloodTable();

	if (!robot.GetProperAngles(true) || !robot.GetProperAngles(true))
	{
		robot.angle.clear();
	}
	else
	{
		int aStart = NormalizeAngle(static_cast<int>(robot.arm1.GetAngle(true)));
		int bStart = NormalizeAngle(static_cast<int>(robot.arm2.GetAngle(true)));

		int aEnd = NormalizeAngle(static_cast<int>(robot.arm1.GetAngle(false)));
		int bEnd = NormalizeAngle(static_cast<int>(robot.arm2.GetAngle(false)));

		RunFlood(aStart, bStart, aEnd, bEnd);

		RetrievePath(aEnd, bEnd);
	}

	DrawFlood();
}

void Simulation::DrawFlood()
{
	int* fft = ffTableTmp.get();

	Vector4* pt = parametrizationTable.get();
	Vector4* ptt = parametrizationTableTmp.get();

	int max = 0;
	for (int i = 0; i < N * N; i++)
		if (fft[i] < 1000000 && fft[i] >= 0 && fft[i] > max)
			max = fft[i];

	for (int i = 0; i < N * N; i++)
	{
		int val = fft[i];
		if (val == -1)
			ptt[i] = pt[i];
		else if (val == 1000000)
			ptt[i] = Vector4(0, 0, 0, 1);
		else
			ptt[i] = Vector4(1, 1, 1, 1) * (1.0 - (1.0 * val / max));
	}

	for (int i = 0; i < robot.angle.size(); i++)
		ptt[robot.angle[i].first * 360 + robot.angle[i].second] = Vector4(1, 0, 0, 1);
}

void Simulation::Animate()
{
	time = 0;
	paused = false;
}
void Simulation::UpdateAnimation(float dt)
{
	if (time > animationTime)
		paused = true;

	if (paused)
		return;

	time += dt / 1000;
}

int Simulation::NormalizeAngle(int angle)
{
	while (angle < 0) angle += 360;
	while (angle >= 360) angle -= 360;

	return angle;
}

void Simulation::SetPosition(Vector2 position, bool start)
{
	robot.SetPosition(position, start);

	if (start)
	{
		robot.properAnglesStart &= CheckPosition(robot.arm1.startAngle, robot.arm2.startAngle);
		robot.properAnglesStartAlt &= CheckPosition(robot.arm1.startAngleAlt, robot.arm2.startAngleAlt);
	}
	else
	{
		robot.properAnglesEnd &= CheckPosition(robot.arm1.endAngle, robot.arm2.endAngle);
		robot.properAnglesEndAlt &= CheckPosition(robot.arm1.endAngleAlt, robot.arm2.endAngleAlt);
	}
}