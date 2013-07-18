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
using namespace Maki::Core;

int RunProcess(char *exe, const char *args)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	char commandLine[4096] = "";
	strcat_s(commandLine, exe);
	strcat_s(commandLine, " ");
	strcat_s(commandLine, args);

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	printf("%s\n", commandLine);

	// Start the child process. 
	if(!CreateProcessA(NULL,   // No module name (use command line)
		commandLine,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi )           // Pointer to PROCESS_INFORMATION structure
	) {
		printf("CreateProcess failed (%d)\n", GetLastError());
		return 1;
	}

	WaitForSingleObject( pi.hProcess, INFINITE );
    
	DWORD exitCode;
	if(!GetExitCodeProcess(pi.hProcess, &exitCode)) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		printf("GetExitCodeProcess failed (%d)\n", GetLastError());
		return 1;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return exitCode;
}

bool GenerateShader(bool d3d, const char *filePath, const char *profile, const char *entryPoint, const vector< pair<string, string> > &defines)
{
	std::stringstream ss;
	ss << "-entry " << entryPoint << " -profile " << profile;
	if(d3d) {
		ss << " -d3d";
	}
	for(auto iter = defines.begin(); iter != defines.end(); ++iter) {
		ss << " -D" << iter->first << "=" << iter->second;
	}
	ss << " -o assets/" << filePath << (d3d ? ".d3d" : ".ogl") << " assets/" << filePath;
	return RunProcess("%MAKI_DIR%\\tools\\cgc.exe", ss.str().c_str()) == 0;
}

bool Compile(bool d3d, Document::Node *shaderNode, const char *filePath, const char *profile, const char *entryPoint, const vector< pair<string, string> > &defines, const char *variantName) {
	
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
	
	if(!GenerateShader(d3d, filePath, profile, entryPoint, defines)) {
		printf("Failed to generate shader with CG compiler (d3d=%d)\n", d3d);
		return false;
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
	inputFile << "assets/" << filePath << ".d3d";
	if(FAILED(D3DCompileFromFile(inputFile.str().c_str(), macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, 0, 0, &codeBlob, &errorMessages))) {
		SAFE_DELETE(macros);
		assert(errorMessages != nullptr);
		cout << string((char *)errorMessages->GetBufferPointer(), errorMessages->GetBufferSize()) << endl;
		SAFE_RELEASE(errorMessages);
		return false;
	}
	SAFE_DELETE(macros);
	assert(codeBlob != nullptr);


	string encodedData;
	if(d3d) {

		// Base64 encode the blob
		string byteCode((char *)codeBlob->GetBufferPointer(), codeBlob->GetBufferSize());
		stringstream in(byteCode, ios::in | ios::binary);
		stringstream out(ios::out | ios::binary);
		if(!Base64::Encode(in, out, 80)) {
			SAFE_RELEASE(codeBlob);
			printf("Failed to base64 encode shader data\n");
			return false;
		}
		encodedData = out.str();

	} else {

		stringstream inputFile;
		inputFile << "assets/" << filePath << ".ogl";
		ifstream sourceFile(inputFile.str());
		if(!sourceFile.good()) {
			SAFE_RELEASE(codeBlob);
			printf("Failed to load file from disk: %s\n", inputFile.str().c_str());
			return false;
		}

		// Base64 encode the source code
		stringstream out(ios::out | ios::binary);
		if(!Base64::Encode(sourceFile, out, 80)) {
			SAFE_RELEASE(codeBlob);
			printf("Failed to base64 encode shader data\n");
			sourceFile.close();
			return false;
		}
		encodedData = out.str();
		sourceFile.close();
	}

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
			int32 nameIndex = -1;
			for(int32 j = 0; j < 3; j++) {
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

bool CompileAllVariants(bool d3d, Document::Node *shaderNode) {
	const char *fileName = shaderNode->ResolveValue("file_name.#0");
	if(fileName == nullptr) {
		printf("shader filename must be provided\n");
		return false;
	}

	const char *entry = shaderNode->ResolveValue("entry_point.#0");
	if(entry == nullptr) {
		printf("shader entry point must be provided\n");
		return false;
	}

	const char *profile = shaderNode->ResolveValue("target.#0");
	if(profile == nullptr) {
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
	if(!Compile(d3d, shaderNode, fileName, profile, entry, defines, "")) {
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
		if(!Compile(d3d, shaderNode, fileName, profile, entry, variantDefines, variantName)) {
			return false;
		}
	}

	return true;
}

bool CompileAll(char *src, char *dst, bool d3d, bool binary) {
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
	if(vs == nullptr || !CompileAllVariants(d3d, vs)) {
		return false;
	}

	Document::Node *ps = doc.root->Resolve("pixel_shader");
	if(ps == nullptr || !CompileAllVariants(d3d, ps)) {
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
	if(argc < 5) {
		printf("Requires three command line params; src, dst, api, binary (where api is d3d|ogl and binary is 1 or 0)\n");
		return 1;
	}
	
	bool d3d = string(argv[3]) == "d3d";
	bool binary = argv[4][0] == '1' ? true : false;
	//printf("binary=%d\n", binary);
	
	bool ret = CompileAll(argv[1], argv[2], d3d, binary);
	return ret ? 0 : 1;
}