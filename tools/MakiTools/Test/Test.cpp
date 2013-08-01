// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "dxguid")

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{

	ID3D10Blob *codeBlob = nullptr;
	ID3D10Blob *errorMessages = nullptr;
	if(FAILED(D3DCompileFromFile(L"../../../../assets/shaders/pos_norm/pos_norm.vs", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_4_0", 0, 0, &codeBlob, &errorMessages))) {
		cout << string((char *)errorMessages->GetBufferPointer(), errorMessages->GetBufferSize()) << endl;
		return 1;
	}

	
	// Use reflection to analyze the layout of the cbuffers
	ID3D11ShaderReflection *reflect = nullptr; 
	if(FAILED(D3DReflect(codeBlob->GetBufferPointer(), codeBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflect))) {
		printf("Failed to reflect upon shader blob\n");
		return 1;
	}

	D3D11_SHADER_DESC desc;
	if(FAILED(reflect->GetDesc(&desc))) {
		printf("Failed to get shader_desc from reflection interface\n");
		return 1;
	}


	return 0;
}

