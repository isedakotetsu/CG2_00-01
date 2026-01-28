#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment (lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "xaudio2.lib")


#include <windows.h>
#include<cstdint>
#include <string>
#include <format>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include<dbghelp.h>
#include<strsafe.h>
#include<dxgidebug.h>
#include<dxcapi.h>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#include <vector>
#include <numbers>
#include <sstream>
#include <wrl.h>
#include <xaudio2.h>
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "D3DResourceLeakChecker.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"











struct Matrix4x4 {
	float m[4][4];
};
struct Matrix3x3 
{
	float m[3][3];
};

struct Transform
{
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};
struct Sphere {
	// 中心点
	Vector3 center;

	float radius;
};
struct VertexData
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};
struct Material
{
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};
struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 World;
};
struct DirectionalLight
{
	Vector4 color;//ライトの色
	Vector3 direction;//ライトの向き
	float intensity;//輝度
};
struct MaterialData
{
	std::string textureFilePath;
};
struct ModelData
{
	std::vector<VertexData> vertices;
	MaterialData material;
};

//正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip)
{
	Matrix4x4 result = {};
	result.m[0][0] = 2 / (right - left);
	result.m[0][1] = 0;
	result.m[0][2] = 0;
	result.m[0][3] = 0;
	result.m[1][0] = 0;
	result.m[1][1] = 2 / (top - bottom);
	result.m[1][2] = 0;
	result.m[1][3] = 0;
	result.m[2][0] = 0;
	result.m[2][1] = 0;
	result.m[2][2] = 1 / (farClip - nearClip);
	result.m[2][3] = 0;
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1;
	return result;
}
Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 result{};
	float determinant =
		m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0] - m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0] + m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0] +
		m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0] - m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1] + m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1] +
		m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1] - m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1] + m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1] +
		m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2] - m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2] - m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2] + m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2] +
		m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2] - m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2] - m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3] + m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3] +
		m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3] - m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3] + m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3];


	float determinantRecp = 1.0f / determinant;
	result.m[0][0] = (m.m[1][2] * m.m[2][3] * m.m[3][1] - m.m[1][3] * m.m[2][2] * m.m[3][1] + m.m[1][3] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][3] * m.m[3][2] -
		m.m[1][2] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][2] * m.m[3][3]) *
		determinantRecp;
	result.m[0][1] = (m.m[0][3] * m.m[2][2] * m.m[3][1] - m.m[0][2] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][3] * m.m[3][2] +
		m.m[0][2] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][2] * m.m[3][3]) *
		determinantRecp;
	result.m[0][2] = (m.m[0][2] * m.m[1][3] * m.m[3][1] - m.m[0][3] * m.m[1][2] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][3] * m.m[3][2] -
		m.m[0][2] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][2] * m.m[3][3]) *
		determinantRecp;
	result.m[0][3] = (m.m[0][3] * m.m[1][2] * m.m[2][1] - m.m[0][2] * m.m[1][3] * m.m[2][1] - m.m[0][3] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][3] * m.m[2][2] +
		m.m[0][2] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][2] * m.m[2][3]) *
		determinantRecp;
	result.m[1][0] = (m.m[1][3] * m.m[2][2] * m.m[3][0] - m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[1][3] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][3] * m.m[3][2] +
		m.m[1][2] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][2] * m.m[3][3]) *
		determinantRecp;
	result.m[1][1] = (m.m[0][2] * m.m[2][3] * m.m[3][0] - m.m[0][3] * m.m[2][2] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][3] * m.m[3][2] -
		m.m[0][2] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][2] * m.m[3][3]) *
		determinantRecp;
	result.m[1][2] = (m.m[0][3] * m.m[1][2] * m.m[3][0] - m.m[0][2] * m.m[1][3] * m.m[3][0] - m.m[0][3] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][3] * m.m[3][2] +
		m.m[0][2] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][2] * m.m[3][3]) *
		determinantRecp;
	result.m[1][3] = (m.m[0][2] * m.m[1][3] * m.m[2][0] - m.m[0][3] * m.m[1][2] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][3] * m.m[2][2] -
		m.m[0][2] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][2] * m.m[2][3]) *
		determinantRecp;
	result.m[2][0] = (m.m[1][1] * m.m[2][3] * m.m[3][0] - m.m[1][3] * m.m[2][1] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][1] - m.m[1][0] * m.m[2][3] * m.m[3][1] -
		m.m[1][1] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][1] * m.m[3][3]) *
		determinantRecp;
	result.m[2][1] = (m.m[0][3] * m.m[2][1] * m.m[3][0] - m.m[0][1] * m.m[2][3] * m.m[3][0] - m.m[0][3] * m.m[2][0] * m.m[3][1] + m.m[0][0] * m.m[2][3] * m.m[3][1] +
		m.m[0][1] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][1] * m.m[3][3]) *
		determinantRecp;
	result.m[2][2] = (m.m[0][1] * m.m[1][3] * m.m[3][0] - m.m[0][3] * m.m[1][1] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][1] - m.m[0][0] * m.m[1][3] * m.m[3][1] -
		m.m[0][1] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][1] * m.m[3][3]) *
		determinantRecp;
	result.m[2][3] = (m.m[0][3] * m.m[1][1] * m.m[2][0] - m.m[0][1] * m.m[1][3] * m.m[2][0] - m.m[0][3] * m.m[1][0] * m.m[2][1] + m.m[0][0] * m.m[1][3] * m.m[2][1] +
		m.m[0][1] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][1] * m.m[2][3]) *
		determinantRecp;
	result.m[3][0] = (m.m[1][2] * m.m[2][1] * m.m[3][0] - m.m[1][1] * m.m[2][2] * m.m[3][0] - m.m[1][2] * m.m[2][0] * m.m[3][1] + m.m[1][0] * m.m[2][2] * m.m[3][1] +
		m.m[1][1] * m.m[2][0] * m.m[3][2] - m.m[1][0] * m.m[2][1] * m.m[3][2]) *
		determinantRecp;
	result.m[3][1] = (m.m[0][1] * m.m[2][2] * m.m[3][0] - m.m[0][2] * m.m[2][1] * m.m[3][0] + m.m[0][2] * m.m[2][0] * m.m[3][1] - m.m[0][0] * m.m[2][2] * m.m[3][1] -
		m.m[0][1] * m.m[2][0] * m.m[3][2] + m.m[0][0] * m.m[2][1] * m.m[3][2]) *
		determinantRecp;
	result.m[3][2] = (m.m[0][2] * m.m[1][1] * m.m[3][0] - m.m[0][1] * m.m[1][2] * m.m[3][0] - m.m[0][2] * m.m[1][0] * m.m[3][1] + m.m[0][0] * m.m[1][2] * m.m[3][1] +
		m.m[0][1] * m.m[1][0] * m.m[3][2] - m.m[0][0] * m.m[1][1] * m.m[3][2]) *
		determinantRecp;
	result.m[3][3] = (m.m[0][1] * m.m[1][2] * m.m[2][0] - m.m[0][2] * m.m[1][1] * m.m[2][0] + m.m[0][2] * m.m[1][0] * m.m[2][1] - m.m[0][0] * m.m[1][2] * m.m[2][1] -
		m.m[0][1] * m.m[1][0] * m.m[2][2] + m.m[0][0] * m.m[1][1] * m.m[2][2]) *
		determinantRecp;
	return result;
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float asprectRatio, float nearClip, float farclip)
{
	float cot = 1.0f / std::tanf(fovY / 2.0f);
	Matrix4x4 result = {};
	result.m[0][0] = cot / asprectRatio;
	result.m[0][1] = 0;
	result.m[0][2] = 0;
	result.m[0][3] = 0;
	result.m[1][0] = 0;
	result.m[1][1] = cot;
	result.m[1][2] = 0;
	result.m[1][3] = 0;
	result.m[2][0] = 0;
	result.m[2][1] = 0;
	result.m[2][2] = farclip / (farclip - nearClip);
	result.m[2][3] = 1;
	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = (-nearClip * farclip) / (farclip - nearClip);
	result.m[3][3] = 0;
	return result;
}

Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result = {};
	result.m[0][0] = 1.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = 1.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;
	result.m[2][3] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;
	return result;
}
static Matrix4x4 MakeTranslateMatrix(const Vector3& translate)
{
	Matrix4x4 result = {};
	result.m[0][0] = 1;
	result.m[0][1] = 0;
	result.m[0][2] = 0;
	result.m[0][3] = 0;
	result.m[1][0] = 0;
	result.m[1][1] = 1;
	result.m[1][2] = 0;
	result.m[1][3] = 0;
	result.m[2][0] = 0;
	result.m[2][1] = 0;
	result.m[2][2] = 1;
	result.m[2][3] = 0;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1;
	return result;
}
static Matrix4x4 MakeScaleMatrix(const Vector3& scale)
{

	Matrix4x4 result = {};
	result.m[0][0] = scale.x;
	result.m[0][1] = 0;
	result.m[0][2] = 0;
	result.m[0][3] = 0;
	result.m[1][0] = 0;
	result.m[1][1] = scale.y;
	result.m[1][2] = 0;
	result.m[1][3] = 0;
	result.m[2][0] = 0;
	result.m[2][1] = 0;
	result.m[2][2] = scale.z;
	result.m[2][3] = 0;
	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = 0;
	result.m[3][3] = 1;
	return result;
}

static Matrix4x4 MakeRotateXMatrix(float radian)
{
	Matrix4x4 result = {};
	result.m[0][0] = 1;
	result.m[0][1] = 0;
	result.m[0][2] = 0;
	result.m[0][3] = 0;
	result.m[1][0] = 0;
	result.m[1][1] = std::cos(radian);
	result.m[1][2] = std::sin(radian);
	result.m[1][3] = 0;
	result.m[2][0] = 0;
	result.m[2][1] = -std::sin(radian);
	result.m[2][2] = std::cos(radian);
	result.m[2][3] = 0;
	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = 0;
	result.m[3][3] = 1;
	return result;
}
static Matrix4x4 MakeRotateYMatrix(float radian)
{
	float sinradion = std::sin(radian);
	float cosradion = std::cos(radian);
	Matrix4x4 result = {};
	result.m[0][0] = cosradion;
	result.m[0][1] = 0;
	result.m[0][2] = -sinradion;
	result.m[0][3] = 0;
	result.m[1][0] = 0;
	result.m[1][1] = 1;
	result.m[1][2] = 0;
	result.m[1][3] = 0;
	result.m[2][0] = sinradion;
	result.m[2][1] = 0;
	result.m[2][2] = cosradion;
	result.m[2][3] = 0;
	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = 0;
	result.m[3][3] = 1;
	return result;
}

static Matrix4x4 MakeRotateZMatrix(float radian)
{
	float sinradion = std::sin(radian);
	float cosradion = std::cos(radian);
	Matrix4x4 result = {};
	result.m[0][0] = cosradion;
	result.m[0][1] = sinradion;
	result.m[0][2] = 0;
	result.m[0][3] = 0;
	result.m[1][0] = -sinradion;
	result.m[1][1] = cosradion;
	result.m[1][2] = 0;
	result.m[1][3] = 0;
	result.m[2][0] = 0;
	result.m[2][1] = 0;
	result.m[2][2] = 1;
	result.m[2][3] = 0;
	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = 0;
	result.m[3][3] = 1;
	return result;
}

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	result.m[0][0] = (m1.m[0][0] * m2.m[0][0]) + (m1.m[0][1] * m2.m[1][0]) + (m1.m[0][2] * m2.m[2][0]) + (m1.m[0][3] * m2.m[3][0]);
	result.m[0][1] = (m1.m[0][0] * m2.m[0][1]) + (m1.m[0][1] * m2.m[1][1]) + (m1.m[0][2] * m2.m[2][1]) + (m1.m[0][3] * m2.m[3][1]);
	result.m[0][2] = (m1.m[0][0] * m2.m[0][2]) + (m1.m[0][1] * m2.m[1][2]) + (m1.m[0][2] * m2.m[2][2]) + (m1.m[0][3] * m2.m[3][2]);
	result.m[0][3] = (m1.m[0][0] * m2.m[0][3]) + (m1.m[0][1] * m2.m[1][3]) + (m1.m[0][2] * m2.m[2][3]) + (m1.m[0][3] * m2.m[3][3]);

	result.m[1][0] = (m1.m[1][0] * m2.m[0][0]) + (m1.m[1][1] * m2.m[1][0]) + (m1.m[1][2] * m2.m[2][0]) + (m1.m[1][3] * m2.m[3][0]);
	result.m[1][1] = (m1.m[1][0] * m2.m[0][1]) + (m1.m[1][1] * m2.m[1][1]) + (m1.m[1][2] * m2.m[2][1]) + (m1.m[1][3] * m2.m[3][1]);
	result.m[1][2] = (m1.m[1][0] * m2.m[0][2]) + (m1.m[1][1] * m2.m[1][2]) + (m1.m[1][2] * m2.m[2][2]) + (m1.m[1][3] * m2.m[3][2]);
	result.m[1][3] = (m1.m[1][0] * m2.m[0][3]) + (m1.m[1][1] * m2.m[1][3]) + (m1.m[1][2] * m2.m[2][3]) + (m1.m[1][3] * m2.m[3][3]);

	result.m[2][0] = (m1.m[2][0] * m2.m[0][0]) + (m1.m[2][1] * m2.m[1][0]) + (m1.m[2][2] * m2.m[2][0]) + (m1.m[2][3] * m2.m[3][0]);
	result.m[2][1] = (m1.m[2][0] * m2.m[0][1]) + (m1.m[2][1] * m2.m[1][1]) + (m1.m[2][2] * m2.m[2][1]) + (m1.m[2][3] * m2.m[3][1]);
	result.m[2][2] = (m1.m[2][0] * m2.m[0][2]) + (m1.m[2][1] * m2.m[1][2]) + (m1.m[2][2] * m2.m[2][2]) + (m1.m[2][3] * m2.m[3][2]);
	result.m[2][3] = (m1.m[2][0] * m2.m[0][3]) + (m1.m[2][1] * m2.m[1][3]) + (m1.m[2][2] * m2.m[2][3]) + (m1.m[2][3] * m2.m[3][3]);

	result.m[3][0] = (m1.m[3][0] * m2.m[0][0]) + (m1.m[3][1] * m2.m[1][0]) + (m1.m[3][2] * m2.m[2][0]) + (m1.m[3][3] * m2.m[3][0]);
	result.m[3][1] = (m1.m[3][0] * m2.m[0][1]) + (m1.m[3][1] * m2.m[1][1]) + (m1.m[3][2] * m2.m[2][1]) + (m1.m[3][3] * m2.m[3][1]);
	result.m[3][2] = (m1.m[3][0] * m2.m[0][2]) + (m1.m[3][1] * m2.m[1][2]) + (m1.m[3][2] * m2.m[2][2]) + (m1.m[3][3] * m2.m[3][2]);
	result.m[3][3] = (m1.m[3][0] * m2.m[0][3]) + (m1.m[3][1] * m2.m[1][3]) + (m1.m[3][2] * m2.m[2][3]) + (m1.m[3][3] * m2.m[3][3]);
	return result;
}

struct Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate)
{
	Matrix4x4 result = {};


	Matrix4x4 ScaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 RotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 RotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 RotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 TranslateMatrix = MakeTranslateMatrix(translate);
	result = Multiply(ScaleMatrix, RotateXMatrix);
	result = Multiply(result, RotateYMatrix);
	result = Multiply(result, RotateZMatrix);
	result = Multiply(result, TranslateMatrix);


	return result;



}

float Length(const Vector3& v)
{
	float length = sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	return length;
};

Vector3 Normalize(const Vector3& v)
{
	Vector3 normalize{};
	float length = sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	normalize.x = v.x / length;
	normalize.y = v.y / length;
	normalize.z = v.z / length;

	return normalize;
};





static LONG WINAPI ExposrtDump(EXCEPTION_POINTERS* exception)
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFilehandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();

	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;

	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFilehandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);
	return EXCEPTION_EXECUTE_HANDLER;
}
struct ChunkHeader
{
	char id[4];
	int32_t size;
};
struct RiffHeader
{
	ChunkHeader chunk;
	char type[4];
};
struct FormatChunk
{
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

MaterialData LoadMaterialTempLateFile(const std::string& directoryPath, const std::string& filename)
{
	MaterialData materialData;//構築するmaterialData
	std::string line;//ファイルから読んだ1桁を格納するもの
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなかったら止める
	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		//identifierに応じた処理
		if (identifier == "map_Kd")
		{
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパス
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData;
	std::vector<Vector4> positions;//位置
	std::vector<Vector3> normals;//法線
	std::vector<Vector2> texcoords;//テクスチャ座標
	std::string line; //ファイルから読んだ1桁を格納するもの
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなかったら止める

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;//先頭の識別子を読む

		if (identifier == "v")
		{
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			//position.y *= -1.0f;
			position.x *= -1.0f;
			positions.push_back(position);

		}
		else if (identifier == "vt")
		{
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn")
		{
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			//normal.y *= -1.0f;
			normal.x *= -1.0f;
			normals.push_back(normal);
		}
		else if (identifier == "f")
		{
			VertexData triangle[3];
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex)
			{
				std::string vertexDefinition;
				s >> vertexDefinition;
			//頂点の要素へのindexは「位置・UV・法線」で格納されているので、ぶんかいしてindexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element)
				{
					std::string index;
					std::getline(v, index, '/');//区切りでインデックスを読み込んでいく
					elementIndices[element] = std::stoi(index);
				}
				//要素へのindexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				/*VertexData vertex = { position, texcoord, normal };
				modelData.vertices.push_back(vertex);*/
				triangle[faceVertex] = { position, texcoord, normal };

			}
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib")
		{
			//materialTemplateLibraryファイルの名前を取得
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTempLateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
	
}





int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

	D3DResourceLeakChecker leakCheck;

	CoInitializeEx(0, COINIT_MULTITHREADED);
	SetUnhandledExceptionFilter(ExposrtDump);

	
	Input* input = nullptr;
	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	

	
	winApp = new WinApp();
	winApp->Initialize();

	input = new Input();
	input->Initialize(winApp);

	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	TextureManager::GetInstance()->Initialize();

	SpriteCommon* spriteCommon = nullptr;
	//スプライト共通部の初期化
	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);

	Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon);

	std::vector<Sprite*> sprites;
	for (uint32_t i = 0; i < 5; ++i)
	{
		Sprite* sprite = new Sprite();
		sprite->Initialize(spriteCommon);

		sprite->SetSize({ 64.0f, 64.0f });
		sprite->SetPosition({ 50.0f + 100.0f * i, 50.0f });

		sprites.push_back(sprite);
	}

	

	//log出力用のフォルダ[logs]作成
	std::filesystem::create_directory("logs");

	//現在時刻を取得
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	//ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);

	//日本時間に変換（PCの設定）
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };

	//formatを使って年月日時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);

	//時刻を使ってファイル名を決定
	std::string logfilePath = std::string("logs/") + dateString + ".log";

	//ファイルを作って書き込み準備
	std::ofstream logstream(logfilePath);
	
	
	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;//0から始まる
	descriptorRange[0].NumDescriptors = 1;//数は一つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//offsetを自動計算

	
	//RootParameter作成。複数設定できるので配列。今回は結果１つだけなので長さ１の配列
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;//レジスタ番号０とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;//レジスタ番号を使う
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//descriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//pixelshaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//tableで利用する数
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//pixelshdaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;//レジスタ番号1を使う
	descriptionRootSignature.pParameters = rootParameters;//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ

	//samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0-1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのmipmapを使う
	staticSamplers[0].ShaderRegister = 0;//レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//pixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	

	

	

	//シリアライズしてバイナリする
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Logger::Log(logstream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);

	}
	//バイナリをもとに生成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//blendstateの設定
	D3D12_BLEND_DESC blendDesc{};
	//全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//ResiterzeStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;





	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	//Shaderをコンパイルする
	ComPtr<IDxcBlob> vertexShaderBlob = dxCommon->CompileShader(L"resources/shaders/Object3D.VS.hlsl",
		L"vs_6_0", dxcUtils, dxcCompiler, includeHandler, logstream);
	assert(vertexShaderBlob != nullptr);

	ComPtr<IDxcBlob> pixelShaderBlob = dxCommon->CompileShader(L"resources/shaders/Object3D.PS.hlsl",
		L"ps_6_0", dxcUtils, dxcCompiler, includeHandler, logstream);
	assert(pixelShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPirelineStateDesc{};
	graphicsPirelineStateDesc.pRootSignature = rootSignature.Get();//rootsignature
	graphicsPirelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
	graphicsPirelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//vetexShader
	graphicsPirelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//PixelShader
	graphicsPirelineStateDesc.BlendState = blendDesc;//BlendState
	graphicsPirelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	//書き込むRTVの情報
	graphicsPirelineStateDesc.NumRenderTargets = 1;
	graphicsPirelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ（形状）のタイプ。三角形
	graphicsPirelineStateDesc.PrimitiveTopologyType =
	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定（気にしなくてもよい)
	graphicsPirelineStateDesc.SampleDesc.Count = 1;
	graphicsPirelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//depthstencilstateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はlessequal。つまり近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//depthStencilの設定
	graphicsPirelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPirelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPirelineStateDesc,
	IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));


	ModelData modelData = LoadObjFile("resources", "bunny.obj");



	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * modelData.vertices.size());

	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	//1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData)* modelData.vertices.size());

	//Obj用のtransformationMatrix用のリソースを作る。matrix4x4　1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceObj = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(Matrix4x4));
	//データを書き込む
	Matrix4x4* transformationMatrixDataObj = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceObj->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataObj));
	//単位行列を書き込んでおく
	*transformationMatrixDataObj = MakeIdentity4x4();


	//マテリアル用のリソースを作る。今回はcolor1つ分のサイズ用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceObj = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(Material));
	//マテリアルにデータを書き込む
	Material* materialDataObj = nullptr;
	//書き込むためのアドレスを取得
	materialResourceObj->Map(0, nullptr, reinterpret_cast<void**>(&materialDataObj));
	//今回は赤を書き込んでみる
	materialDataObj->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDataObj->enableLighting = true;
	materialDataObj->uvTransform = MakeIdentity4x4();


	//WVP用のリソースを作る。matrix4x4　1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResourceObj = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix));

	//データを読み込む
	TransformationMatrix* wvpDataObj = nullptr;
	//書き込むためのアドレスを取得
	wvpResourceObj->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataObj));
	//単位行列を書き込んでおく
	wvpDataObj->WVP = MakeIdentity4x4();



	int32_t kSubdivision = 16;
	uint32_t vertexCount = (kSubdivision + 1) * (kSubdivision + 1);
	uint32_t indexCount = kSubdivision * kSubdivision * 6;


	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceShere = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * vertexCount);
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(uint32_t) * indexCount);

	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewShere{};
	vertexBufferViewShere.BufferLocation = vertexResourceShere->GetGPUVirtualAddress();
	vertexBufferViewShere.SizeInBytes = sizeof(VertexData) * vertexCount;
	vertexBufferViewShere.StrideInBytes = sizeof(VertexData);
	


	VertexData* vertexDataShere = nullptr;
	vertexResourceShere->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataShere));

	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / float(kSubdivision);
	const float kLatEvery = std::numbers::pi_v<float> / float(kSubdivision);

	for (int lat = 0; lat <= kSubdivision; ++lat) {
		float latAngle = -std::numbers::pi_v<float> / 2.0f + kLatEvery * lat;
		for (int lon = 0; lon <= kSubdivision; ++lon) {
			float lonAngle = lon * kLonEvery;

			uint32_t index = lat * (kSubdivision + 1) + lon;

			float x = std::cosf(latAngle) * std::cosf(lonAngle);
			float y = std::sinf(latAngle);
			float z = std::cosf(latAngle) * std::sinf(lonAngle);

			vertexDataShere[index].position = { x, y, z, 1.0f };
			vertexDataShere[index].texcoord = {
				float(lon) / float(kSubdivision),
				1.0f - float(lat) / float(kSubdivision)
			};
			vertexDataShere[index].normal = { x, y, z };
		}
	}




	


	//shere用のtransformationMatrix用のリソースを作る。matrix4x4　1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceShere = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(Matrix4x4));
	//データを書き込む
	Matrix4x4* transformationMatrixDataShere = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceShere->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataShere));
	//単位行列を書き込んでおく
	*transformationMatrixDataShere = MakeIdentity4x4();


	

	
	
	//マテリアル用のリソースを作る。今回はcolor1つ分のサイズ用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(Material));
	//マテリアルにデータを書き込む
	Material* materialData = nullptr;
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は赤を書き込んでみる
	materialData->color = Vector4 (1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();


	//WVP用のリソースを作る。matrix4x4　1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix));

	//データを読み込む
	TransformationMatrix* wvpData = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//単位行列を書き込んでおく
	wvpData->WVP = MakeIdentity4x4();



	
	

	//平行光源用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> directionnalLightResource = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(DirectionalLight));
	//マテリアルにデータを書き込む
	DirectionalLight* directionnalLightData = nullptr;
	//書き込むためのアドレスを取得
	directionnalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionnalLightData));
	//白を書き込んでみる
	*directionnalLightData = {};
	directionnalLightData->color = { 1.0f, 1.0f, 1.0f ,1.0f};
	directionnalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionnalLightData->intensity = 1.0f;


	
	Microsoft::WRL::ComPtr<ID3D12Resource> indexSphereResource = dxCommon->CreateBufferResource(dxCommon->GetDevice(), sizeof(uint32_t) * indexCount);

	uint32_t* indexData = nullptr;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	uint32_t idx = 0;
	for (int lat = 0; lat < kSubdivision; ++lat) {
		for (int lon = 0; lon < kSubdivision; ++lon) {
			uint32_t i0 = lat * (kSubdivision + 1) + lon;
			uint32_t i1 = (lat + 1) * (kSubdivision + 1) + lon;
			uint32_t i2 = lat * (kSubdivision + 1) + (lon + 1);
			uint32_t i3 = (lat + 1) * (kSubdivision + 1) + (lon + 1);

			// 三角形1
			indexData[idx++] = i0;
			indexData[idx++] = i1;
			indexData[idx++] = i2;

			// 三角形2
			indexData[idx++] = i2;
			indexData[idx++] = i1;
			indexData[idx++] = i3;
		}
	}


	// インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * indexCount;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;






	

	Transform transform{ {1.0f, 1.0f, 1.0f},{0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	Transform transformObj{ {1.5f, 1.5f, 1.5f},{0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	Transform cameraTransform{ {1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,-10.0f} };

	Transform cameraTransformObj{ {1.0f, 1.0f, 1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,-10.0f} };

	/*Transform uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};*/
	

	
	

	

	
	//textureを読んで転送する
	DirectX::ScratchImage mipImages = dxCommon->LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon->UploadTextureData(textureResource, mipImages, dxCommon->GetDevice(), dxCommon->GetCommandList());

	////metadataを基にsrvの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);


	//2枚目のtextureを読んで転送する
	DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(dxCommon->GetDevice(), metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2 = dxCommon->UploadTextureData(textureResource2, mipImages2, dxCommon->GetDevice(), dxCommon->GetCommandList());

	////metadataを基にsrvの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	
	////関数化（descriptorSize)
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(1);

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 =  dxCommon->GetSRVGPUDescriptorHandle(2);
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 =  dxCommon->GetSRVCPUDescriptorHandle(2);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);
	//利用するheapの設定非常に特殊な運用
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;




	bool useMonsterBall = true;

	

	

	MSG msg{};



	while (msg.message != WM_QUIT)
	{
		if (winApp->ProcessMessage()) {
			break;
		} else {
			// ゲーム処理
			input->Update();
			sprite->Update();
			for (auto* sp : sprites)
			{
				sp->Update();
			}

			/*Sprite::Vector2 position = sprite->GetPosition();
			position.x += 0.1f;
			position.y += 0.1f;
			sprite->SetPosition(position);*/

			/*float rotation = sprite->GetRotation();
			rotation += 0.01f;
			sprite->SetRotation(rotation);*/

			Vector2 size = sprite->GetSize();
			size.x += 0.1f;
			size.y += 0.1f;
			sprite->SetSize(size);

			Vector4 color = sprite->GetColor();
			color.x += 0.01f;
			if (color.x > 1.0f)
			{
				color.x -= 1.0f;
			}
			sprite->SetColor(color);

			if (input->TriggerKey(DIK_0))
			{
				OutputDebugStringA("Hit 0\n");
			}
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			// 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
			ImGui::ShowDemoWindow();

			ImGui::Begin("Window");
			ImGui::ColorEdit3("color", &(materialData->color.x));
			ImGui::SliderFloat3("translateSprite", &transform.translate.x, 0.0f, 600.0f);
			ImGui::Checkbox("useMosterBall", &useMonsterBall);
			ImGui::SliderFloat3("Light", &directionnalLightData->direction.x, -1.0f, 0.8f);
			//ImGui::DragFloat2("uvTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			//ImGui::DragFloat2("uvcale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			//ImGui::SliderAngle("uvRotate", &uvTransformSprite.rotate.z);
			ImGui::SliderAngle("SphereRotate", &transform.rotate.y);
			ImGui::SliderAngle("SphereScale", &transform.scale.y);
			ImGui::SliderAngle("Spheretranslate", &transform.translate.y);
			ImGui::SliderAngle("ObjRotate", &transformObj.rotate.y);
			ImGui::SliderAngle("ObjScale", &transformObj.scale.y);
			ImGui::SliderAngle("Objtranslate", &transformObj.translate.y);




			directionnalLightData->direction = Normalize(directionnalLightData->direction);


			ImGui::End();


			//sp用
			Matrix4x4 worldMatrix = MakeAffineMatrix(
				transform.scale, transform.rotate, transform.translate);

			Matrix4x4 cameraMatrix =
				MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate,
					cameraTransform.translate);

			Matrix4x4 viewMatrix = Inverse(cameraMatrix);

			Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
				0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);

			Matrix4x4 worldViewProjectionMatrix =
				Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

			/*Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			materialDataSprite->uvTransform = uvTransformMatrix;*/


			wvpData->WVP = worldViewProjectionMatrix;
			wvpData->World = worldMatrix;


			//Obj用
			Matrix4x4 worldMatrixObj = MakeAffineMatrix(
				transformObj.scale, transformObj.rotate, transformObj.translate);

			Matrix4x4 cameraMatrixObj =
				MakeAffineMatrix(cameraTransformObj.scale, cameraTransformObj.rotate,
					cameraTransformObj.translate);

			Matrix4x4 viewMatrixObj = Inverse(cameraMatrixObj);

			Matrix4x4 projectionMatrixObj = MakePerspectiveFovMatrix(
				0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);

			Matrix4x4 worldViewProjectionMatrixObj =
				Multiply(worldMatrixObj, Multiply(viewMatrixObj, projectionMatrixObj));



			wvpDataObj->WVP = worldViewProjectionMatrixObj;
			wvpDataObj->World = worldMatrixObj;






					//// ImGuiの内部コマンドを生成する
					ImGui::Render();

					dxCommon->PreDraw();
					
					
					//  // PSOを設定
					
					// RootSignatureを設定。PSOに設定しているけど別途設定が必要
					dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
					dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
					dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewShere);
					dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferView);


					// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
					dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(
						0, materialResource->GetGPUVirtualAddress());
					// wvp用のCBufferの場所を設定
					dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(
						1, wvpResource->GetGPUVirtualAddress());
					// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
					dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
					//平行光源用CBufferの場所を設定
					dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(
						3, directionnalLightResource->GetGPUVirtualAddress());
					
					// 描画！（DrawCall/ドローコール）。3頂点で1つのインスタンス。インスタンスについては今後
					
					//dxCommon->GetCommandList()->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
								

					// RootSignatureを設定。PSOに設定しているけど別途設定が必要
					dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
					dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
					dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
					// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
					dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(
						0, materialResourceObj->GetGPUVirtualAddress());
					// wvp用のCBufferの場所を設定
					dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(
						1, wvpResourceObj->GetGPUVirtualAddress());
					// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
					dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
					
					//dxCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
					
					


				
					

					
					//共通描画設定
					spriteCommon->CommonRenderState();
					//sprite->Draw();
					for (auto* sp : sprites) 
					{
						sp->Draw();
					}

					// 実際のdxCommon->GetCommandList()のImGuiの描画コマンドを積む
					ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

					dxCommon->PostDraw();

				}

				
			}

			ImGui_ImplDX12_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();

			
			delete input;

			TextureManager::GetInstance()->finalize();

			delete dxCommon;
			
			winApp->Finalize();
			delete winApp;

			delete spriteCommon;
			delete sprite;
			

			for (auto* sp : sprites) 
			{
				delete sp;
			}
			






			//警告時に止まる
#ifdef _DEBUG
	//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
#endif
			return 0;

		}
	



