#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CampusARCardActor.generated.h"

class UBoxComponent;
class UCampusPhotoWidget;
class UProceduralMeshComponent;
class USceneComponent;
class USoundWaveProcedural;
class UTextRenderComponent;
class UTexture2D;
class UWidgetComponent;

UENUM()
enum class ECampusCardPage : uint8
{
	Profile,
	Website,
	Emblem,
	Custom
};

UCLASS()
class ARPROJECT_API ACampusARCardActor : public AActor
{
	GENERATED_BODY()

public:
	ACampusARCardActor();

	virtual void Tick(float DeltaSeconds) override;

	void SetCampusCardScale(float NewScale);
	void AddYaw(float DeltaDegrees);
	void AddLocalOffsetFromGesture(const FVector2D& ScreenDelta);
	void ResetCard();
	bool HandleWorldTap(const FVector& RayStart, const FVector& RayEnd, bool* bOutResetRequested = nullptr);
	void SetProfilePhoto(UTexture2D* PhotoTexture);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	USceneComponent* SceneRoot;

	UPROPERTY()
	UProceduralMeshComponent* MainCardMesh;

	UPROPERTY()
	UProceduralMeshComponent* InfoPanelMesh;

	UPROPERTY()
	UProceduralMeshComponent* BaseMesh;

	UPROPERTY()
	UProceduralMeshComponent* LogoMesh;

	UPROPERTY()
	TArray<UTextRenderComponent*> PageTexts;

	UPROPERTY()
	TArray<USceneComponent*> ProfilePageComponents;

	UPROPERTY()
	TArray<USceneComponent*> WebsitePageComponents;

	UPROPERTY()
	TArray<USceneComponent*> EmblemPageComponents;

	UPROPERTY()
	TArray<USceneComponent*> CustomPageComponents;

	UPROPERTY()
	TArray<UTextRenderComponent*> MenuTexts;

	UPROPERTY()
	TArray<UBoxComponent*> MenuHitBoxes;

	UPROPERTY()
	USoundWaveProcedural* ClickSound;

	UPROPERTY()
	UWidgetComponent* PhotoWidgetComponent;

	FVector InitialLocation;
	FRotator InitialRotation;
	float InitialScale;
	float CurrentScale;
	ECampusCardPage CurrentPage;

	void BuildMeshes();
	void BuildTexts();
	void BuildMenu();
	void BuildPhotoWidget();
	void BuildClickSound();
	void CreatePlaneMesh(UProceduralMeshComponent* Mesh, float Width, float Height, const FLinearColor& Color, bool bVertical, int32 SectionIndex = 0);
	void CreateBoxMesh(UProceduralMeshComponent* Mesh, const FVector& Extents, const FLinearColor& Color, int32 SectionIndex = 0);
	void CreateCylinderMesh(UProceduralMeshComponent* Mesh, float Radius, float Height, const FLinearColor& Color, int32 Segments = 48, int32 SectionIndex = 0);
	UTextRenderComponent* AddText(const FString& Name, const FString& Text, const FVector& Location, float Size, const FLinearColor& Color);
	UBoxComponent* AddMenuButton(const FString& Label, const FVector& Location, int32 ButtonIndex);
	void SetPage(ECampusCardPage NewPage);
	void SetPageComponentsVisible(const TArray<USceneComponent*>& Components, bool bVisible);
	void RefreshPageText();
	void PlayIntroSound();
};
