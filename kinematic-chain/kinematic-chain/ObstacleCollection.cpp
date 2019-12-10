#include "ObstacleCollection.h"

void ObstacleCollection::Init()
{
	selectedIdx = -1;
}

void ObstacleCollection::AddObstacle(Vector2 p1, Vector2 p2)
{
	if (p1.x * p2.x < 0 && p1.y * p2.y < 0)
		return;

	Obstacle o;
	o.position = Vector2(min(p1.x, p2.x), min(p1.y, p2.y));
	o.size = Vector2(abs(p1.x - p2.x), abs(p1.y - p2.y));
	o.color = GetRandomColor();

	if (o.size.x < 10 || o.size.y < 10)
		return;

	obstacles.push_back(o);
}
void ObstacleCollection::DeleteSelected()
{
	if (selectedIdx == -1) return;

	obstacles.erase(obstacles.begin() + selectedIdx, obstacles.begin() + selectedIdx + 1);
	selectedIdx = -1;
}
void ObstacleCollection::Select(int x, int y)
{
	for (int i = obstacles.size() - 1; i >= 0; i--)
	{
		Obstacle& o = obstacles[i];
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
Vector4 ObstacleCollection::GetRandomColor()
{
	float r = GetRandomFloat(0, 1);
	float g = GetRandomFloat(0, 1);
	float b = GetRandomFloat(0, 1);
	return Vector4(r, g, b, 1);
}
float ObstacleCollection::GetRandomFloat(float min, float max)
{
	return  std::uniform_real_distribution<float>{min, max}(gen);
}

int ObstacleCollection::CheckSegment(Vector2 v1, Vector2 v2)
{
	for (int i = 0; i < obstacles.size(); i++)
	{
		Obstacle& o = obstacles[i];
		Vector2 p = o.position;
		Vector2 s = o.size;

		if (SegmentIntersect(v1, v2, p, Vector2(p.x + s.x, p.y))
			|| SegmentIntersect(v1, v2, p, Vector2(p.x, p.y + s.y))
			|| SegmentIntersect(v1, v2, Vector2(p.x + s.x, p.y), p + s)
			|| SegmentIntersect(v1, v2, Vector2(p.x, p.y + s.y), p + s))
			return i;
	}

	return -1;
}
bool ObstacleCollection::SegmentIntersect(Vector2 p1, Vector2 q1, Vector2 p2, Vector2 q2)
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
bool ObstacleCollection::OnSegment(Vector2 p, Vector2 q, Vector2 r)
{
	return q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) && q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y);
}
int ObstacleCollection::CheckOrientation(Vector2 p, Vector2 q, Vector2 r)
{
	int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
	return (val == 0) ? 0 : (val > 0) ? 1 : 2;
}