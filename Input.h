#pragma once
#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>
#include <wrl.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")



class Input
{
public:

	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNUmber);
private:
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard;
	Microsoft::WRL::ComPtr<IDirectInput8> directInput = nullptr;
	BYTE key[256] = {};
	BYTE keyPre[256] = {};
};
