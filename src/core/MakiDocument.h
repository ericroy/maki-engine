#pragma once
#include "core/core_stdafx.h"
#include <cstdlib>

namespace Maki
{
	namespace Core
	{

		class Document
		{
			friend class DocumentTextSerializer;
			friend class DocumentBinarySerializer;

		public:
			class Node
			{
				friend class Document;
				friend class DocumentTextSerializer;
				friend class DocumentBinarySerializer;
		
			public:
				Node(char *data, uint32 length, bool shouldCloneData);
				~Node();

				void AppendChild(Node *n);
				inline void AppendSibling(Node *n) { assert(parent != nullptr); parent->AppendChild(n); }
				Node *RemoveChild(uint32 index);
				bool RemoveChild(Node *n);
				void Node::Detach();

				inline bool ValueEquals(const char *v) const { return strcmp(v, value) == 0; }
			
				int32 ValueAsInt(int32 defaultValue = 0) const;
				bool ValueAsInt(int32 *out) const;

				uint32 ValueAsUInt(uint32 defaultValue = 0) const;
				bool ValueAsUInt(uint32 *out) const;

				float ValueAsFloat(float defaultValue = 0.0f) const;
				bool ValueAsFloat(float *out) const;

				bool ValueAsBool(bool defaultValue = false) const;
				bool ValueAsBool(bool *out) const;
			
				// Path resolution helpers:
				Node *Resolve(const char *nodePath);
				inline char *ResolveValue(const char *nodePath) { Document::Node *n = Resolve(nodePath); if(n) { return n->value; } return nullptr; }
				inline bool ResolveAsInt(const char *nodePath, int32 *out) { Document::Node *n = Resolve(nodePath); if(n) { return n->ValueAsInt(out); } return false; }
				inline int32 ResolveAsInt(const char *nodePath, int32 defaultValue = 0) { Document::Node *n = Resolve(nodePath); if(n) { return n->ValueAsInt(defaultValue); } return defaultValue; }
				inline bool ResolveAsUInt(const char *nodePath, uint32 *out) { Document::Node *n = Resolve(nodePath); if(n) { return n->ValueAsUInt(out); } return false; }
				inline uint32 ResolveAsUInt(const char *nodePath, uint32 defaultValue = 0) { Document::Node *n = Resolve(nodePath); if(n) { return n->ValueAsUInt(defaultValue); } return defaultValue; }
				inline bool ResolveAsFloat(const char *nodePath, float *out) { Document::Node *n = Resolve(nodePath); if(n) { return n->ValueAsFloat(out); } return false; }
				inline float ResolveAsFloat(const char *nodePath, float defaultValue = 0.0f) { Document::Node *n = Resolve(nodePath); if(n) { return n->ValueAsFloat(defaultValue); } return defaultValue; }
				inline bool ResolveAsBool(const char *nodePath, bool *out) { Document::Node *n = Resolve(nodePath); if(n) { return n->ValueAsBool(out); } return false; }
				inline bool ResolveAsBool(const char *nodePath, bool defaultValue = false) { Document::Node *n = Resolve(nodePath); if(n) { return n->ValueAsBool(defaultValue); } return defaultValue; }
			
				inline bool ResolveAsVectorN(const char *nodePath, uint32 count, float *out) {
					Document::Node *n = Resolve(nodePath);
					if(n) {
						assert(n->count >= count);
						for(uint32 i = 0; i < count; i++) {
							out[i] = n->children[i]->ValueAsFloat();
						}
						return true;
					}
					return false;
				}

			public:
				Node *parent;
				char *value;
				uint32 count;
				uint32 capacity;
				Node **children;
			};
	
	
		public:
			Document();
			virtual ~Document();
		
			void Print(const char *indentToken = "  ");
			bool Load(char *data, uint32 length);
	#ifndef MAKI_TOOLS
			bool Load(Rid rid);
	#endif

		public:
			Node *root;
		};


		class DocumentTextSerializer
		{
		private:
			static const int32 STACK_CHILDREN_COUNT_THRESHOLD = 4;

		public:
			DocumentTextSerializer(Document &doc);
			~DocumentTextSerializer();

			bool Serialize(char *path, const char *indentToken = "\t");
			bool Serialize(std::ostream &out, const char *indentToken = "\t");
			bool Deserialize(char *data, uint32 length);

		private:
			bool AddNode(Document::Node *n, int32 indentLevel, Document::Node **previous, int32 &previousIndentLevel, bool &cleanLine, bool &appendSibling);
			void SerializeNode(Document::Node *n, uint32 depth, std::ostream &out, bool stacking, bool firstInStack, bool lastInStack, const char *indentToken);

		private:
			Document &doc;
		};


		class DocumentBinarySerializer
		{
		private:
			static const uint8 BINARY_HEADER[4];

		public:
			static bool IsBinaryDocument(char *data, uint32 length);

		public:
			DocumentBinarySerializer(Document &doc);
			~DocumentBinarySerializer();

			bool Serialize(char *path);
			bool Serialize(std::ostream &out);
			bool Deserialize(char *data, uint32 length);

		private:
			uint16 GetOrAdd(std::vector<std::string> &stringTable, char *str);
			void SerializeNode(Document::Node *n, std::ostream &body, std::vector<std::string> &stringTable, uint32 &level);

		private:
			Document &doc;
		};


	} // namespace Core

} // namespace Maki