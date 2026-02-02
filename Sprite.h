#pragma once
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>
#include <cmath>
#include "Vector.h"
#include <string>
class SpriteCommon;
class DirectXCommon;
class Sprite
{
public:
	struct Matrix4x4
	{
		float m[4][4];
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
	struct Transform
	{
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

	void Initialize(SpriteCommon* spriteCommon, std::string textureFilePath);
	void Update();
	void Draw();
	//座標
	const Vector2& GetPosition() const { return position; }
	void SetPosition(const Vector2& position) { this->position = position; }

	//回転
	float GetRotation() const { return rotation; }
	void SetRotation(float rotation) { this->rotation = rotation; }
	//色
	const Vector4& GetColor() const { return materialData->color; }
	void SetColor(const Vector4& color) { materialData->color = color; }

	//サイズ
	const Vector2& GetSize() const { return size; }
	void SetSize(const Vector2& size) { this->size = size; }
	//アンカーポイント
	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }
	const Vector2& GetAnchorPoint() const { return anchorPoint; }
	//フリップ
	bool getFlipX() const {return isFlipX_;}
	void setFlipX(bool isFlipX) { this->isFlipX_ = isFlipX; }

	bool getFlipY() const {return isFlipY_;}

	void setFlipY(bool isFlipY) {this->isFlipY_ = isFlipY;}

	void SetTextureLeftTop(const Vector2& textureLeftTop) { this->textureLeftTop = textureLeftTop; }
	const Vector2& GetTextureLeftTop() const { return textureLeftTop; }

	void SettextureSize(const Vector2& textureSize) { this->textureSize = textureSize; }
	const Vector2& GetTextureSize() const { return textureSize; }




private:
	SpriteCommon* spriteCommon = nullptr;


	DirectXCommon* dxCommon_;
	HRESULT hr;

	//sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;

	//Sprite用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource{};

	//sprite用のtransformationMatrix用のリソースを作る。matrix4x4　1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource{};

	//マテリアルにデータを書き込む
	Material* materialData = nullptr;
	VertexData* vertexData = nullptr;
	uint32_t* indexResourceData = { nullptr };
	TransformationMatrix* transformationMatrixData = nullptr;
	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU{};

	Matrix4x4 MakeIdentity4x4() 
	{
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

	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2)
	{
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

	Transform transformSprite{ { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

	Vector2 position = { 0.0f, 0.0f };
	float rotation = 0.0f;

	Vector2 size = { 640.0f, 360.0f };

	uint32_t textureIndex = 0;

	Vector2 anchorPoint = { 0.5f, 0.5f };

	Vector2 textureLeftTop = { 0.0f, 0.0f };
	Vector2 textureSize = { 64.0f, 64.0f };

	bool isFlipX_ = false;
	bool isFlipY_ = false;

	

	void AdjustTextureSize();
	void CreateVertexData();
	void CreateMaterial();
	void CreateTransformMatrix();
};