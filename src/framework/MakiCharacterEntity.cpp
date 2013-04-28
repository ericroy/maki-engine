#include "framework/framework_stdafx.h"
#include "framework/MakiCharacterEntity.h"
#include "framework/MakiSkinnedMeshEntity.h"
#include "framework/MakiCamera.h"

namespace Maki
{
	const float CharacterEntity::TURN_SPEED = 8.0f;
	const float CharacterEntity::CAM_TURN_SPEED = 4.0f;
	const float CharacterEntity::MOVE_SPEED = 2.0f;

	CharacterEntity::CharacterEntity()
		: Entity(), body(nullptr)
	{
		SetFlag(Flag_Draw, false);

		PhysicsWorld *world = PhysicsWorld::Get();
		
		const float radius = 0.4f;
		const float height = 1.8f;
		btCollisionShape *shape = world->Track(new btCapsuleShape(radius, height-2*radius));
		
		btCompoundShape *container = world->Track(new btCompoundShape());
		container->addChildShape(btTransform(TO_BTQUAT(Vector3(MAKI_PI/2.0f, 0.0f, 0.0f)), btVector3(0.0f, 0.0f, height/2)), shape);

		btRigidBody::btRigidBodyConstructionInfo info(80.0f, &physicsLink, container, btVector3(0,0,0));
		body = world->Track(new btRigidBody(info));
		body->setActivationState(DISABLE_DEACTIVATION);
		body->setAngularFactor(0.0f);
		world->AddRigidBody(body);

		Engine *eng = Engine::Get();
		bool ok = blender.Load(eng->assets->PathToRid("animations/player_blend.mdoc"));
		assert(ok);
	}

	CharacterEntity::~CharacterEntity()
	{
	}

	void CharacterEntity::Update(float dt)
	{
		assert(children.size() >= 2);
		SkinnedMeshEntity *skin = (SkinnedMeshEntity *)children[0];
		Entity *camContainer = children[1];
		Entity *cam = camContainer->children[0];

		Vector4 deltaVel(0.0f);

		// Read controller input and apply velocities to the rigid body
		Engine *eng = Engine::Get();
		InputState::Controller *c = eng->inputState->GetPlayer(0)->controller;

		// Create vectors to describe the thumbstick influences from the controller.
		// Allow these to be less than unit length, but not more.
		Vector4 walkFactor(0.0f);
		Vector4 camFactor(0.0f);
		if(c != nullptr) {
			walkFactor = Vector4(-c->GetFloat(InputState::Button_LeftThumbX), c->GetFloat(InputState::Button_LeftThumbY), 0.0f, 1.0f);
			if(walkFactor.LengthSquared() > 1.0f) {
				walkFactor.Normalize();
			}
			camFactor = Vector4(c->GetFloat(InputState::Button_RightThumbX), c->GetFloat(InputState::Button_RightThumbY), 0.0f, 1.0f);
			if(camFactor.LengthSquared() > 1.0f) {
				camFactor.Normalize();
			}
		}

		// Orbit the camera according to the right thubstick influence
		Vector4 camPerp = camContainer->GetWorldMatrix() * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
		
		Quaternion pitchDelta, yawDelta;
		pitchDelta.FromAngleAxis(camFactor.y * CAM_TURN_SPEED * dt, camPerp);		
		yawDelta.FromAngleAxis(camFactor.x * CAM_TURN_SPEED * dt, Vector4::UnitZ);
		Quaternion camOrient = yawDelta * pitchDelta * camContainer->GetOrientation();

		// Recalculate the camera directions based on our change to the orientation
		camPerp = camOrient * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
		Vector4 camDir = Vector4::UnitZ.Cross(camPerp);

		// Check if pitch needs clamping
		const float thresholdUpper = -0.25f;
		const float thresholdLower = -0.75f;
		Vector4 toCam = camOrient * Vector4(0.0f, 0.0f, 1.0f, 0.0f);
		float d = toCam.Dot(camDir);
		if(d > thresholdUpper && toCam.z > 0.0f) {
			// Clamp to north pole limit
			Quaternion q;
			q.FromAngleAxis(std::acos(thresholdUpper), camPerp);
			Vector4 clampedToCam = q * camDir;
			q.FromRotationArc(toCam, clampedToCam);
			camOrient = q * camOrient;

			// Update camera directions
			camPerp = camOrient * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
			camDir = Vector4::UnitZ.Cross(camPerp);
		} else if(d > thresholdLower && toCam.z <= 0.0f) {
			// Clamp to south pole limit
			Quaternion q;
			q.FromAngleAxis(2*MAKI_PI - std::acos(thresholdLower), camPerp);
			Vector4 clampedToCam = q * camDir;
			q.FromRotationArc(toCam, clampedToCam);
			camOrient = q * camOrient;

			// Update camera directions
			camPerp = camOrient * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
			camDir = Vector4::UnitZ.Cross(camPerp);
		}
		camContainer->SetOrientation(camOrient);

		
		// Direction the character model is facing
		Vector4 dir = skin->GetWorldMatrix() * Vector4(0.0f, -1.0f, 0.0f, 0.0f);

		// Direction the character model *should* be facing, according to the controller thumbstick
		Vector4 targetDir = camDir * walkFactor.y + camPerp * walkFactor.x;
		float targetDirLen = targetDir.Length();
		if(targetDirLen < 0.000001f) {
			targetDirLen = 0.0f;
			targetDir = dir;
		} else {
			targetDir /= targetDirLen;
		}

		// Slerp the character model's facing direction towards the desired facing direction
		Quaternion rotDelta;
		rotDelta.FromRotationArc(dir, targetDir);
		const Quaternion &skinOrient = skin->GetOrientation();

		Quaternion finalOrientation = Quaternion::Slerp(dt * TURN_SPEED, skinOrient, rotDelta*skinOrient);
		// Prevent drift
		finalOrientation.Normalize();
		skin->SetOrientation(finalOrientation);

		// Add linear velocity in the target facing direction.  Scale this down by the degree
		// to which the character is not yet facing in the right direction.  This means the character
		// will accelerate up to the desired speed as they turn to face the move direction.
		float speed = MOVE_SPEED * std::min(targetDirLen, 1.0f);
		deltaVel += targetDir * speed * Clamp(dir.Dot(targetDir), 0.0f, 1.0f);

		// Calculate the impulse required to give us the velocity that we want
		Vector4 impulse = deltaVel / body->getInvMass();

		// Stomp x and y velocity, then apply our desired impulse
		btVector3 linearVel = body->getLinearVelocity();
		linearVel.setZ(0.0f);
		body->applyImpulse(-linearVel/body->getInvMass(), btVector3(0, 0, 0));
		body->applyImpulse(TO_BTVEC3(impulse), btVector3(0, 0, 0));

		// Update animation blend tree based on speed
		((AnimationBlender::Blend *)blender.root)->balance.Set(speed / MOVE_SPEED, 0.1f);
		blender.AdvanceState(dt, 1.0f, skin->pose);
		skin->SetPoseDirty(true);
	}








	CharacterEntityFactory::CharacterEntityFactory()
		: EntityFactory()
	{
	}

	CharacterEntityFactory::~CharacterEntityFactory()
	{
	}

	bool CharacterEntityFactory::PreCreate(Document::Node *node)
	{
		return EntityFactory::PreCreate(node);
	}

	CharacterEntity *CharacterEntityFactory::Create()
	{
		return new CharacterEntity();
	}

	void CharacterEntityFactory::PostCreate(CharacterEntity *e)
	{
		EntityFactory::PostCreate(e);
	}



} // namespace Maki