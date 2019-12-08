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
		if (properAnglesStart)
		{
			rs.armAngle1 = arm1.GetAngle(true);
			rs.armAngle2 = arm2.GetAngle(true);
		}
		else
		{
			rs.isEmpty = true;
			return rs;
		}
	}
	else if (animationProgress == 1)
	{
		if (properAnglesEnd)
		{
			rs.armAngle1 = arm1.GetAngle(false);
			rs.armAngle2 = arm2.GetAngle(false);
		}
		else
		{
			rs.isEmpty = true;
			return rs;
		}
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

void Robot::SetPosition(Vector2 position, bool start)
{
	float l1 = arm1.length;
	float l2 = arm2.length;
	float px = position.x;
	float py = position.y;
	float l = sqrtf(px * px + py * py);

	float* angle1;
	float* angle2;
	float* angleAlt1;
	float* angleAlt2;
	bool* properAngles;

	if (start)
	{
		angle1 = &arm1.startAngle;
		angle2 = &arm2.startAngle;
		angleAlt1 = &arm1.startAngleAlt;
		angleAlt2 = &arm2.startAngleAlt;
		properAngles = &properAnglesStart;
	}
	else
	{
		angle1 = &arm1.endAngle;
		angle2 = &arm2.endAngle;
		angleAlt1 = &arm1.endAngleAlt;
		angleAlt2 = &arm2.endAngleAlt;
		properAngles = &properAnglesEnd;
	}

	float beta = atan2f(py, px);
	float delta = acosf((l1 * l1 + l * l - l2 * l2) / (2 * l1 * l));
	float gamma = acosf((l1 * l1 + l2 * l2 - l * l) / (2 * l1 * l2));

	if (isnan(beta) || isnan(delta) || isnan(gamma))
	{
		*properAngles = false;
		return;
	}
	*properAngles = true;

	*angle1 = XMConvertToDegrees(beta + delta);
	*angle2 = XMConvertToDegrees(XM_PI + gamma);

	*angleAlt1 = XMConvertToDegrees(beta - delta);
	*angleAlt2 = XMConvertToDegrees(XM_PI - gamma);
}

float Robot::InterpolateAngle(float animationProgress, bool firstArm)
{
	int idx = static_cast<int>(floorf(animationProgress * angle.size()));
	float localProgress = (animationProgress * angle.size() - idx);

	float start = NormalizeAngle(firstArm ? angle[idx].first : angle[idx].second);

	if (idx == angle.size() - 1)
		return start;

	float end = NormalizeAngle(firstArm ? angle[idx + 1].first : angle[idx + 1].second);

	if (end - start > 180)
		end -= 360;
	else if (start - end > 180)
		start -= 360;

	return NormalizeAngle(start + localProgress * (end - start));
}

float Robot::NormalizeAngle(float angle)
{
	while (angle < 0) angle += 360;
	while (angle >= 360) angle -= 360;

	return angle;
}