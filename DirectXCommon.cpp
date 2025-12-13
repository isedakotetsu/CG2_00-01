#include "DirectXCommon.h"
#include <cassert>


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")


void DirectXCommon::Initialize(WinApp* winApp)
{
	assert(winApp);
	this->winApp = winApp;
	InitializeFixFPS();
	deviceInitialize();
	CommandInitialize();
	SwapChains();
	CreatedsvDescriptorHeap();
	RtvInitialize();
	DepthStencilViewInitialize();
	fenceInitialize();
	viewportInitialize();
	scissorRectInitialize();
	CreatedxcCompiler();
	ImGuiInitialize();
}

void DirectXCommon::deviceInitialize()
{
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		//デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif


	
	//HRESULTはWindows系のエラーコードであり
	//関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか
	//どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));




	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;

	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i)
	{

		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//Logger::Log(logstream, StringUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}

	assert(useAdapter != nullptr);

	



	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};

	const char* featureLevelString[] = { "12.2", "12.1", "12.0" };

	for (size_t i = 0; i < _countof(featureLevels); ++i)
	{
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));

		if (SUCCEEDED(hr))
		{
			//Logger::Log(logstream, std::format("FeatureLevel : {}\n", featureLevelString[i]));
			break;
		}
	}

	assert(device != nullptr);
	//Logger::Log(logstream, "complete create D3D12Device!!!\n");
#ifdef _DEBUG
	    ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] =
		{
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		infoQueue->PushStorageFilter(&filter);
		//解放
		infoQueue->Release();
	}
#endif
}

void DirectXCommon::CommandInitialize()
{
	
	HRESULT hr;

	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(hr));


	

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(hr));


	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::SwapChains()
{
	HRESULT hr;
	
	swapChainDesc.Width = WinApp::kClientWidth;//画面の幅
	swapChainDesc.Height = WinApp::kClientHeight;//画面の高さ
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタにうつしたら、中身を
	//コマンドキュー、ウィンドハンドル、設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));
}



void DirectXCommon::InitializeFixFPS()
{
	reference_ = std::chrono::steady_clock::now();
}

void DirectXCommon::UpdateFixFPS()
{
	//1/60秒ぴったりの時間
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
	//1/60秒よりわずかに短い時間
	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));
	//現在時間を取得する
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	//前回記録からの経過時間を取得する
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	//1/60秒（よりわずかに短い時間）経っていない場合
	if (elapsed < kMinCheckTime)
	{
		//1/60秒経過するまで微小なスリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
	reference_ = std::chrono::steady_clock::now();

}

ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap( D3D12_DESCRIPTOR_HEAP_TYPE heaptype, UINT numDescriptors, bool shaderVisible)
{
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heaptype;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

void DirectXCommon::DepthStencilViewInitialize()
{
	ComPtr<ID3D12Resource> depthStencilresource = CreateDepthStencilTextureResource(device, WinApp::kClientWidth, WinApp::kClientHeight);


	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//format基本的にresourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2dTexture
	//DSVHeapの先頭にDSVを作る
	device->CreateDepthStencilView(depthStencilresource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

}
void DirectXCommon::fenceInitialize()
{
	HRESULT hr;
	//初期値0でfenceを作る
	uint64_t fenceValue = 0;

	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	//fenceのsignalを待つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);

	
}
void DirectXCommon::viewportInitialize()
{
	
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = WinApp::kClientWidth;
	viewport.Height = WinApp::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}
void DirectXCommon::scissorRectInitialize()
{
	
	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = WinApp::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kClientHeight;
}
void DirectXCommon::CreatedxcCompiler()
{
	HRESULT hr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
}
void DirectXCommon::ImGuiInitialize()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}
void DirectXCommon::PreDraw()
{
	//// これから書き込むバックバッファのインデックスを取得
					UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

					// 今回のバリアはTransitioin
					barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					// Noneにしておく
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					// バリアを張る対象のリソース。現在のバックバッファに対して行う
					barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
					// 遷移前（現在）のResouceState
					barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
					// 遷移後のResourceState
					barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
					// TransitionBarrieを張る
					GetCommandList()->ResourceBarrier(1, &barrier);

					// 描画先のRTVを設定する
					GetCommandList()->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false,
						nullptr);

					// 描画先のRTVとDSVを設定する
					D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
						dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

					// 指定した色で画面全体をクリアする
					float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f }; // 青っぽい色。RGBAの順

					GetCommandList()->ClearRenderTargetView(rtvHandles[backBufferIndex],
						clearColor, 0, nullptr);

					// 指定した深度で画面全体をクリアする
					GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH,
						1.0f, 0, 0, nullptr);

					GetCommandList()->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false,
						&dsvHandle);

					// 描画用のDescriptorHeapの設定
					Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvDescriptorHeap };
					GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

					GetCommandList()->RSSetViewports(1, &viewport);       // Viewportを設定
					GetCommandList()->RSSetScissorRects(1, &scissorRect); // Scirssorを設定
}
void DirectXCommon::PostDraw()
{
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	//// 今回はRenderTargetからPresentにする
					barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
					barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
					// TransitionBarrierを張る
					GetCommandList()->ResourceBarrier(1, &barrier);

					HRESULT hr;

					// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
					hr = GetCommandList()->Close();
					assert(SUCCEEDED(hr));

					ID3D12CommandList* lists[] = { GetCommandList() };
					commandQueue->ExecuteCommandLists(_countof(lists), lists);

				

					// GPUとOSに画面の交換を行うよう通知する
					swapChain->Present(1, 0);

					// Fenceの値を更新
					fenceValue++;
					// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
					commandQueue->Signal(fence.Get(), fenceValue);

					// Fenceの値が指定したSignal値にたどり着いてるか確認する
					// GetCompletedValueの初期化はFence作成時に渡した初期化
					if (fence->GetCompletedValue() < fenceValue) {
						// 指定したSignalにたどりついていないので、たどり着くまで待つようにイベントを設定する
						fence->SetEventOnCompletion(fenceValue, fenceEvent);
						// イベントを待つ
						WaitForSingleObject(fenceEvent, INFINITE);
					}

					UpdateFixFPS();

					// 次のフレーム用のコマンドリストを準備
					hr = commandAllocator->Reset();
					assert(SUCCEEDED(hr));
					hr = GetCommandList()->Reset(commandAllocator.Get(), nullptr);
					assert(SUCCEEDED(hr));
}
void DirectXCommon::CreatedsvDescriptorHeap()
{



	 desriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	 desriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	 desriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//ディスクリプタヒープの生成 shader内で触れるものではない
	rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	//触れるもの

	srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	//DSV用のヒープでディスクリプタの数は１。DSVはshader内で触る物ではないのでshaderVisibleはfalse
	dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	

}



D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(srvDescriptorHeap, desriptorSizeSRV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(srvDescriptorHeap, desriptorSizeSRV, index);
}



ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource(ComPtr<ID3D12Device>& device, int32_t width, int32_t height)
{
	//生成するresourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;//textureの幅
	resourceDesc.Height = height;//textureの高さ
	resourceDesc.MipLevels = 1;//mipmapの数
	resourceDesc.DepthOrArraySize = 1;//奥行き　or　配列textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//depthstencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1;//サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//depthstencilとして使う通知

	//利用するheapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//Vram上に作る

	//深度地のCLEAR設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;//1.0f(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット。resourceと合わせる


	//Resourceの生成
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,//heapの設定
		D3D12_HEAP_FLAG_NONE,//heapの特殊設定。特になし
		&resourceDesc,//resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度地を書き込む状態にしておく
		&depthClearValue,//clear最適値
		IID_PPV_ARGS(&resource));//作成するresourceポインタへのポインタ
	assert(SUCCEEDED(hr));

	return resource;
}

ComPtr<IDxcBlob> DirectXCommon::CompileShader(//compilerするshaderファイルへのバス
	const std::wstring& filePath,
	//compiler使用するprofile
	const wchar_t* profile,
	//初期化で生成したものを3つ
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler,
	std::ostream& os)
{
	//これからシェーダーをコンパイルする旨をログに出す
	Logger::Log(os, StringUtility::ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
	//hlslファイルを読み込む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	//読めなかったら止める
	assert(SUCCEEDED(hr));
	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;

	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();

	shaderSourceBuffer.Size = shaderSource->GetBufferSize();

	shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8の文字コードであることを通知

	LPCWSTR arguments[] = {
		filePath.c_str(),//コンパイル対象のhlslファイル名
		L"-E", L"main",//エントリーポイントの指定。基本的にmain以外にはしない
		L"-T", profile,//shaderProfileの設定
		L"-Zi", L"-Qembed_debug",//デバッグ用の情報を埋め込む
		L"-Od",//最適化を外しておく
		L"-Zpr",//メモリレイアウトは行優先

	};
	//実際にshaderをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,//読み込んだファイル
		arguments,//コンパイルオプション
		_countof(arguments),//コンパイルオプションの数
		includeHandler,//includeが含まれた
		IID_PPV_ARGS(&shaderResult)//コンパイル結果

	);
	//コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
	{
		Logger::Log(os, shaderError->GetStringPointer());
		//警告。エラーダメ絶対
		assert(false);
	}
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したろぐをだす
	Logger::Log(os, StringUtility::ConvertString(std::format(L"Comile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	//もう使わないリソースを解放
	shaderSource->Release();
	shaderResult->Release();
	//実行用のバイナリを返却
	return shaderBlob;
}

ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& Device, size_t sizeInBytes)
{
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//uploadHeapを使う
	//頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	//バッファリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;//リソースのサイズ今回はvector4を3頂点分
	//バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	HRESULT hr = Device.Get()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	return vertexResource;
}

ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& Device, const DirectX::TexMetadata& metadata)
{
	//metdataを基にresourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);//textureの幅
	resourceDesc.Height = UINT(metadata.height);//textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);//mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);//奥行or配列textureの配列数
	resourceDesc.Format = metadata.format;//textureのformat
	resourceDesc.SampleDesc.Count = 1;//サンプリングカウント。１固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);//textureの次元数。普段使っているのは二次元

	//利用するHeapの設定。非常に特殊な運用。02-04exで一般的なケース版がある
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//細かい設定を行う
	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//writebackポリシーでcpuアクセス可能
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//プロセッサの近くに配置

	//resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = Device.Get()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;


}

ComPtr<ID3D12Resource> DirectXCommon::UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages, const Microsoft::WRL::ComPtr<ID3D12Device>& Device, const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& CommandList)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(Device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(Device.Get(), intermediateSize);
	UpdateSubresources(CommandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	CommandList->ResourceBarrier(1, &barrier);
	return intermediateResource;
}

DirectX::ScratchImage DirectXCommon::LoadTexture(const std::string& filePath)
{
	//テクスチャファイルを読み込んでプログラムを扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));
	//ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));
	//みっぷマップ付きのデータを返す
	return mipImages;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

void DirectXCommon::RtvInitialize()
{
	HRESULT hr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	//rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//レンダーターゲットビュー用
	//rtvDescriptorHeapDesc.NumDescriptors = 2;//ダブルバッファ用に2つ多くても構わない
	//hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	////ディスクリプタヒープが作れなかったので起動できない
	//assert(SUCCEEDED(hr));
	//swapchainからresourceを引っ張ってくる
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));

	assert(SUCCEEDED(hr));
	//RTVの設定
	
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2Dテクスチャとして書き込む
	//ディスクリプタの戦闘を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStarHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (uint32_t i = 0; i < 2; ++i)
	{
		
		//まずは一つ目を作る、一つ目は最初のところに作る、作る場所をこちらで指定してあげる必要がある
		rtvHandles[0] = rtvStarHandle;
		device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);

		//二つ目はディスクリプタハンドルを得る
		rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		//二つ目を作る
		device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);
	}
}
