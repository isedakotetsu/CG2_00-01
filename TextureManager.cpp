#include "TextureManager.h"

TextureManager* TextureManager::instance = nullptr;

void TextureManager::Initialize()
{
    textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

TextureManager* TextureManager::GetInstance()
{
    if (instance == nullptr)
    {
        instance = new TextureManager;
    }
    return instance;
}

void TextureManager::finalize()
{
    delete instance;
    instance = nullptr;
}

void TextureManager::LoadTexture(const std::string& filePath)
{
	auto it = std::find_if(textureDatas.begin(), textureDatas.end(),
		[&](TextureData& textureData) {return textureData.filePath == filePath; });
	if (it != textureDatas.end())
	{
		return;
	}
	//テクスチャファイルを読み込んでプログラムを扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));
	//ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));
	//テクスチャデータを追加
	textureDatas.resize(textureDatas.size() + 1);
	//追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas.back();
	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource =
		dxCommon_->CreateTextureResource(dxCommon_->GetDevice(), textureData.metadata);

	dxCommon_->UploadTextureData(
		textureData.resource,
		mipImages,
		dxCommon_->GetDevice(),
		dxCommon_->GetCommandList()
	);

	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1);

	textureData.srvHandleCPU = dxCommon_->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = dxCommon_->GetSRVGPUDescriptorHandle(srvIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


	dxCommon_->GetDevice()->CreateShaderResourceView(
		textureData.resource.Get(),
		&srvDesc,
		textureData.srvHandleCPU
	);



}
