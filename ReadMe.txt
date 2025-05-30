特に加点要素は追加していません
//meta情報を取得
const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
//ぜんmipmapについて
for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel)
{
	
	//mipmaplevelを指定して各imageを取得
	const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
	//textureに転送
	HRESULT hr = texture->WriteToSubresource(
		UINT(mipLevel),
		nullptr,//全領域コピー
		img->pixels,//元データアドレス
		UINT(img->rowPitch),//1lineサイズ
		UINT(img->slicePitch)//1枚サイズ
	);
	assert(SUCCEEDED(hr));
}