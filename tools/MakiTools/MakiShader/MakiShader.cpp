#include "core/MakiDocument.h"
#include "core/MakiBase64.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "dxguid")

using namespace std;
using namespace Maki;

bool Compile(Document::Node *shaderNode, const char *filePath, const char *compTarget, const char *entryPoint, const vector< pair<string, string> > &defines, const char *variantName) {
	
	string variantPrefix = variantName;
	if(variantPrefix.length() > 0) {
		variantPrefix += "_";
	}

	Document::Node *metaNode = shaderNode->Resolve((variantPrefix+"meta").c_str());
	if(metaNode != nullptr) {
		metaNode->Detach();
		delete metaNode;
		metaNode = nullptr;
	}

	Document::Node *dataNode = shaderNode->Resolve((variantPrefix+"data").c_str());
	if(dataNode != nullptr) {
		dataNode->Detach();
		delete dataNode;
		dataNode = nullptr;
	}
	
	D3D_SHADER_MACRO *macros = new D3D_SHADER_MACRO[defines.size()+1];
	ZeroMemory(macros, (defines.size()+1)*sizeof(D3D_SHADER_MACRO));
	for(uint32 i = 0; i < defines.size(); i++) {
		macros[i].Name = defines[i].first.c_str();
		macros[i].Definition = defines[i].second.c_str();
	}
	
	
	ID3D10Blob *codeBlob = nullptr;
	ID3D10Blob *errorMessages = nullptr;
	wstringstream inputFile;
	inputFile << "assets/" << filePath;
	if(FAILED(D3DCompileFromFile(inputFile.str().c_str(), macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, compTarget, 0, 0, &codeBlob, &errorMessages))) {
		SAFE_DELETE(macros);
		assert(errorMessages != nullptr);
		cout << string((char *)errorMessages->GetBufferPointer(), errorMessages->GetBufferSize()) << endl;
		SAFE_RELEASE(errorMessages);
		return false;
	}
	SAFE_DELETE(macros);
	assert(codeBlob != nullptr);

	// Base64 encode the blob
	string byteCode((char *)codeBlob->GetBufferPointer(), codeBlob->GetBufferSize());
	stringstream in(byteCode, ios::in | ios::binary);
	stringstream out(ios::out | ios::binary);
	if(!Base64::Encode(in, out, 80)) {
		printf("Failed to base64 encode shader data\n");
		return false;
	}
	string encodedData = out.str();

	string dataNodeName = variantPrefix + "data";	
	dataNode = new Document::Node(const_cast<char *>(dataNodeName.c_str()), dataNodeName.length(), true);
	shaderNode->AppendChild(dataNode);
	dataNode->AppendChild(new Document::Node(const_cast<char *>(encodedData.c_str()), encodedData.length(), true));


	// Use reflection to analyze the layout of the cbuffers
	ID3D11ShaderReflection *reflect = nullptr; 
	if(FAILED(D3DReflect(codeBlob->GetBufferPointer(), codeBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflect))) {
		SAFE_RELEASE(codeBlob);
		printf("Failed to reflect upon shader blob\n");
		return false;
	}
	
	D3D11_SHADER_DESC desc;
	if(FAILED(reflect->GetDesc(&desc))) {
		SAFE_RELEASE(codeBlob);
		printf("Failed to get shader_desc from reflection interface\n");
		return false;
	}

	string metaName = variantPrefix + "meta";
	metaNode = new Document::Node(const_cast<char *>(metaName.c_str()), metaName.length(), true);

	HRESULT hr;	
	char *bufferNames[3] = {"enginePerFrame", "enginePerObject", "material"};

	for(uint32 i = 0; i < desc.ConstantBuffers; i++) {
		ID3D11ShaderReflectionConstantBuffer *cbuf = reflect->GetConstantBufferByIndex(i);
		if(cbuf != nullptr) {
			D3D11_SHADER_BUFFER_DESC sbDesc;
			hr = cbuf->GetDesc(&sbDesc);
			if(FAILED(hr)) {
				// No such constant buffer
				continue;
			}
			
			// See if it's one of the buffers that we care about
			uint32 nameIndex = -1;
			for(uint32 j = 0; j < 3; j++) {
				if(strcmp(bufferNames[j], sbDesc.Name) == 0) {
					nameIndex = j;
					break;
				}
			}
			if(nameIndex < 0) {
				continue;
			}
					
			Document::Node *bufferNode = new Document::Node(bufferNames[nameIndex], strlen(bufferNames[nameIndex]), true);
			metaNode->AppendChild(bufferNode);
			
			Document::Node *slotNode = new Document::Node("slot", 4, true);
			bufferNode->AppendChild(slotNode);

			char buffer[32];
			_itoa_s(i, buffer, 10);
			slotNode->AppendChild(new Document::Node(buffer, strlen(buffer), true));
			
			Document::Node *uniformsNode = new Document::Node("uniforms", 8, true);
			bufferNode->AppendChild(uniformsNode);

			for(uint32 j = 0; j < sbDesc.Variables; j++) {
				ID3D11ShaderReflectionVariable *var = cbuf->GetVariableByIndex(j);
				assert(var != nullptr);

				D3D11_SHADER_VARIABLE_DESC varDesc;
				hr = var->GetDesc(&varDesc);
				assert(SUCCEEDED(hr));

				Document::Node *varNode = new Document::Node(const_cast<char *>(varDesc.Name), strlen(varDesc.Name), true);
				uniformsNode->AppendChild(varNode);

				_itoa_s(varDesc.StartOffset, buffer, 10);
				Document::Node *varOffsetNode = new Document::Node(buffer, strlen(buffer), true);
				varNode->AppendChild(varOffsetNode);

				_itoa_s(varDesc.Size, buffer, 10);
				Document::Node *varSizeNode = new Document::Node(buffer, strlen(buffer), true);
				varNode->AppendChild(varSizeNode);
			}
		}
	}

	shaderNode->AppendChild(metaNode);

	SAFE_RELEASE(codeBlob);
	return true;
}

bool CompileAllVariants(Document::Node *shaderNode) {
	Document::Node *fileName = shaderNode->Resolve("file_name");
	if(fileName == nullptr || fileName->count != 1) {
		printf("shader filename must be provided\n");
		return false;
	}

	Document::Node *entry = shaderNode->Resolve("entry_point");
	if(entry == nullptr || entry->count < 1) {
		printf("shader entry point must be provided\n");
		return false;
	}

	Document::Node *target = shaderNode->Resolve("target");
	if(target == nullptr || target->count < 1) {
		printf("compilation target must be provided\n");
		return false;
	}

	Document::Node *definesNode = shaderNode->Resolve("defines");
	vector< pair<string, string> > defines;
	if(definesNode != nullptr) {
		for(uint32 i = 0; i < definesNode->count; i++) {
			Document::Node *entry = definesNode->children[i];
			if(entry->count != 1) {
				printf("defines entry must have a value, ignoring %s\n", entry->value);
			} else {
				defines.push_back(make_pair(definesNode->children[i]->value, definesNode->children[i]->children[0]->value));
			}
		}
	}


	// Regular shader
	if(!Compile(shaderNode, fileName->children[0]->value, target->children[0]->value, entry->children[0]->value, defines, "")) {
		return false;
	}
	
	// Variants
	Document::Node *variants = shaderNode->Resolve("variants");
	if(variants == nullptr) {
		return true;
	}

	for(uint32 i = 0; i < variants->count; i++) {
		Document::Node *variantNode = variants->children[i];
		const char *variantName = variantNode->value;

		vector< pair<string, string> > variantDefines;
		for(uint32 i = 0; i < variantNode->count; i++) {
			Document::Node *variantDefine = variantNode->children[i];
			if(variantDefine->count != 1) {
				printf("Variant define must have exactly one value, skipping %s\n", variantDefine->value);
			} else {
				variantDefines.push_back(make_pair(variantDefine->value, variantDefine->children[0]->value));
			}
		}
		for(uint32 i = 0; i < defines.size(); i++) {
			variantDefines.push_back(defines[i]);
		}

		// Shadow variant shader
		if(!Compile(shaderNode, fileName->children[0]->value, target->children[0]->value, entry->children[0]->value, variantDefines, variantName)) {
			return false;
		}
	}

	return true;
}

bool CompileAll(char *src, char *dst, bool binary) {
	ifstream in(src, ios::in | ios::binary);
	if(!in.good()) {
		in.close();
		printf("Failed to open maki shader: %s\n", src);
		return false;
	}
	
	in.seekg(0, ios::end);
	unsigned int size = (unsigned int)in.tellg();
	in.seekg(0, ios::beg);

	char *buffer = new char[size+1];
	in.read(buffer, size);
	in.close();
	buffer[size] = 0;

	Document doc;
	if(!doc.Load(buffer, size)) {
		delete[] buffer;
		printf("Failed to deserialize document: %s\n", src);
		return false;
	}

	delete[] buffer;
	buffer = nullptr;

	// Compile the hlsl file that the maki shader refers to, and embed its data into the maki shader doc (base64 encoded)
	Document::Node *vs = doc.root->Resolve("vertex_shader");
	if(vs == nullptr || !CompileAllVariants(vs)) {
		return false;
	}

	Document::Node *ps = doc.root->Resolve("pixel_shader");
	if(ps == nullptr || !CompileAllVariants(ps)) {
		return false;
	}

	bool success;
	if(binary) {
		DocumentBinarySerializer serial(doc);
		success = serial.Serialize(dst);
	} else {
		DocumentTextSerializer serial(doc);
		success = serial.Serialize(dst, "\t");
	}	
	if(!success) {
		printf("Failed to serialize document to file: %s\n", dst);
		return false;
	}

	return true;
}

int main(int argc, char **argv) {
	if(argc < 4) {
		printf("Requires three command line params; src, dst, binary (where binary is 1 or 0)\n");
		return 1;
	}
	
	bool binary = argv[3][0] == '1' ? true : false;
	//printf("binary=%d\n", binary);
	
	bool ret = CompileAll(argv[1], argv[2], binary);
	return ret ? 0 : 1;
}