#pragma once

#define TO_BTVEC3(v) btVector3(v.x, v.y, v.z)
#define TO_BTQUAT(euler) btQuaternion(euler.z, euler.x, euler.y)