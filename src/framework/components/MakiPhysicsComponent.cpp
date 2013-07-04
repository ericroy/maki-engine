#pragma once
#include "framework/framework_stdafx.h"
#include "framework/components/MakiPhysicsComponent.h"
#include "framework/components/MakiTransformComponent.h"

namespace Maki
{
	namespace Framework
	{
		namespace Components
		{

			Physics::Physics()
				: Component(TYPE, DEPENDENCIES),
				objectType(ObjectType_Static),
				objectShape(ObjectShape_Mesh),
				mass(1.0f),
				inertia(1.0f),
				mesh(HANDLE_NONE),
				transComp(nullptr),
				shape(nullptr),
				subShape(nullptr),
				collisionObject(nullptr)
			{
			}

			Physics::~Physics()
			{
				transComp = nullptr;
				
				if(objectType == ObjectType_Static) {
					SAFE_DELETE(collisionObject);
				} else {
					SAFE_DELETE(body);
				}

				SAFE_DELETE(shape);

				if(objectShape == ObjectShape_Mesh) {
					SAFE_DELETE(vertexArray);
					MeshManager::Free(mesh);
				} else {
					SAFE_DELETE(subShape);
				}
			}

			bool Physics::Init(Document::Node *props)
			{
				Document::Node *typeNode = props->Resolve("type.#0");
				if(typeNode == nullptr) {
					Console::Error("Physics component needs 'type' node");
					return false;
				}

				Document::Node *shapeNode = props->Resolve("shape.#0");
				if(shapeNode == nullptr) {
					Console::Error("Physics component needs 'shape' node");
					return false;
				}

				mass = props->ResolveAsFloat("mass", 1.0f);
				props->ResolveAsVectorN("inertia", 3, inertia.vals);

				if(typeNode->ValueEquals("static")) {
					objectType = ObjectType_Static;
				} else {
					objectType = ObjectType_Dynamic;
				}

				if(shapeNode->ValueEquals("box")) {
					Vector4 minCorner(0.0f), maxCorner(0.0f);
					if(!props->ResolveAsVectorN("min", 3, minCorner.vals)) {
						Console::Error("Physics box needs 'min' node");
						return false;
					}
					if(!props->ResolveAsVectorN("max", 3, maxCorner.vals)) {
						Console::Error("Physics box needs 'max' node");
						return false;
					}

					return InitBoxShape(objectType, minCorner, maxCorner, mass, inertia);
				} else if(shapeNode->ValueEquals("mesh")) {
					const char *meshPath = props->ResolveValue("mesh.#0");
					if(meshPath == nullptr) {
						Console::Error("Entity did not specify a mesh");
						return false;
					}
					Rid meshRid = Engine::Get()->assets->PathToRid(meshPath);
					if(meshRid == RID_NONE) {
						Console::Error("No RID for path: %s", meshPath);
						return false;
					}

					return InitMeshShape(objectType, meshRid, mass, inertia);
				} else {
					Console::Error("Unrecognized physics component type: %s", typeNode->value);
					return false;
				}
			}

			bool Physics::InitMeshShape(ObjectType type, HandleOrRid meshId, float mass, const Vector3 &inertia)
			{
				objectType = type;
				objectShape = ObjectShape_Mesh;
				this->mass = mass;
				this->inertia = inertia;

				if(meshId.isHandle) {
					if(meshId.handle == HANDLE_NONE) {
						Console::Error("Mesh handle must not be HANDLE_NONE");
						return false;
					}
					MeshManager::AddRef(meshId.handle);
					mesh = meshId.handle;
				} else {
					mesh = CoreManagers::Get()->meshManager->Load(meshId.rid);
					if(mesh == HANDLE_NONE) {
						Console::Error("Failed to load mesh: Rid<%u>", meshId.rid);
						return false;
					}
				}
				return true;
			}

			bool Physics::InitBoxShape(ObjectType type, const Vector4 &minCorner, const Vector4 &maxCorner, float mass, const Vector3 &inertia)
			{
				objectType = type;
				objectShape = ObjectShape_Box;
				this->mass = mass;
				this->inertia = inertia;
				this->minCorner = minCorner;
				this->maxCorner = maxCorner;
				return true;
			}

			void Physics::OnAttach()
			{
				transComp = owner->Get<Transform>();
				assert(transComp != nullptr);
			}
			
			void Physics::OnDetach()
			{
				transComp = nullptr;
			}


			void Physics::getWorldTransform(btTransform &worldTransform) const
			{
				if(transComp != nullptr) {
					const Matrix44 &m = transComp->GetMatrix();
					worldTransform.setFromOpenGLMatrix(m.vals);
				}
			}

			void Physics::setWorldTransform(const btTransform &worldTransform)
			{
				if(transComp != nullptr) {
					Matrix44 m;
					worldTransform.getOpenGLMatrix(m.vals);
					transComp->SetMatrix(m);
				}
			}





		} // namspace Components

	} // namespace Framework

} // namespace Maki