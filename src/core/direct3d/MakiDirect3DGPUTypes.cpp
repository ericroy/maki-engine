#include "core/core_stdafx.h"
#include "core/direct3d/MakiDirect3DGPUTypes.h"
#include "core/direct3d/MakiDirect3DCommon.h"

namespace Maki
{
	
	GPUVertexShader::GPUVertexShader(uint32 vertexFormatsPerVertexShader)
		: blob(nullptr),
		blobSize(0),
		vs(nullptr),
		perFrameConstants(nullptr),
		perObjectConstants(nullptr),
		materialConstants(nullptr),
		inputLayouts(nullptr),
		inputLayoutCount(0),
		inputLayoutCapacity(vertexFormatsPerVertexShader)
	{
		inputLayouts = (InputLayout *)Allocator::Malloc(sizeof(InputLayout) * inputLayoutCapacity);
		memset(inputLayouts, 0, sizeof(InputLayout)*inputLayoutCapacity);
	}

	GPUVertexShader::~GPUVertexShader()
	{
		for(uint32 i = 0; i < inputLayoutCount; i++) {
			SAFE_RELEASE(inputLayouts[i].inputLayout);
		}
		SAFE_FREE(inputLayouts);

		SAFE_RELEASE(vs);
		SAFE_FREE(blob);
		SAFE_RELEASE(perFrameConstants);
		SAFE_RELEASE(perObjectConstants);
		SAFE_RELEASE(materialConstants);
	}

	ID3D11InputLayout *GPUVertexShader::GetOrCreateInputLayout(ID3D11Device *device, const VertexFormat *vf) {
		for(uint32 i = 0; i < inputLayoutCount; i++) {
			if(inputLayouts[i].vertexFormatKey == vf->equalityKey) {
				return inputLayouts[i].inputLayout;
			}
		}

		// Else create this layout and cache it
		assert(inputLayoutCount < inputLayoutCapacity && "Too many vertex formats for this vertex shader");

		uint32 offset = 0;
		uint32 index = 0;
		D3D11_INPUT_ELEMENT_DESC layoutData[VertexFormat::AttributeCount];
		for(uint8 i = 0; i < VertexFormat::AttributeCount; i++) {
			VertexFormat::Attribute attr = (VertexFormat::Attribute)i;
			if(vf->HasAttribute(attr)) {

				VertexFormat::DataType dataType = vf->GetDataType(attr);
				uint8 dataCount = vf->GetDataCount(attr);

				layoutData[index].SemanticName = attrToSemanticString[attr];
				layoutData[index].SemanticIndex = attrToSemanticIndex[attr];
				layoutData[index].Format = typeAndCountToFormat[dataType][dataCount];
				layoutData[index].InputSlot = 0;
				layoutData[index].AlignedByteOffset = offset;
				layoutData[index].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				layoutData[index].InstanceDataStepRate = 0;

				offset += dataCount * VertexFormat::DataTypeSizes[dataType];
				index++;
			}
		}

		if(MAKI_D3D_FAILED(device->CreateInputLayout(layoutData, index, blob, blobSize, &inputLayouts[inputLayoutCount].inputLayout))) {
			Console::Error("Failed to create input layout");
			return nullptr;
		}
		inputLayouts[inputLayoutCount].vertexFormatKey = vf->equalityKey;
		inputLayoutCount++;
		return inputLayouts[inputLayoutCount].inputLayout;
	}
		
			




	GPUPixelShader::GPUPixelShader()
		: ps(nullptr),
		perFrameConstants(nullptr),
		perObjectConstants(nullptr),
		materialConstants(nullptr)
	{
	}

	GPUPixelShader::~GPUPixelShader()
	{
		SAFE_RELEASE(ps);
		SAFE_RELEASE(perFrameConstants);
		SAFE_RELEASE(perObjectConstants);
		SAFE_RELEASE(materialConstants);
	}
			




	GPUTexture::GPUTexture()
		: shaderResourceView(nullptr),
		samplerState(nullptr),
		depthStencilView(nullptr),
		renderTargetView(nullptr)
	{
	}

	GPUTexture::~GPUTexture()
	{
		SAFE_RELEASE(shaderResourceView);
		SAFE_RELEASE(samplerState);
		SAFE_RELEASE(depthStencilView);
		SAFE_RELEASE(renderTargetView);
	}


} // namespace Maki