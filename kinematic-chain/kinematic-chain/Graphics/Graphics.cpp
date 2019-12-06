#include "Graphics.h"

Graphics::~Graphics()
{
}

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	this->windowWidth = width;
	this->windowHeight = height;
	this->fpsTimer.Start();

	if (!InitializeDirectX(hwnd))
		return false;

	if (!InitializeShaders())
		return false;

	if (!InitializeScene())
		return false;

	InitGui(hwnd);

	return true;
}
void Graphics::RenderFrame()
{
	float bgcolor[] = { 0.05f, 0.05f, 0.1f, 1.0f };
	this->deviceContext->ClearRenderTargetView(this->renderTargetView.Get(), bgcolor);
	this->deviceContext->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	RenderVisualization();
	RenderGui();

	this->swapchain->Present(0, NULL);
}


void Graphics::InitGui(HWND hwnd) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(this->device.Get(), this->deviceContext.Get());
	ImGui::StyleColorsDark();
}

void Graphics::RenderGui() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	RenderMainPanel();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Graphics::RenderMainPanel() {
	ImGui::SetNextWindowSize(ImVec2(500, 450), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_Once);
	if (!ImGui::Begin("Main Panel"))
	{
		ImGui::End();
		return;
	}

	//ImGui::Text("Arm 1");
	//ImGui::DragFloat("length##1", &simulation->arm1.length, 1);
	//ImGui::DragFloat("start angle##start1", &simulation->arm1.startAngle, 1);
	//ImGui::DragFloat("start angle alt##startAlt1", &simulation->arm1.startAngleAlt, 1);
	//ImGui::DragFloat("end angle##end1", &simulation->arm1.endAngle, 1);
	//ImGui::DragFloat("end angle alt##endAlt1", &simulation->arm1.endAngleAlt, 1);

	//ImGui::Text("Arm 2");
	//ImGui::DragFloat("length##2", &simulation->arm2.length, 1);
	//ImGui::DragFloat("start angle##start2", &simulation->arm2.startAngle, 1);
	//ImGui::DragFloat("start angle alt##startAlt2", &simulation->arm2.startAngleAlt, 1);
	//ImGui::DragFloat("end angle##end2", &simulation->arm2.endAngle, 1);
	//ImGui::DragFloat("end angle alt##endAlt2", &simulation->arm2.endAngleAlt, 1);

	//ImGui::Separator();
	//ImGui::Checkbox("use alt start", &simulation->useAltStart);
	//ImGui::Checkbox("use alt end", &simulation->useAltEnd);

	if (ImGui::Button("Update scene"))
	{
		simulation->UpdateValues();
		UpdateTexture();
	}

	if (ImGui::Button("Find path"))
	{
		vector<int> angle1;
		vector<int> angle2;
		simulation->UpdateValues();
		if (simulation->FindPath())
			for (int i = 0; i < angle1.size(); i++)
				simulation->parametrizationTable.get()[angle1[i] * 360 + angle2[i]] = Vector4(1, 0, 0, 1);
		UpdateTexture();
	}

	ImGui::Separator();

	int idx = simulation->Obstacles.selectedIdx;
	if (idx != -1) {
		Obstacle& o = simulation->Obstacles.obstacles[idx];

		ImGui::DragFloat2("position", &o.position.x, 1);
		ImGui::DragFloat2("size", &o.size.x, 1);

		if (ImGui::Button("Delete"))
			simulation->Obstacles.DeleteSelected();

		ImGui::ColorPicker3("color", &o.color.x);
	}
	ImGui::End();
}

void Graphics::RenderVisualization()
{
	this->deviceContext->IASetInputLayout(this->color_vs.GetInputLayout());
	this->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->deviceContext->RSSetState(this->rasterizerState.Get());
	this->deviceContext->OMSetDepthStencilState(this->depthStencilState.Get(), 0);

	this->deviceContext->VSSetShader(color_vs.GetShader(), NULL, 0);
	this->deviceContext->PSSetShader(color_ps.GetShader(), NULL, 0);
	this->deviceContext->VSSetConstantBuffers(0, 1, this->cbColoredObject.GetAddressOf());
	this->deviceContext->PSSetConstantBuffers(0, 1, this->cbColoredObject.GetAddressOf());

	RenderAxis();
	RenderObstacles();
	RenderArms();

	this->deviceContext->VSSetShader(texture_vs.GetShader(), NULL, 0);
	this->deviceContext->PSSetShader(texture_ps.GetShader(), NULL, 0);
	this->deviceContext->VSSetConstantBuffers(0, 1, this->cbObject.GetAddressOf());
	this->deviceContext->PSSetConstantBuffers(0, 1, this->cbObject.GetAddressOf());
	this->deviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());
	this->deviceContext->PSSetShaderResources(0, 1, my_textureRV.GetAddressOf());

	RenderParametrisation();
}

void Graphics::RenderArms()
{
	RobotState start = simulation->robot.GetState(0);
	RenderSquare(start.armMatrix1, Vector4(1, 1, 0, 1));
	RenderSquare(start.armMatrix2, Vector4(1, 1, 0, 1));

	RobotState end = simulation->robot.GetState(1);
	RenderSquare(end.armMatrix1, Vector4(1, 0, 1, 1));
	RenderSquare(end.armMatrix2, Vector4(1, 0, 1, 1));
}
void Graphics::RenderAxis()
{
	float thickness = 2;
	Matrix center = Matrix::CreateTranslation(-0.5f, 0, -0.5f);
	Matrix xwm = center * Matrix::CreateScale(windowWidth, 0, thickness);
	Matrix ywm = center * Matrix::CreateScale(thickness, 0, windowWidth);
	RenderSquare(xwm, Vector4(0.8, 0, 0, 1));
	RenderSquare(ywm, Vector4(0, 0.8, 0, 1));
}

void Graphics::RenderObstacles()
{
	for (auto& o : simulation->Obstacles.obstacles)
		RenderSquare(o.GetWorldMatrix(), o.color);
}

void Graphics::RenderSquare(Matrix worldMatrix, Vector4 color)
{
	UINT offset = 0;

	cbColoredObject.data.worldMatrix = worldMatrix;
	cbColoredObject.data.wvpMatrix = cbColoredObject.data.worldMatrix * camera.GetViewMatrix() * camera.GetProjectionMatrix();
	cbColoredObject.data.color = color;

	cbColoredObject.ApplyChanges();
	this->deviceContext->IASetVertexBuffers(0, 1, vbSquare.GetAddressOf(), vbSquare.StridePtr(), &offset);
	this->deviceContext->IASetIndexBuffer(ibSquare.Get(), DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(ibSquare.BufferSize(), 0, 0);
}

void Graphics::RenderParametrisation()
{
	UINT offset = 0;

	cbObject.data.worldMatrix = Matrix::CreateScale(360) * Matrix::CreateTranslation(-930, 0, -470);
	cbObject.data.wvpMatrix = cbObject.data.worldMatrix * camera.GetViewMatrix() * camera.GetProjectionMatrix();

	cbObject.ApplyChanges();
	this->deviceContext->IASetVertexBuffers(0, 1, vbSquareT.GetAddressOf(), vbSquareT.StridePtr(), &offset);
	this->deviceContext->IASetIndexBuffer(ibSquare.Get(), DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(ibSquare.BufferSize(), 0, 0);

}

bool Graphics::InitializeDirectX(HWND hwnd)
{
	std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

	if (adapters.size() < 1)
	{
		ErrorLogger::Log("No IDXGI Adapters found.");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferDesc.Width = this->windowWidth;
	scd.BufferDesc.Height = this->windowHeight;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = hwnd;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(adapters[0].pAdapter, //IDXGI Adapter
		D3D_DRIVER_TYPE_UNKNOWN,
		NULL, //FOR SOFTWARE DRIVER TYPE
		NULL, //FLAGS FOR RUNTIME LAYERS
		NULL, //FEATURE LEVELS ARRAY
		0, //# OF FEATURE LEVELS IN ARRAY
		D3D11_SDK_VERSION,
		&scd, //Swapchain description
		this->swapchain.GetAddressOf(), //Swapchain Address
		this->device.GetAddressOf(), //Device Address
		NULL, //Supported feature level
		this->deviceContext.GetAddressOf()); //Device Context Address

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create device and swapchain.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	hr = this->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "GetBuffer Failed.");
		return false;
	}

	hr = this->device->CreateRenderTargetView(backBuffer.Get(), NULL, this->renderTargetView.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "Failed to create render target view.");
		return false;
	}

	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = this->windowWidth;
	depthStencilDesc.Height = this->windowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = this->device->CreateTexture2D(&depthStencilDesc, NULL, this->depthStencilBuffer.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "Failed to create depth stencil buffer.");
		return false;
	}

	hr = this->device->CreateDepthStencilView(this->depthStencilBuffer.Get(), NULL, this->depthStencilView.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "Failed to create depth stencil view.");
		return false;
	}

	this->deviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());

	//Create depth stencil state
	D3D11_DEPTH_STENCIL_DESC depthstencildesc;
	ZeroMemory(&depthstencildesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthstencildesc.DepthEnable = true;
	depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	hr = this->device->CreateDepthStencilState(&depthstencildesc, this->depthStencilState.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create depth stencil state.");
		return false;
	}

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = this->windowWidth;
	viewport.Height = this->windowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;


	//Set the Viewport
	this->deviceContext->RSSetViewports(1, &viewport);

	//Create Rasterizer State
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	hr = this->device->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create rasterizer state.");
		return false;
	}

	//Create sampler description for sampler state
	CD3D11_SAMPLER_DESC sampDesc(D3D11_DEFAULT);
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = this->device->CreateSamplerState(&sampDesc, this->samplerState.GetAddressOf()); //Create sampler state

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = 360;
	desc.Height = 360;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	hr = this->device->CreateTexture2D(&desc, nullptr, this->my_texture.GetAddressOf());
	hr = this->device->CreateShaderResourceView(my_texture.Get(), 0, my_textureRV.GetAddressOf());
	deviceContext->PSSetShaderResources(0, 1, my_textureRV.GetAddressOf());

	return true;
}

void Graphics::UpdateTexture()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(my_texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	unsigned int* data = ((unsigned int*)mappedResource.pData);
	Vector4* src = simulation->parametrizationTable.get();

	for (int i = 0; i < 360; i++)
	{
		for (int j = 0; j < 360; j++)
		{
			Vector4 color = src[j + i * 360];
			unsigned int a = 255U * color.w;
			unsigned int r = 255U * color.x;
			unsigned int g = 255U * color.y;
			unsigned int b = 255U * color.z;
			data[j + i * mappedResource.RowPitch / 4] = (a << 24) + (b << 16) + (g << 8) + r;
		}
	}

	deviceContext->Unmap(my_texture.Get(), 0);
}

bool Graphics::InitializeShaders()
{
	D3D11_INPUT_ELEMENT_DESC layoutPN[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElementsPN = ARRAYSIZE(layoutPN);

	if (!color_vs.Initialize(this->device, L"my_vs.cso", layoutPN, numElementsPN))
		return false;

	if (!color_ps.Initialize(this->device, L"my_ps.cso"))
		return false;


	D3D11_INPUT_ELEMENT_DESC layoutPT[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElementsPT = ARRAYSIZE(layoutPT);


	if (!texture_vs.Initialize(this->device, L"texture_vs.cso", layoutPT, numElementsPT))
		return false;

	if (!texture_ps.Initialize(this->device, L"texture_ps.cso"))
		return false;

	return true;
}

bool Graphics::InitializeScene()
{
	VertexPN v[] = {
		VertexPN(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f),
		VertexPN(1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f),
		VertexPN(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		VertexPN(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)
	};

	VertexPT vT[] = {
		VertexPT(0.0f, 0.0f, 1.0f, 0.0f, 1.0f),
		VertexPT(1.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		VertexPT(1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		VertexPT(0.0f, 0.0f, 0.0f, 0.0f, 0.0f)
	};

	int indices[] = { 0, 2, 3, 0,1, 2 };

	this->vbSquare.Initialize(this->device.Get(), v, ARRAYSIZE(v));
	this->vbSquareT.Initialize(this->device.Get(), vT, ARRAYSIZE(v));
	this->ibSquare.Initialize(this->device.Get(), indices, ARRAYSIZE(indices));

	//Initialize Constant Buffer(s)
	this->cbColoredObject.Initialize(this->device.Get(), this->deviceContext.Get());
	this->cbObject.Initialize(this->device.Get(), this->deviceContext.Get());

	camera.SetPosition(0, -5.0f, 0);
	camera.SetOrthogonalProjection(windowWidth, windowHeight, 0.1f, 1000.0f);

	return true;
}
