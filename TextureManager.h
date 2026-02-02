#pragma once
#include <string>       
#include "DirectXCommon.h"    
#include <wrl.h>
#include <vector>
#include <d3d12.h>        
#include <dxgi1_6.h>      
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
class TextureManager
{
public:
	static TextureManager* GetInstance();
	void Initialize(DirectXCommon*dxCommon);
	//終了
	void finalize();

	void LoadTexture(const std::string& filePath);

	void ReleaseAllUploadResources();

	uint32_t GetTextureIndexByFilePath(const std::string& filePath);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

	const DirectX::TexMetadata& GetMetaData(uint32_t textureIndex);

private:
	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

	DirectXCommon* dxCommon_;

	struct TextureData 
	{
		std::string filePath;
		DirectX::TexMetadata metadata{};
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadResource;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU{};
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU{};
		uint32_t srvIndex = 0; 
	};


	std::vector<TextureData> textureDatas;

	static uint32_t kSRVIndexTop;

	
	


};