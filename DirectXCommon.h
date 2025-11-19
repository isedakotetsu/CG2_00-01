#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"
#include <array>
#include <dxcapi.h>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "Logger.h"
#include "StringUtility.h"
#include <format>

using namespace Microsoft::WRL;

class DirectXCommon
{
public:
	
	void Initialize(WinApp* winApp);

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);
	ID3D12Device* GetDevice() const { return device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }
	
private:
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heaptype, UINT numDescriptors, bool shaderVisible);
	ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ComPtr<ID3D12Device>& device, int32_t width, int32_t height);
	ComPtr<ID3D12Device> device;

	ComPtr<IDXGIFactory7> dxgiFactory;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	ComPtr<IDXGISwapChain4> swapChain = nullptr;
	ComPtr<ID3D12Resource> resource = nullptr;
	WinApp* winApp = nullptr;
	
	uint32_t desriptorSizeSRV;
	uint32_t desriptorSizeRTV;
	uint32_t desriptorSizeDSV;

	//ディスクリプタヒープの生成 shader内で触れるものではない
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	//触れるもの									 
												  
	ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
												  
	//DSV用のヒープでディスクリプタの数は１。DSVは
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	std::array<ComPtr<ID3D12Resource>,2> swapChainResources;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	

	void deviceInitialize();
	void CommandInitialize();
	void SwapChains();
	void RtvInitialize();
	void CreatedsvDescriptorHeap();
	void DepthStencilViewInitialize();
	void fenceInitialize();
	void viewportInitialize();
	void scissorRectInitialize();
	void CreatedxcCompiler();
	void ImGuiInitialize();
};
