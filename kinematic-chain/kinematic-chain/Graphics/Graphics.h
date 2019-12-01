#pragma once
#include "AdapterReader.h"
#include "Shaders.h"
#include "Vertex.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Camera.h"
#include "..\\Timer.h"
#include "..\\Simulation.h"
#include "ImGUI\\imgui.h"
#include "ImGUI\\imgui_impl_win32.h"
#include "ImGUI\\imgui_impl_dx11.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class Graphics
{
public:
	~Graphics();
	bool Initialize(HWND hwnd, int width, int height);
	void RenderFrame();
	Camera camera;
	Simulation* simulation;

	int windowWidth = 0;
	int windowHeight = 0;
private:
	bool InitializeDirectX(HWND hwnd);
	bool InitializeShaders();
	bool InitializeScene();

	void InitGui(HWND hwnd);
	void RenderGui();
	void RenderMainPanel();
	void RenderVisualization();

	void RenderParametrisation();
	void RenderObsticles();
	void RenderAxis();
	void RenderArms();
	void RenderSquare(Matrix worldMatrix, Vector4 color);


	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;

	VertexShader color_vs;
	PixelShader color_ps;

	VertexShader texture_vs;
	PixelShader texture_ps;

	ConstantBuffer<ColoredObjectBuffer> cbColoredObject;

	VertexBuffer<VertexPN> vbSquare;
	IndexBuffer ibSquare;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;


	Timer fpsTimer;
};