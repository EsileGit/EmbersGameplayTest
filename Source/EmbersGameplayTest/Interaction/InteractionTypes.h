// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

/**
 *  Object channel used by world objects the player can look at and interact with.
 *  Declared in DefaultEngine.ini under [/Script/Engine.CollisionProfile] as "PickupItem".
 */
#define COLLISION_PICKUPITEM ECollisionChannel::ECC_GameTraceChannel2
