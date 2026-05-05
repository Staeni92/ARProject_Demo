#include "CampusARPlayerController.h"

#include "ARBlueprintLibrary.h"
#include "ARSessionConfig.h"
#include "ARTrackable.h"
#include "ARTraceResult.h"
#include "CampusARCardActor.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "TimerManager.h"
#include "TextureResource.h"

ACampusARPlayerController::ACampusARPlayerController()
{
	bEnableTouchEvents = true;
	bEnableClickEvents = true;
	bShowMouseCursor = false;
	PrimaryActorTick.bCanEverTick = true;

	CampusCard = nullptr;
	bWasTouch1Down = false;
	bWasTouch2Down = false;
	PinchStartVector = FVector2D::ZeroVector;
	LastPinchDistance = 0.0f;
	ScaleAtPinchStart = 1.0f;
	CurrentCardScale = 1.0f;
	YawOffsetDegrees = 0.0f;
	YawAtPinchStart = 0.0f;
	AccumulatedTouchMove = 0.0f;
	CardLocalOffset = FVector::ZeroVector;
	LatestImageTransform = FTransform::Identity;
	bHasLatestImageTransform = false;
	RuntimeImageSessionConfig = nullptr;
	MarkerTexture = nullptr;
	ProfilePhotoTexture = nullptr;
	bImageSessionStarted = false;
}

void ACampusARPlayerController::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimerForNextTick(this, &ACampusARPlayerController::StartCampusCardImageSession);
	FTimerHandle RestartTimer;
	GetWorldTimerManager().SetTimer(RestartTimer, this, &ACampusARPlayerController::StartCampusCardImageSession, 2.0f, false);
}

void ACampusARPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PollTrackedCampusCard();
	UpdateTouchInput();
}

void ACampusARPlayerController::StartCampusCardImageSession()
{
	if (!GetWorld())
	{
		return;
	}

	if (!MarkerTexture)
	{
		MarkerTexture = LoadTextureFromContentRaw(TEXT("StudentIDMarker.jpg"), TEXT("CampusIDMarkerTexture"));
	}
	if (!ProfilePhotoTexture)
	{
		ProfilePhotoTexture = LoadTextureFromContentRaw(TEXT("ProfilePhoto.jpg"), TEXT("CampusProfilePhotoTexture"));
	}
	if (!MarkerTexture)
	{
		return;
	}

	RuntimeImageSessionConfig = NewObject<UARSessionConfig>(this, TEXT("CampusIDRuntimeImageSessionConfig"));
	RuntimeImageSessionConfig->SetEnableAutoFocus(true);
	RuntimeImageSessionConfig->SetResetCameraTracking(true);
	RuntimeImageSessionConfig->SetResetTrackedObjects(true);
	RuntimeImageSessionConfig->ClearCandidateImages();

	UARCandidateImage* CandidateImage = UARBlueprintLibrary::AddRuntimeCandidateImage(RuntimeImageSessionConfig, MarkerTexture, TEXT("CampusIDMarker"), 8.6f);
	if (!CandidateImage)
	{
		CandidateImage = UARCandidateImage::CreateNewARCandidateImage(MarkerTexture, TEXT("CampusIDMarker"), 8.6f, 5.4f, EARCandidateImageOrientation::Landscape);
		RuntimeImageSessionConfig->AddCandidateImage(CandidateImage);
	}

	UARBlueprintLibrary::StartARSession(RuntimeImageSessionConfig);
	bImageSessionStarted = true;
}

void ACampusARPlayerController::PollTrackedCampusCard()
{
	if (!bImageSessionStarted)
	{
		return;
	}

	const TArray<UARTrackedImage*> TrackedImages = UARBlueprintLibrary::GetAllGeometriesByClass<UARTrackedImage>();
	for (UARTrackedImage* TrackedImage : TrackedImages)
	{
		if (!TrackedImage || !TrackedImage->GetDetectedImage())
		{
			continue;
		}

		if (TrackedImage->GetDetectedImage()->GetFriendlyName() != TEXT("CampusIDMarker"))
		{
			continue;
		}

		FTransform ImageTransform = TrackedImage->GetLocalToWorldTransform();
		ImageTransform.SetScale3D(FVector(0.8f));
		ImageTransform.ConcatenateRotation(FQuat(FRotator(0.0f, 0.0f, 0.0f)));
		LatestImageTransform = ImageTransform;
		bHasLatestImageTransform = true;

		if (!CampusCard)
		{
			SpawnCardAtTransform(ImageTransform);
			if (CampusCard)
			{
				CurrentCardScale = CampusCard->GetActorScale3D().X;
				CampusCard->SetProfilePhoto(ProfilePhotoTexture);
			}
		}
		else
		{
			ApplyCampusCardManipulation();
		}

		return;
	}
}

UTexture2D* ACampusARPlayerController::LoadTextureFromContentRaw(const FString& FileName, const FString& DebugName) const
{
	const FString FullPath = FPaths::ProjectContentDir() / TEXT("Raw") / FileName;
	TArray<uint8> CompressedData;
	if (!FFileHelper::LoadFileToArray(CompressedData, *FullPath))
	{
		return nullptr;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	const EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(CompressedData.GetData(), CompressedData.Num());
	if (ImageFormat == EImageFormat::Invalid)
	{
		return nullptr;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(CompressedData.GetData(), CompressedData.Num()))
	{
		return nullptr;
	}

	TArray<uint8> RawBGRA;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawBGRA))
	{
		return nullptr;
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8, *DebugName);
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		return nullptr;
	}

	Texture->SRGB = true;
	void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, RawBGRA.GetData(), RawBGRA.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();
	return Texture;
}

void ACampusARPlayerController::UpdateTouchInput()
{
	float X1 = 0.0f;
	float Y1 = 0.0f;
	float X2 = 0.0f;
	float Y2 = 0.0f;
	bool bTouch1Down = false;
	bool bTouch2Down = false;
	GetInputTouchState(ETouchIndex::Touch1, X1, Y1, bTouch1Down);
	GetInputTouchState(ETouchIndex::Touch2, X2, Y2, bTouch2Down);

	const FVector2D Touch1(X1, Y1);
	const FVector2D Touch2(X2, Y2);

	if (bTouch1Down && !bWasTouch1Down)
	{
		LastTouch1 = Touch1;
		AccumulatedTouchMove = 0.0f;
	}

	if (bTouch1Down && bTouch2Down)
	{
		const float CurrentDistance = FVector2D::Distance(Touch1, Touch2);
		const FVector2D CurrentPinchVector = Touch2 - Touch1;
		if (!bWasTouch2Down)
		{
			LastPinchDistance = CurrentDistance;
			PinchStartVector = CurrentPinchVector;
			ScaleAtPinchStart = CurrentCardScale;
			YawAtPinchStart = YawOffsetDegrees;
		}
		else if (CampusCard && LastPinchDistance > 1.0f)
		{
			const float ScaleFactor = CurrentDistance / LastPinchDistance;
			CurrentCardScale = FMath::Clamp(ScaleAtPinchStart * ScaleFactor, 0.45f, 2.2f);

			if (!PinchStartVector.IsNearlyZero() && !CurrentPinchVector.IsNearlyZero())
			{
				const float Cross = (PinchStartVector.X * CurrentPinchVector.Y) - (PinchStartVector.Y * CurrentPinchVector.X);
				const float Dot = FVector2D::DotProduct(PinchStartVector, CurrentPinchVector);
				YawOffsetDegrees = YawAtPinchStart + FMath::RadiansToDegrees(FMath::Atan2(Cross, Dot));
			}

			ApplyCampusCardManipulation();
		}
	}
	else if (bTouch1Down && bWasTouch1Down && CampusCard)
	{
		const FVector2D Delta = Touch1 - LastTouch1;
		AccumulatedTouchMove += Delta.Size();
		CardLocalOffset += FVector(Delta.X * 0.035f, -Delta.Y * 0.035f, 0.0f);
		ApplyCampusCardManipulation();
	}

	if (!bTouch1Down && bWasTouch1Down && !bWasTouch2Down && AccumulatedTouchMove < 18.0f)
	{
		HandleTap(LastTouch1);
	}

	if (bTouch1Down)
	{
		LastTouch1 = Touch1;
	}
	if (bTouch2Down)
	{
		LastTouch2 = Touch2;
	}

	bWasTouch1Down = bTouch1Down;
	bWasTouch2Down = bTouch2Down;
}

void ACampusARPlayerController::HandleTap(const FVector2D& ScreenPosition)
{
	if (CampusCard && TryTapMenu(ScreenPosition))
	{
		return;
	}

	if (!CampusCard)
	{
		// Placement is intentionally driven only by the configured campus-card image target.
	}
}

bool ACampusARPlayerController::TryTapMenu(const FVector2D& ScreenPosition)
{
	FVector WorldOrigin;
	FVector WorldDirection;
	if (!DeprojectScreenPositionToWorld(ScreenPosition.X, ScreenPosition.Y, WorldOrigin, WorldDirection))
	{
		return false;
	}

	const FVector RayEnd = WorldOrigin + WorldDirection * 10000.0f;
	bool bResetRequested = false;
	const bool bHandled = CampusCard && CampusCard->HandleWorldTap(WorldOrigin, RayEnd, &bResetRequested);
	if (bResetRequested)
	{
		ResetCampusCardManipulation();
	}
	return bHandled;
}

bool ACampusARPlayerController::TryPlaceCampusCard(const FVector2D& ScreenPosition)
{
	const TArray<FARTraceResult> TraceResults = UARBlueprintLibrary::LineTraceTrackedObjects(ScreenPosition, false, true, true, true);
	if (TraceResults.Num() > 0)
	{
		FTransform SpawnTransform = TraceResults[0].GetLocalToWorldTransform();
		SpawnTransform.SetScale3D(FVector(1.0f));
		SpawnCardAtTransform(SpawnTransform);
		return true;
	}

	SpawnCardAtTransform(GetFallbackCameraTransform());
	return true;
}

void ACampusARPlayerController::SpawnCardAtTransform(const FTransform& SpawnTransform)
{
	if (CampusCard || !GetWorld())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	CampusCard = GetWorld()->SpawnActor<ACampusARCardActor>(ACampusARCardActor::StaticClass(), SpawnTransform, SpawnParams);
	if (CampusCard)
	{
		CurrentCardScale = SpawnTransform.GetScale3D().X;
	}
}

void ACampusARPlayerController::ApplyCampusCardManipulation()
{
	if (!CampusCard || !bHasLatestImageTransform)
	{
		return;
	}

	const FVector ManipulatedLocation = LatestImageTransform.TransformPosition(CardLocalOffset);
	const FQuat ManipulatedRotation = LatestImageTransform.GetRotation() * FQuat(FRotator(0.0f, YawOffsetDegrees, 0.0f));

	CampusCard->SetActorLocation(ManipulatedLocation);
	CampusCard->SetActorRotation(ManipulatedRotation);
	CampusCard->SetCampusCardScale(CurrentCardScale);
}

void ACampusARPlayerController::ResetCampusCardManipulation()
{
	CardLocalOffset = FVector::ZeroVector;
	YawOffsetDegrees = 0.0f;
	YawAtPinchStart = 0.0f;
	CurrentCardScale = bHasLatestImageTransform ? LatestImageTransform.GetScale3D().X : 1.0f;
	ApplyCampusCardManipulation();
}

FTransform ACampusARPlayerController::GetFallbackCameraTransform() const
{
	FVector CameraLocation = FVector::ZeroVector;
	FRotator CameraRotation = FRotator::ZeroRotator;
	if (PlayerCameraManager)
	{
		CameraLocation = PlayerCameraManager->GetCameraLocation();
		CameraRotation = PlayerCameraManager->GetCameraRotation();
	}

	const FVector Forward = CameraRotation.Vector();
	const FVector SpawnLocation = CameraLocation + Forward * 120.0f + FVector(0.0f, 0.0f, -25.0f);
	const FRotator SpawnRotation(0.0f, CameraRotation.Yaw + 180.0f, 0.0f);
	return FTransform(SpawnRotation, SpawnLocation, FVector(1.0f));
}
