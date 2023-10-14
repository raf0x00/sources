#include "gui.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include <vector>
#include <boost/multiprecision/cpp_int.hpp> 


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

namespace mp = boost::multiprecision;

bool buttonPressed = false;


static mp::cpp_int count = 8;

long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = LOWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;



	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU)
			return 0;
	}break;


	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;


	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter);
	}return 0;

	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}
	}return 0;

	}

	return DefWindowProcW(window, message, wideParameter, longParameter);
}


void gui::CreateHWindow(
	const char* windowName,
	const char* className)noexcept
{

	windowClass.cbSize = sizeof(WNDCLASSEXA);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = className;
	windowClass.hIconSm = 0;

	RegisterClassExA(&windowClass);

	window = CreateWindowA(
		className,
		windowName,
		WS_POPUP,
		100,
		100,
		WIDTH,
		HEIGHT,
		0,
		0,
		windowClass.hInstance,
		0
	);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);


}

void gui::DestroyHWindow()noexcept
{

	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

}

bool gui::CreateDevice() noexcept
{

	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;


	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;


}

void gui::ResetDevice() noexcept
{

	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();

}

void gui::DestroyDevice() noexcept
{

	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}

}

void gui::CreateImGui() noexcept
{


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);

}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{

	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

bool isPrimeNumber(mp::cpp_int n) {

	mp::cpp_int i;

	if (n < 2) {
		/* 2未満の場合は素数でない */
		return false;
	}

	/* nが2〜n-1で割り切れるかどうかを確認 */
	for (i = 2; i < n; i++) {
		if (n % i == 0) {
			/* 2〜n-1で割り切れる場合はnは素数でない */
			return false;
		}
	}

	return true;
}


std::vector<mp::cpp_int> primeFactorization(mp::cpp_int n) {
	std::vector<mp::cpp_int> factors;

	// 2で割り続ける
	while (n % 2 == 0) {
		factors.push_back(2);
		n = n / 2;
	}

	// 奇数の素因数を探す
	for (int i = 3; i < n; i = i + 2) {
		while (n % i == 0) {
			factors.push_back(i);
			n = n / i;
		}
	}

	// nが素数の場合
	if (isPrimeNumber(n)) {
		factors.push_back(n);
	}

	return factors;
}

bool isIntegerInArray(std::vector<mp::cpp_int>& array, mp::cpp_int target) {
	for (mp::cpp_int element : array) {
		if (element == target) {
			return true; // 特定の整数が配列内に存在する場合
		}
	}
	return false; // 特定の整数が配列内に存在しない場合
}


mp::cpp_int AddPrime(std::vector<mp::cpp_int> list) {

	mp::cpp_int product = 1;

	for (int i = 0; i < list.size(); i++)
	{
		product *= list[i];
	}
	product++;
	if (isPrimeNumber(product)) {
		return product;
	}

	std::vector<mp::cpp_int> fact;
	fact = primeFactorization(product);
	for (int i = 0; i < fact.size(); i++) {
		if (!(isIntegerInArray(list, fact[i]))) {
			return fact[i];
		}
	}




}

void gui::Render() noexcept
{

	ImGui::SetNextWindowPos({ 0,0 });
	ImGui::SetNextWindowSize({ WIDTH,HEIGHT });
	if (ImGui::Begin(
		"hack_Tool",
		&exit,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove
	)) {

		std::vector<mp::cpp_int> prime_list = { 2,3 };
		mp::cpp_int new_prime = 0;


		//Start-ボタン
		if (ImGui::Button("click me"))
		{
			buttonPressed = true;
			count++;
		}


		if (buttonPressed) {
			for (int i = 0; i < count; i++) {
				new_prime = AddPrime(prime_list);
				prime_list.push_back(new_prime);
				ImGui::Text("%d", prime_list[i]);
			}
		}

		//End-ボタン

	/*

		//Start-チェックボックス
		static bool toggle = false;
		if (ImGui::Checkbox("toggle me", &toggle)) {
			//チェックボックスが押された時の処理
		}
		//End-チェックボックス

		//Start-ボタン
		if (ImGui::Button("click me"))
		{
			//ボタンが押された時の処理
		}
		//End-ボタン


		//Start-テキストボックス

		//ここでbufは入力された値
		static char buf[256];
		ImGui::InputText("name", buf, 256);
		//End-テキストボックス

	*/





		ImGui::End();
	}
}