#include "Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	timer.Start();

	if (!this->render_window.Initialize(this, hInstance, window_title, window_class, width, height))
		return false;

	if (!gfx.Initialize(this->render_window.GetHWND(), width, height))
		return false;

	gfx.simulation = &simulation;
	simulation.Init();

	return true;
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::Update()
{
	float dt = timer.GetMilisecondsElapsed();
	timer.Restart();

	gfx.simulation->UpdateAnimation(dt);

	HandleInput();
}
void Engine::HandleInput()
{
	while (!keyboard.CharBufferIsEmpty())
	{
		unsigned char ch = keyboard.ReadChar();
	}

	while (!keyboard.KeyBufferIsEmpty())
	{
		KeyboardEvent kbe = keyboard.ReadKey();
		unsigned char keycode = kbe.GetKeyCode();
	}


	//for edit mode
	static int x = 0, y = 0;

	while (!mouse.EventBufferIsEmpty())
	{
		MouseEvent me = mouse.ReadEvent();

		if (keyboard.KeyIsPressed(0x10))//shift
		{
			if (me.GetType() == MouseEvent::RPress)
			{
				simulation.Obstacles.Select(me.GetPosX() - gfx.windowWidth / 2, -me.GetPosY() + gfx.windowHeight / 2);
			}
			if (me.GetType() == MouseEvent::LPress)
			{
				x = me.GetPosX();
				y = me.GetPosY();
			}
			if (me.GetType() == MouseEvent::LRelease)
			{
				Vector2 ScreenOffest(gfx.windowWidth / 2, -gfx.windowHeight / 2);
				auto p1 = Vector2(x, -y) - ScreenOffest;
				auto p2 = Vector2(me.GetPosX(), -me.GetPosY()) - ScreenOffest;

				simulation.Obstacles.AddObstacle(p1, p2);
			}
		}
		if (keyboard.KeyIsPressed(0x11))//lctrl
		{
			if (me.GetType() == MouseEvent::LPress)
			{
				Vector2 ScreenOffest(gfx.windowWidth / 2, -gfx.windowHeight / 2);
				Vector2 p = Vector2(me.GetPosX(), -me.GetPosY()) - ScreenOffest;
				simulation.SetPosition(p, true);
			}
			if (me.GetType() == MouseEvent::RPress)
			{
				Vector2 ScreenOffest(gfx.windowWidth / 2, -gfx.windowHeight / 2);
				Vector2 p = Vector2(me.GetPosX(), -me.GetPosY()) - ScreenOffest;
				simulation.SetPosition(p, false);
			}
		}

	}
}

void Engine::RenderFrame()
{
	this->gfx.RenderFrame();
}
