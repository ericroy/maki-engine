#include "core/MakiDocument.h"
#include <fstream>

using namespace std;
using namespace Maki;
using namespace Maki::Core;

bool compile(char *src, char *dst, bool binary) {
	ifstream in(src, ios::in | ios::binary);
	if(!in.good()) {
		printf("Failed to open document: %s\n", src);
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
		printf("Failed to deserialize document: %s\n", src);
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
	}

	delete[] buffer;
	in.close();
	return true;
}

int main(int argc, char **argv) {
	if(argc < 4) {
		printf("Requires three command line params; src, dst, binary (where binary is 1 or 0)\n");
		return 1;
	}
	
	bool binary = argv[3][0] == '1' ? true : false;
	//printf("binary=%d\n", binary);
	
	bool ret = compile(argv[1], argv[2], binary);
	return ret ? 0 : 1;
}