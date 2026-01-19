#pragma once
#include <vector>
class SpriteCommon;
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
	
	void Initialize(SpriteCommon* spriteCommon);
	void Update();
private:
	SpriteCommon* spriteCommon = nullptr;

	DirectXCommon* dxCommon_;
	HRESULT hr;

	//sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;

	//Sprite用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;

	//sprite用のtransformationMatrix用のリソースを作る。matrix4x4　1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;

	//マテリアルにデータを書き込む
	Material* materialData = nullptr;
	VertexData* vertexData = nullptr;
	uint32_t* indexResourceData = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;
	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

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

	void CreateVertexData();
	void CreateMaterial();
	void CreateTransformMatrix()
};