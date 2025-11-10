#include "Input.h"
#include <wrl.h>
#include <cassert>

void Input::Initialize(WinApp* winApp)
{
	this->winApp = winApp;
	HRESULT result = DirectInput8Create
		
	(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
	(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	result = keyboard->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);

	assert(SUCCEEDED(result));
	

}

void Input::Update()
{
	memcpy(keyPre, key, sizeof(key));
	keyboard->Acquire();

	
	keyboard->GetDeviceState(sizeof(key), key);
}

bool Input::PushKey(BYTE keyNumber)
{
	if (key[DIK_0])
	{
		OutputDebugStringA("Hit 0\n");
	}
	return false;
}

bool Input::TriggerKey(BYTE keyNUmber)
{
	if (!keyPre[DIK_0] && key[DIK_0])
	{
		return true;
	}
	return false;
}
