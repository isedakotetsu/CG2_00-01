#include "Sprite.h"
#include "SpriteCommon.h"

void Sprite::Initialize(SpriteCommon* spriteCommon)
{
	this->spriteCommon = spriteCommon;

	dxCommon_ = spriteCommon->GetDxCommon();  
	assert(dxCommon_);

	CreateVertexData();
	CreateMaterial();
	CreateTransformMatrix();

	textureSrvHandleGPU = dxCommon_->GetSRVGPUDescriptorHandle(1);
}


void Sprite::Update()
{
	transformSprite.translate = { position.x, position.y, 0.0f };
	transformSprite.rotate = { 0.0f,0.0f, rotation };

	//一枚目の三角形
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	assert(SUCCEEDED(hr));
	vertexData[0].position = { 0.0f, 360.0f, 0.0f, 1.0f };//左下
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };
	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };//左上
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[2].position = { 640.0f, 360.0f, 0.0f, 1.0f };//右下
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[3].position = { 640.0f, 0.0f, 0.0f, 1.0f };//右上
	vertexData[3].texcoord = { 1.0f, 0.0f };

	
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexResourceData));
	assert(SUCCEEDED(hr));
	indexResourceData[0] = 0;
	indexResourceData[1] = 1;
	indexResourceData[2] = 2;
	indexResourceData[3] = 1;
	indexResourceData[4] = 3;
	indexResourceData[5] = 2;

	// Sprite用のWorldViewProjectionMatrixを作る
	Matrix4x4 worldMatrixSprite =
		MakeAffineMatrix(transformSprite.scale, transformSprite.rotate,
			transformSprite.translate);
	Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
	Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(
		0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrixSprite =
		Multiply(worldMatrixSprite,
			Multiply(viewMatrixSprite, projectionMatrixSprite));

	transformationMatrixData->World = worldMatrixSprite;
	transformationMatrixData->WVP = worldViewProjectionMatrixSprite;

	
}

void Sprite::Draw()
{
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // WBVを設定
	dxCommon_->GetCommandList()->IASetIndexBuffer(&indexBufferView);//IBVを設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(
		0, materialResource->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(
		1, transformationMatrixResource->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
	// 描画！（DrawCall/ドローコール)
	dxCommon_->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

}

void Sprite::CreateVertexData()
{
	vertexResource = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * 6);
	indexResource = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * 6);

	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点6つ文のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	//1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	//リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス６つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	
}

void Sprite::CreateMaterial()
{
	materialResource = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//白を書き込んでみる
	*materialData = {};
	materialData->color = Vector4{ 1.0f, 1.0f, 1.0f,1.0f };
	materialData->enableLighting = 1;
	materialData->uvTransform = MakeIdentity4x4();

	materialData->enableLighting = false;



}

void Sprite::CreateTransformMatrix()
{
	transformationMatrixResource = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));

	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	//単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
}
