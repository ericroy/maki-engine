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
			static const uint32 DEFAULT_MAX_LINES = 2048;

		public:
			DebugPhysicsRenderer(Config *config, Rid matRid);
			virtual ~DebugPhysicsRenderer();

			void Update(btDynamicsWorld *world);
			void Draw();
			virtual void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color);
			virtual void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int32 lifeTime, const btVector3 &color);
			virtual void reportErrorWarning(const char *warningString);
			virtual void draw3dText(const btVector3 &location, const char *textString);
			virtual void setDebugMode(int32 debugMode);
			virtual int getDebugMode() const;
		
		private:
			CoreManagers res;
			Handle lines;
			Handle material;
			DrawCommand cmd;
			uint32 lineIndex;
			uint32 maxLines;
			int debugMode;
		};

	} // namespace Framework

} // namespace Maki