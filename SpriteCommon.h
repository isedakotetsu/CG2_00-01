#pragma once
#include "DirectXCommon.h"

class SpriteCommon
{
public:

	void Initialize(DirectXCommon* dxCommon);

	void CommonRenderState();


	DirectXCommon* GetDxCommon() const { return dxCommon_; }

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	DirectXCommon* dxCommon_;
	HRESULT hr;



	void RootSignature();

	void GraphicsPipeline();
};