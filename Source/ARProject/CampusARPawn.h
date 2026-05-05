#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CampusARPawn.generated.h"

class UCameraComponent;
class USceneComponent;

UCLASS()
class ARPROJECT_API ACampusARPawn : public APawn
{
	GENERATED_BODY()

public:
	ACampusARPawn();

private:
	UPROPERTY(VisibleAnywhere, Category = "Campus AR")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Campus AR")
	UCameraComponent* Camera;
};
