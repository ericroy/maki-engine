#pragma once
#include "framework/framework_stdafx.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

namespace Maki
{
	namespace Framework
	{
	
		class DebugPhysicsRenderer : public btIDebugDraw
		{
		public:
			static const uint32 MAX_LINES = 4096;

		public:
			DebugPhysicsRenderer(Rid matRid);
			virtual ~DebugPhysicsRenderer();

			void Draw(btDynamicsWorld *world);
			virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
			virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
			virtual void reportErrorWarning(const char* warningString);
			virtual void draw3dText(const btVector3& location, const char* textString);
			virtual void setDebugMode(int debugMode);
			virtual int getDebugMode() const;
		
		private:
			int debugMode;
			std::vector<Handle> lines;
			uint32 lineIndex;
			Handle material;
			CoreManagers res;
		};

	} // namespace Framework

} // namespace Maki