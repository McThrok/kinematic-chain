#include "Simulation.h"

void Simulation::Init()
{
	editMode = true;
	firstOption = false;

	arm1.length = 150;
	arm2.length = 100;

	SetPosition(Vector2(150, 100));

	parametrizationTable = new Vector4[360 * 360];
}

void Simulation::AddObsticle(Vector2 p1, Vector2 p2)
{
	if (p1.x * p2.x < 0 && p1.y * p2.y < 0)
		return;

	Obsticle o;
	o.position = Vector2(min(p1.x, p2.x), min(p1.y, p2.y));
	o.size = Vector2(abs(p1.x - p2.x), abs(p1.y - p2.y));
	o.color = GetRandomColor();

	if (o.size.x == 0 || o.size.y == 0)
		return;

	obsitcles.push_back(o);
}

Vector4 Simulation::GetRandomColor()
{
	float r = GetRandomFloat(0, 1);
	float g = GetRandomFloat(0, 1);
	float b = GetRandomFloat(0, 1);
	return Vector4(r, g, b, 1);
}


bool Simulation::SetPosition(Vector2 position)
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

	arm1.angle = XMConvertToDegrees(XMScalarACos(Vector2(1, 0).Dot(v1) / v1.Length()));
	arm2.angle = XMConvertToDegrees(XMScalarACos(v1.Dot(w1) / v1.Length() / w1.Length()));
	if (((Vector3)XMVector2Cross(Vector2(1, 0), v1)).x < 0) arm1.angle *= -1;
	if (((Vector3)XMVector2Cross(v1, w1)).x < 0) arm2.angle *= -1;

	arm1.altAngle = XMConvertToDegrees(XMScalarACos(Vector2(1, 0).Dot(v2) / v2.Length()));
	arm2.altAngle = XMConvertToDegrees(XMScalarACos(v2.Dot(w2) / v2.Length() / w2.Length()));
	if (((Vector3)XMVector2Cross(Vector2(1, 0), v2)).x < 0) arm1.altAngle *= -1;
	if (((Vector3)XMVector2Cross(v2, w2)).x < 0) arm2.altAngle *= -1;
}

void Simulation::SwapAngles()
{
	swap(arm1.angle, arm1.altAngle);
	swap(arm2.angle, arm2.altAngle);
}

float Simulation::GetRandomFloat(float min, float max)
{
	return  std::uniform_real_distribution<float>{min, max}(gen);
}

void Simulation::Update()
{
	Vector2 v1(0, 0);

	for (int i = 0; i < 360; i++)
	{
		Matrix t1 = Matrix::CreateTranslation(Vector3(arm1.length, 0, 0));
		Matrix r1 = Matrix::CreateRotationY(XMConvertToRadians(-i));

		Vector3 w2 = XMVector3TransformCoord(Vector3(arm1.length, 0, 0), r1);
		Vector2 v2(w2.x, w2.z);

		for (int j = 0; j < 360; j++)
		{
			Matrix r2 = Matrix::CreateRotationY(XMConvertToRadians(-j));
			Matrix m = r2 * t1 * r1;

			Vector3 w3 = XMVector3TransformCoord(Vector3(arm2.length, 0, 0), m);
			Vector2 v3(w3.x, w3.z);


			Vector4 color(1, 1, 1, 1);
			if (CheckSegment(v1, v2, color))
				CheckSegment(v2, v3, color);

			parametrizationTable[i * 360 + j] = color;
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