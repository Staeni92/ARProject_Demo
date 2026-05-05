#include "CampusARPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"

ACampusARPawn::ACampusARPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ARCamera"));
	Camera->SetupAttachment(SceneRoot);
	Camera->bUsePawnControlRotation = false;

	bFindCameraComponentWhenViewTarget = true;
}
