#include "Robot.h"


void Robot::Init()
{
	arm1.length = 150;
	arm2.length = 100;

	useAltStart = false;
	useAltEnd = false;

	arm1.useAltStart = &useAltStart;
	arm1.useAltEnd = &useAltEnd;
	arm2.useAltStart = &useAltStart;
	arm2.useAltEnd = &useAltEnd;

	SetPosition(Vector2(150, 100), true);
	SetPosition(Vector2(100, 150), false);
}

RobotState Robot::GetState(float animationProgress)
{
	animationProgress = animationProgress < 0 ? 0 : animationProgress > 1 ? 1 : animationProgress;

	RobotState rs;

	if (animationProgress == 0)
	{
		rs.armAngle1 = arm1.GetAngle(true);
		rs.armAngle2 = arm2.GetAngle(true);
	}
	else if (animationProgress == 1)
	{
		rs.armAngle1 = arm1.GetAngle(false);
		rs.armAngle2 = arm2.GetAngle(false);
	}
	else
	{
		if (angle.size() == 0)
		{
			rs.isEmpty = true;
			return rs;
		}
		else
		{
			rs.armAngle1 = InterpolateAngle(animationProgress, true);
			rs.armAngle2 = InterpolateAngle(animationProgress, false);
		}
	}

	rs.isEmpty = false;

	rs.armMatrix1 = Matrix::CreateScale(arm1.length, 1, 1) * Matrix::CreateRotationY(-XMConvertToRadians(rs.armAngle1));
	Matrix world = Matrix::CreateScale(arm2.length, 1, 1) * Matrix::CreateRotationY(-XMConvertToRadians(rs.armAngle2));
	Matrix t = Matrix::CreateTranslation(Vector3(arm1.length, 0, 0));
	Matrix r = Matrix::CreateRotationY(XMConvertToRadians(-rs.armAngle1));
	rs.armMatrix2 = world * t * r;

	return rs;
}

bool Robot::SetPosition(Vector2 position, bool start)
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

float Robot::InterpolateAngle(float animationProgress, bool firstArm)
{
	int max = angle.size() - 1;
	int idx = static_cast<int>(floorf(animationProgress * max));
	float localProgress = (animationProgress * max - idx) / (1.0f / max);

	float start = NormalizeAngle(firstArm ? angle[idx].first : angle[idx].second);
	float end = NormalizeAngle(firstArm ? angle[idx + 1].first : angle[idx + 1].second);

	if (abs(end - start) > 180)
		end -= 360;

	return NormalizeAngle(start + localProgress * (end - start));
}

float Robot::NormalizeAngle(float angle)
{
	while (angle < 0) angle += 360;
	while (angle >= 360) angle -= 360;

	return angle;
}