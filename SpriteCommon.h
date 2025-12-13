#pragma once
#include "DirectXCommon.h"

class SpriteCommon
{
public:

	void Initialize(DirectXCommon* dxCommon);

	DirectXCommon* GetDxCommon() const { return dxCommon_; }
private:

	DirectXCommon* dxCommon_;

	void RootSignature();

	void GraphicsPipeline();
};