#pragma once
#include "DirectXCommon.h"

class SpriteCommon
{
public:

	void Initialize(DirectXCommon* dxCommon);

	DirectXCommon* GetDxCommon() const { return dxCommon_; }
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	DirectXCommon* dxCommon_;

	void RootSignature();

	void GraphicsPipeline();
};