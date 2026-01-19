#include "SpriteCommon.h"
#include <fstream>

void SpriteCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;

	GraphicsPipeline();
}


void SpriteCommon::RootSignature()
{
	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignatureSprite{};
	descriptionRootSignatureSprite.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRangeSprite[1] = {};
	descriptorRangeSprite[0].BaseShaderRegister = 0;//0から始まる
	descriptorRangeSprite[0].NumDescriptors = 1;//数は一つ
	descriptorRangeSprite[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRangeSprite[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//offsetを自動計算


	//RootParameter作成。複数設定できるので配列。今回は結果１つだけなので長さ１の配列
	D3D12_ROOT_PARAMETER rootParametersSprite[4] = {};
	rootParametersSprite[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParametersSprite[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParametersSprite[0].Descriptor.ShaderRegister = 0;//レジスタ番号０とバインド
	rootParametersSprite[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParametersSprite[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderで使う
	rootParametersSprite[1].Descriptor.ShaderRegister = 0;//レジスタ番号を使う
	rootParametersSprite[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//descriptorTableを使う
	rootParametersSprite[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//pixelshaderで使う
	rootParametersSprite[2].DescriptorTable.pDescriptorRanges = descriptorRangeSprite;//tableの中身の配列を指定
	rootParametersSprite[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeSprite);//tableで利用する数
	rootParametersSprite[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParametersSprite[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//pixelshdaderで使う
	rootParametersSprite[3].Descriptor.ShaderRegister = 1;//レジスタ番号1を使う
	descriptionRootSignatureSprite.pParameters = rootParametersSprite;//ルートパラメータ配列へのポインタ
	descriptionRootSignatureSprite.NumParameters = _countof(rootParametersSprite);//配列の長さ

	//samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplersSprite[1] = {};
	staticSamplersSprite[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplersSprite[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0-1の範囲外をリピート
	staticSamplersSprite[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplersSprite[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplersSprite[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplersSprite[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのmipmapを使う
	staticSamplersSprite[0].ShaderRegister = 0;//レジスタ番号0を使う
	staticSamplersSprite[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//pixelShaderで使う
	descriptionRootSignatureSprite.pStaticSamplers = staticSamplersSprite;
	descriptionRootSignatureSprite.NumStaticSamplers = _countof(staticSamplersSprite);







	//シリアライズしてバイナリする
	ID3DBlob* signatureBlobSprite = nullptr;
	ID3DBlob* errorBlobSprite = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignatureSprite,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlobSprite, &errorBlobSprite);
	if (FAILED(hr))
	{
		//Logger::Log(logstream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);

	}
	//バイナリをもとに生成
	hr = dxCommon_->GetDevice()->CreateRootSignature(0,
		signatureBlobSprite->GetBufferPointer(), signatureBlobSprite->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
}

void SpriteCommon::GraphicsPipeline()
{
	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignatureSprite{};
	descriptionRootSignatureSprite.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRangeSprite[1] = {};
	descriptorRangeSprite[0].BaseShaderRegister = 0;//0から始まる
	descriptorRangeSprite[0].NumDescriptors = 1;//数は一つ
	descriptorRangeSprite[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//SRVを使う
	descriptorRangeSprite[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;//offsetを自動計算


	//RootParameter作成。複数設定できるので配列。今回は結果１つだけなので長さ１の配列
	D3D12_ROOT_PARAMETER rootParametersSprite[4] = {};
	rootParametersSprite[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParametersSprite[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParametersSprite[0].Descriptor.ShaderRegister = 0;//レジスタ番号０とバインド
	rootParametersSprite[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParametersSprite[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderで使う
	rootParametersSprite[1].Descriptor.ShaderRegister = 0;//レジスタ番号を使う
	rootParametersSprite[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//descriptorTableを使う
	rootParametersSprite[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//pixelshaderで使う
	rootParametersSprite[2].DescriptorTable.pDescriptorRanges = descriptorRangeSprite;//tableの中身の配列を指定
	rootParametersSprite[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeSprite);//tableで利用する数
	rootParametersSprite[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParametersSprite[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//pixelshdaderで使う
	rootParametersSprite[3].Descriptor.ShaderRegister = 1;//レジスタ番号1を使う
	descriptionRootSignatureSprite.pParameters = rootParametersSprite;//ルートパラメータ配列へのポインタ
	descriptionRootSignatureSprite.NumParameters = _countof(rootParametersSprite);//配列の長さ

	//samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplersSprite[1] = {};
	staticSamplersSprite[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplersSprite[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0-1の範囲外をリピート
	staticSamplersSprite[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplersSprite[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplersSprite[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplersSprite[0].MaxLOD = D3D12_FLOAT32_MAX;//ありったけのmipmapを使う
	staticSamplersSprite[0].ShaderRegister = 0;//レジスタ番号0を使う
	staticSamplersSprite[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//pixelShaderで使う
	descriptionRootSignatureSprite.pStaticSamplers = staticSamplersSprite;
	descriptionRootSignatureSprite.NumStaticSamplers = _countof(staticSamplersSprite);







	//シリアライズしてバイナリする
	ID3DBlob* signatureBlobSprite = nullptr;
	ID3DBlob* errorBlobSprite = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignatureSprite,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlobSprite, &errorBlobSprite);
	if (FAILED(hr))
	{
		//Logger::Log(logstream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);

	}
	//バイナリをもとに生成
	hr = dxCommon_->GetDevice()->CreateRootSignature(0,
		signatureBlobSprite->GetBufferPointer(), signatureBlobSprite->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

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

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescsSprite[3] = {};
	inputElementDescsSprite[0].SemanticName = "POSITION";
	inputElementDescsSprite[0].SemanticIndex = 0;
	inputElementDescsSprite[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescsSprite[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescsSprite[1].SemanticName = "TEXCOORD";
	inputElementDescsSprite[1].SemanticIndex = 0;
	inputElementDescsSprite[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescsSprite[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescsSprite[2].SemanticName = "NORMAL";
	inputElementDescsSprite[2].SemanticIndex = 0;
	inputElementDescsSprite[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescsSprite[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDescSprite{};
	inputLayoutDescSprite.pInputElementDescs = inputElementDescsSprite;
	inputLayoutDescSprite.NumElements = _countof(inputElementDescsSprite);

	//blendstateの設定
	D3D12_BLEND_DESC blendDescSprite{};
	//全ての色要素を書き込む
	blendDescSprite.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//ResiterzeStateの設定
	D3D12_RASTERIZER_DESC rasterizerDescSprite{};
	//裏面（時計回り）を表示しない
	rasterizerDescSprite.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDescSprite.FillMode = D3D12_FILL_MODE_SOLID;




	

	IDxcUtils* dxcUtilsSprite = nullptr;
	IDxcCompiler3* dxcCompilerSprite = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtilsSprite));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompilerSprite));
	assert(SUCCEEDED(hr));

	IDxcIncludeHandler* includeHandlerSprite = nullptr;
	hr = dxcUtilsSprite->CreateDefaultIncludeHandler(&includeHandlerSprite);
	assert(SUCCEEDED(hr));

	//Shaderをコンパイルする
	ComPtr<IDxcBlob> vertexShaderBlobSprite = dxCommon_->CompileShader(L"resources/shaders/Object3D.VS.hlsl",
		L"vs_6_0", dxcUtilsSprite, dxcCompilerSprite, includeHandlerSprite, logstream);
	assert(vertexShaderBlobSprite != nullptr);

	ComPtr<IDxcBlob> pixelShaderBlobSprite = dxCommon_->CompileShader(L"resources/shaders/Object3D.PS.hlsl",
		L"ps_6_0", dxcUtilsSprite, dxcCompilerSprite, includeHandlerSprite, logstream);
	assert(pixelShaderBlobSprite != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPirelineStateDescSprite{};
	graphicsPirelineStateDescSprite.pRootSignature = rootSignature.Get();//rootsignature
	graphicsPirelineStateDescSprite.InputLayout = inputLayoutDescSprite;//InputLayout
	graphicsPirelineStateDescSprite.VS = { vertexShaderBlobSprite->GetBufferPointer(),
	vertexShaderBlobSprite->GetBufferSize() };//vetexShader
	graphicsPirelineStateDescSprite.PS = { pixelShaderBlobSprite->GetBufferPointer(),
	pixelShaderBlobSprite->GetBufferSize() };//PixelShader
	graphicsPirelineStateDescSprite.BlendState = blendDescSprite;//BlendState
	graphicsPirelineStateDescSprite.RasterizerState = rasterizerDescSprite;//RasterizerState
	//書き込むRTVの情報
	graphicsPirelineStateDescSprite.NumRenderTargets = 1;
	graphicsPirelineStateDescSprite.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ（形状）のタイプ。三角形
	graphicsPirelineStateDescSprite.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定（気にしなくてもよい)
	graphicsPirelineStateDescSprite.SampleDesc.Count = 1;
	graphicsPirelineStateDescSprite.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//depthstencilstateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDescSprite{};
	//depthの機能を有効化する
	depthStencilDescSprite.DepthEnable = true;
	//書き込みします
	depthStencilDescSprite.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はlessequal。つまり近ければ描画される
	depthStencilDescSprite.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//depthStencilの設定
	graphicsPirelineStateDescSprite.DepthStencilState = depthStencilDescSprite;
	graphicsPirelineStateDescSprite.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//実際に生成
	
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPirelineStateDescSprite,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}

void SpriteCommon::CommonRenderState()
{
	// マテリアルCBufferの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}