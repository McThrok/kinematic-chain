#include "Simulation.h"

void Simulation::Init()
{
	editMode = true;

	arm1.length = 50;
	arm1.angle = 45;

	arm2.length = 100;
	arm2.angle = 135;
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

float Simulation::GetRandomFloat(float min, float max)
{
	return  std::uniform_real_distribution<float>{min, max}(gen);
}