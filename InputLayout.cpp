#include "InputLayout.h"

D3D12_INPUT_LAYOUT_DESC InputLayout::CreateInputLayout3D()
{
	inputElementDescs3D_[0].SemanticName = "POSITION";
	inputElementDescs3D_[0].SemanticIndex = 0;
	inputElementDescs3D_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs3D_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs3D_[1].SemanticName = "TEXCOORD";
	inputElementDescs3D_[1].SemanticIndex = 0;
	inputElementDescs3D_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs3D_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs3D_[2].SemanticName = "NORMAL";
	inputElementDescs3D_[2].SemanticIndex = 0;
	inputElementDescs3D_[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs3D_[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputLayoutDesc3D_.pInputElementDescs = inputElementDescs3D_;
	inputLayoutDesc3D_.NumElements = _countof(inputElementDescs3D_);

    return inputLayoutDesc3D_;
}

D3D12_INPUT_LAYOUT_DESC InputLayout::CreateInputLayout2D()
{
	inputElementDescs2D_[0].SemanticName = "POSITION";
	inputElementDescs2D_[0].SemanticIndex = 0;
	inputElementDescs2D_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs2D_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs2D_[1].SemanticName = "TEXCOORD";
	inputElementDescs2D_[1].SemanticIndex = 0;
	inputElementDescs2D_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs2D_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputLayoutDesc2D_.pInputElementDescs = inputElementDescs2D_;
	inputLayoutDesc2D_.NumElements = _countof(inputElementDescs2D_);

	return inputLayoutDesc2D_;
}
