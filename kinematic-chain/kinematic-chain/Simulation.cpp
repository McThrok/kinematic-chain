#include "Simulation.h"

void Simulation::Init()
{
	editMode = true;
	firstOption = false;

	arm1.length = 100;
	arm1.angle = 30;

	arm2.length = 50;
	arm2.angle = 50;
	SetPosition(Vector2(100, 50));

	parametrizationTable = new Vector4[360 * 360];
}

void Simulation::AddObsticle(Vector2 p1, Vector2 p2)
{
	Obsticle o;
	o.position = Vector2(min(p1.x, p2.x), min(p1.y, p2.y));
	o.size = Vector2(abs(p1.x - p2.x), abs(p1.y - p2.y));
	o.color = GetRandomColor();

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