#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CampusARPlayerController.generated.h"

class ACampusARCardActor;
class UARSessionConfig;
class UTexture2D;

UCLASS()
class ARPROJECT_API ACampusARPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACampusARPlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY()
	ACampusARCardActor* CampusCard;

	bool bWasTouch1Down;
	bool bWasTouch2Down;
	FVector2D LastTouch1;
	FVector2D LastTouch2;
	FVector2D PinchStartVector;
	float LastPinchDistance;
	float ScaleAtPinchStart;
	float CurrentCardScale;
	float EmblemScaleAtPinchStart;
	float YawOffsetDegrees;
	float YawAtPinchStart;
	float EmblemYawAtPinchStart;
	float AccumulatedTouchMove;
	FVector CardLocalOffset;
	FTransform LatestImageTransform;
	bool bHasLatestImageTransform;

	UPROPERTY()
	UARSessionConfig* RuntimeImageSessionConfig;

	UPROPERTY()
	UTexture2D* MarkerTexture;

	UPROPERTY()
	UTexture2D* ProfilePhotoTexture;

	bool bImageSessionStarted;

	void UpdateTouchInput();
	void StartCampusCardImageSession();
	void PollTrackedCampusCard();
	UTexture2D* LoadTextureFromContentRaw(const FString& FileName, const FString& DebugName) const;
	void HandleTap(const FVector2D& ScreenPosition);
	bool TryTapMenu(const FVector2D& ScreenPosition);
	bool TryPlaceCampusCard(const FVector2D& ScreenPosition);
	void SpawnCardAtTransform(const FTransform& SpawnTransform);
	void ApplyCampusCardManipulation();
	void ResetCampusCardManipulation();
	FTransform GetFallbackCameraTransform() const;
};
