#include "Simulation.h"

void Simulation::Init()
{
	useAltStart = false;
	useAltEnd = false;
	selectedIdx = -1;
	N = 360;
	color = Vector4(1, 1, 1, 1);

	arm1.length = 150;
	arm2.length = 100;
	arm1.useAltStart = &useAltStart;
	arm2.useAltStart = &useAltStart;
	arm1.useAltEnd = &useAltEnd;
	arm2.useAltEnd = &useAltEnd;

	SetPosition(Vector2(150, 100), true);
	SetPosition(Vector2(100, 150), false);

	parametrizationTable = make_unique<Vector4[]>(N * N);
	ffTable = make_unique<int[]>(N * N);
}

void Simulation::AddObsticle(Vector2 p1, Vector2 p2)
{
	if (p1.x * p2.x < 0 && p1.y * p2.y < 0)
		return;

	Obsticle o;
	o.position = Vector2(min(p1.x, p2.x), min(p1.y, p2.y));
	o.size = Vector2(abs(p1.x - p2.x), abs(p1.y - p2.y));
	o.color = GetRandomColor();

	if (o.size.x < 10 || o.size.y < 10)
		return;

	obsitcles.push_back(o);
}
void Simulation::DeleteSelected()
{
	if (selectedIdx == -1) return;

	obsitcles.erase(obsitcles.begin() + selectedIdx, obsitcles.begin() + selectedIdx + 1);
	selectedIdx = -1;
}
void Simulation::Select(int x, int y)
{
	for (int i = obsitcles.size() - 1; i >= 0; i--)
	{
		Obsticle& o = obsitcles[i];
		float x1 = o.position.x;
		float x2 = o.position.x + o.size.x;
		float y1 = o.position.y;
		float y2 = o.position.y + o.size.y;

		if (x1 <= x && x <= x2 && y1 <= y && y <= y2)
		{
			selectedIdx = i;
			return;
		}
	}

	selectedIdx = -1;
}
Vector4 Simulation::GetRandomColor()
{
	float r = GetRandomFloat(0, 1);
	float g = GetRandomFloat(0, 1);
	float b = GetRandomFloat(0, 1);
	return Vector4(r, g, b, 1);
}
float Simulation::GetRandomFloat(float min, float max)
{
	return  std::uniform_real_distribution<float>{min, max}(gen);
}
bool Simulation::SetPosition(Vector2 position, bool start)
{
	float l1 = arm1.length;
	float l2 = arm2.length;
	float px = position.x;
	float py = position.y;

	float tmp = -py * py * (l1 * l1 * l1 * l1 + (-l2 * l2 + px * px + py * py) * (-l2 * l2 + px * px + py * py) - 2 * l1 * l1 * (l2 * l2 + px * px + py * py));
	if (tmp < 0)
		return false;

	float x = l1 * l1 * px - l2 * l2 * px + px * px * px + px * py * py;
	float xDiv = 2 * (px * px + py * py);

	float y = l1 * l1 * py * py - l2 * l2 * py * py + px * px * py * py + py * py * py * py;
	float yDiv = 2 * py * (px * px + py * py);

	Vector2 v1((x - sqrt(tmp)) / xDiv, (y + px * sqrt(tmp)) / yDiv);
	Vector2 v2((x + sqrt(tmp)) / xDiv, (y - px * sqrt(tmp)) / yDiv);

	Vector2 w1 = position - v1;
	Vector2 w2 = position - v2;

	float* angle1;
	float* angle2;
	float* angleAlt1;
	float* angleAlt2;

	if (start)
	{
		angle1 = &arm1.startAngle;
		angle2 = &arm2.startAngle;
		angleAlt1 = &arm1.startAngleAlt;
		angleAlt2 = &arm2.startAngleAlt;
	}
	else
	{
		angle1 = &arm1.endAngle;
		angle2 = &arm2.endAngle;
		angleAlt1 = &arm1.endAngleAlt;
		angleAlt2 = &arm2.endAngleAlt;
	}

	*angle1 = XMConvertToDegrees(XMScalarACos(Vector2(1, 0).Dot(v1) / v1.Length()));
	*angle2 = XMConvertToDegrees(XMScalarACos(v1.Dot(w1) / v1.Length() / w1.Length()));
	if (((Vector3)XMVector2Cross(Vector2(1, 0), v1)).x < 0) *angle1 *= -1;
	if (((Vector3)XMVector2Cross(v1, w1)).x < 0) *angle2 *= -1;

	*angleAlt1 = XMConvertToDegrees(XMScalarACos(Vector2(1, 0).Dot(v2) / v2.Length()));
	*angleAlt2 = XMConvertToDegrees(XMScalarACos(v2.Dot(w2) / v2.Length() / w2.Length()));
	if (((Vector3)XMVector2Cross(Vector2(1, 0), v2)).x < 0) *angleAlt1 *= -1;
	if (((Vector3)XMVector2Cross(v2, w2)).x < 0) *angleAlt2 *= -1;
}

void Simulation::Update()
{
	Vector2 v1(0, 0);

	for (int i = 0; i < N; i++)
	{
		Matrix t1 = Matrix::CreateTranslation(Vector3(arm1.length, 0, 0));
		Matrix r1 = Matrix::CreateRotationY(XMConvertToRadians(-i));

		Vector3 w2 = XMVector3TransformCoord(Vector3(arm1.length, 0, 0), r1);
		Vector2 v2(w2.x, w2.z);

		for (int j = 0; j < N; j++)
		{
			Matrix r2 = Matrix::CreateRotationY(XMConvertToRadians(-j));
			Matrix m = r2 * t1 * r1;

			Vector3 w3 = XMVector3TransformCoord(Vector3(arm2.length, 0, 0), m);
			Vector2 v3(w3.x, w3.z);


			Vector4 color = this->color;
			if (CheckSegment(v1, v2, color))
				CheckSegment(v2, v3, color);

			parametrizationTable.get()[i * N + j] = color;
		}
	}
}

bool Simulation::CheckSegment(Vector2 v1, Vector2 v2, Vector4& color)
{
	for (auto& o : obsitcles)
	{
		Vector2 p = o.position;
		Vector2 s = o.size;

		bool intersect = false;

		if (SegmentIntersect(v1, v2, p, Vector2(p.x + s.x, p.y))
			|| SegmentIntersect(v1, v2, p, Vector2(p.x, p.y + s.y))
			|| SegmentIntersect(v1, v2, Vector2(p.x + s.x, p.y), p + s)
			|| SegmentIntersect(v1, v2, Vector2(p.x, p.y + s.y), p + s))
		{
			color = o.color;
			return false;
		}
	}

	return true;
}
bool Simulation::SegmentIntersect(Vector2 p1, Vector2 q1, Vector2 p2, Vector2 q2)
{
	int o1 = CheckOrientation(p1, q1, p2);
	int o2 = CheckOrientation(p1, q1, q2);
	int o3 = CheckOrientation(p2, q2, p1);
	int o4 = CheckOrientation(p2, q2, q1);

	if (o1 != o2 && o3 != o4) return true;

	if (o1 == 0 && OnSegment(p1, p2, q1)) return true;
	if (o2 == 0 && OnSegment(p1, q2, q1)) return true;
	if (o3 == 0 && OnSegment(p2, p1, q2)) return true;
	if (o4 == 0 && OnSegment(p2, q1, q2)) return true;

	return false;
}
bool Simulation::OnSegment(Vector2 p, Vector2 q, Vector2 r)
{
	return q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) && q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y);
}
int Simulation::CheckOrientation(Vector2 p, Vector2 q, Vector2 r)
{
	int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
	return (val == 0) ? 0 : (val > 0) ? 1 : 2;
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

	if (ff[idx] != -1 && ff[idx] > val+1)
	{
		ff[idx] = val + 1;
		qA.push(a);
		qB.push(b);
	}

}
bool Simulation::RetrievePathStep(int a, int b, int val, vector<int>& angle1, vector<int>& angle2)
{
	a = NormalizeAngle(a);
	b = NormalizeAngle(b);
	int* ff = ffTable.get();
	int idx = a * N + b;

	if (ff[idx] != -1 && ff[idx] < val)
	{
		angle1.push_back(a);
		angle2.push_back(b);
		return true;
	}
	return false;
}
bool Simulation::RetrievePath(int aEnd, int bEnd, vector<int>& angle1, vector<int>& angle2)
{
	angle1.clear();
	angle2.clear();

	int a = aEnd;
	int b = bEnd;
	angle1.push_back(a);
	angle2.push_back(b);

	int* ff = ffTable.get();

	while (true)
	{
		int val = ff[a * N + b];

		if (val == 0)
			return true;

		if (RetrievePathStep(a + 1, b, val, angle1, angle2)) a += 1;
		else if (RetrievePathStep(a - 1, b, val, angle1, angle2)) a -= 1;
		else if (RetrievePathStep(a, b + 1, val, angle1, angle2)) b += 1;
		else if (RetrievePathStep(a, b - 1, val, angle1, angle2)) b -= 1;
		else return false;

		a = NormalizeAngle(a);
		b = NormalizeAngle(b);
	}

	return true;

}
bool Simulation::FindPath(vector<int>& angle1, vector<int>& angle2)
{
	Update();
	ClearFloodTable();

	int aStart = NormalizeAngle(static_cast<int>(arm1.GetAngle(true)));
	int bStart = NormalizeAngle(static_cast<int>(arm2.GetAngle(true)));

	int aEnd = NormalizeAngle(static_cast<int>(arm1.GetAngle(false)));
	int bEnd = NormalizeAngle(static_cast<int>(arm2.GetAngle(false)));

	RunFlood(aStart, bStart, aEnd, bEnd);

	return RetrievePath(aEnd, bEnd, angle1, angle2);
}

int Simulation::NormalizeAngle(int angle)
{
	while (angle < 0) angle += 360;
	while (angle >= 360) angle -= 360;

	return angle;
}