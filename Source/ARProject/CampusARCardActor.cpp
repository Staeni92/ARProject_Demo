#include "CampusARCardActor.h"

#include "CampusPhotoWidget.h"
#include "CampusVideoWidget.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "MediaSoundComponent.h"
#include "ProceduralMeshComponent.h"
#include "Sound/SoundWaveProcedural.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FName ButtonTagProfile(TEXT("Profile"));
	const FName ButtonTagWebsite(TEXT("Website"));
	const FName ButtonTagEmblem(TEXT("Emblem"));
	const FName ButtonTagCustom(TEXT("Custom"));
	const FName ButtonTagReset(TEXT("Reset"));

	const FLinearColor CityBlue(0.02f, 0.24f, 0.52f, 1.0f);
	const FLinearColor CityLightBlue(0.58f, 0.78f, 0.96f, 1.0f);
	const FLinearColor CityWhite(0.96f, 0.98f, 1.0f, 1.0f);
	const FLinearColor CityInk(0.02f, 0.05f, 0.10f, 1.0f);

	UMaterialInterface* LoadSolidColorMaterial()
	{
		static UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		return Material;
	}

	void ApplySolidColorMaterial(UProceduralMeshComponent* Mesh, int32 SectionIndex, const FLinearColor& Color)
	{
		if (!Mesh)
		{
			return;
		}

		UMaterialInterface* BaseMaterial = LoadSolidColorMaterial();
		if (!BaseMaterial)
		{
			return;
		}

		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Mesh);
		if (!DynamicMaterial)
		{
			return;
		}

		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
		Mesh->SetMaterial(SectionIndex, DynamicMaterial);
	}
}

ACampusARCardActor::ACampusARCardActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	MainCardMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MainCampusCard"));
	MainCardMesh->SetupAttachment(SceneRoot);
	MainCardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InfoPanelMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("FloatingInfoPanel"));
	InfoPanelMesh->SetupAttachment(SceneRoot);
	InfoPanelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BaseMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("HologramBase"));
	BaseMesh->SetupAttachment(SceneRoot);
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LogoMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CampusLogo"));
	LogoMesh->SetupAttachment(SceneRoot);
	LogoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CurrentPage = ECampusCardPage::Profile;
	InitialScale = 1.0f;
	CurrentScale = 1.0f;
}

void ACampusARCardActor::BeginPlay()
{
	Super::BeginPlay();

	InitialLocation = GetActorLocation();
	InitialRotation = GetActorRotation();
	InitialScale = GetActorScale3D().X;
	CurrentScale = InitialScale;

	BuildMeshes();
	BuildTexts();
	BuildMenu();
	BuildPhotoWidget();
	BuildWebsiteWidget();
	BuildClickSound();
	RefreshPageText();
}

void ACampusARCardActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurrentPage == ECampusCardPage::Emblem)
	{
		LogoMesh->SetRelativeRotation(LogoMesh->GetRelativeRotation() + FRotator(0.0f, DeltaSeconds * 30.0f, 0.0f));
	}
}

void ACampusARCardActor::SetCampusCardScale(float NewScale)
{
	CurrentScale = FMath::Clamp(NewScale, 0.45f, 2.2f);
	SetActorScale3D(FVector(CurrentScale));
}

void ACampusARCardActor::AddYaw(float DeltaDegrees)
{
	AddActorWorldRotation(FRotator(0.0f, DeltaDegrees, 0.0f));
}

void ACampusARCardActor::AddLocalOffsetFromGesture(const FVector2D& ScreenDelta)
{
	const FVector Right = GetActorRightVector();
	const FVector Forward = GetActorForwardVector();
	const FVector Offset = (Right * ScreenDelta.X * 0.015f) + (Forward * -ScreenDelta.Y * 0.01f);
	AddActorWorldOffset(Offset, false);
}

void ACampusARCardActor::ResetCard()
{
	SetActorLocation(InitialLocation);
	SetActorRotation(InitialRotation);
	SetCampusCardScale(InitialScale);
	SetPage(ECampusCardPage::Profile);
}

bool ACampusARCardActor::HandleWorldTap(const FVector& RayStart, const FVector& RayEnd, bool* bOutResetRequested)
{
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CampusCardButtonTrace), true);
	if (bOutResetRequested)
	{
		*bOutResetRequested = false;
	}

	for (UBoxComponent* Button : MenuHitBoxes)
	{
		if (!Button)
		{
			continue;
		}

		FHitResult ButtonHit;
		if (Button->LineTraceComponent(ButtonHit, RayStart, RayEnd, Params))
		{
			const FName ButtonTag = Button->ComponentTags.Num() > 0 ? Button->ComponentTags[0] : NAME_None;
			if (ButtonTag == ButtonTagProfile)
			{
				SetPage(ECampusCardPage::Profile);
			}
			else if (ButtonTag == ButtonTagWebsite)
			{
				SetPage(ECampusCardPage::Website);
			}
			else if (ButtonTag == ButtonTagEmblem)
			{
				SetPage(ECampusCardPage::Emblem);
			}
			else if (ButtonTag == ButtonTagCustom)
			{
				SetPage(ECampusCardPage::Custom);
			}
			else if (ButtonTag == ButtonTagReset)
			{
				ResetCard();
				if (bOutResetRequested)
				{
					*bOutResetRequested = true;
				}
			}

			return true;
		}
	}

	return false;
}

void ACampusARCardActor::SetProfilePhoto(UTexture2D* PhotoTexture)
{
	if (!PhotoWidgetComponent || !PhotoTexture)
	{
		return;
	}

	if (UCampusPhotoWidget* PhotoWidget = Cast<UCampusPhotoWidget>(PhotoWidgetComponent->GetWidget()))
	{
		PhotoWidget->SetPhotoTexture(PhotoTexture);
	}
}

void ACampusARCardActor::BuildMeshes()
{
	CreateRoundedRectMesh(MainCardMesh, 136.0f, 82.0f, 5.5f, FLinearColor(0.985f, 0.965f, 0.985f, 1.0f), 16);
	MainCardMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.6f));

	CreateRoundedRectMesh(InfoPanelMesh, 126.0f, 72.0f, 4.5f, FLinearColor(0.99f, 0.985f, 0.99f, 1.0f), 16);
	InfoPanelMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 1.4f));
	ProfilePageComponents.Add(InfoPanelMesh);

	CreateRoundedRectMesh(BaseMesh, 137.0f, 83.0f, 5.8f, FLinearColor(0.18f, 0.17f, 0.18f, 1.0f), 16);
	BaseMesh->SetRelativeLocation(FVector(0.8f, 1.0f, 0.0f));

	CreateCylinderMesh(LogoMesh, 18.0f, 2.2f, CityBlue);
	LogoMesh->SetRelativeLocation(FVector(0.0f, -3.0f, 4.0f));
	EmblemPageComponents.Add(LogoMesh);

	auto AddFlatPanel = [this](const FString& Name, const FVector& Location, float Width, float Height, const FLinearColor& Color, TArray<USceneComponent*>& PageComponents)
	{
		UProceduralMeshComponent* PanelMesh = NewObject<UProceduralMeshComponent>(this, *Name);
		PanelMesh->SetupAttachment(SceneRoot);
		PanelMesh->RegisterComponent();
		PanelMesh->SetRelativeLocation(Location);
		PanelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CreatePlaneMesh(PanelMesh, Width, Height, Color, false);
		PageComponents.Add(PanelMesh);
		return PanelMesh;
	};

	AddFlatPanel(TEXT("ProfileLeftTint"), FVector(-43.0f, 2.0f, 1.82f), 42.0f, 68.0f, FLinearColor(0.96f, 0.91f, 0.95f, 1.0f), ProfilePageComponents);
	AddFlatPanel(TEXT("ProfileRightTint"), FVector(37.0f, 14.0f, 1.83f), 50.0f, 46.0f, FLinearColor(0.91f, 0.73f, 0.84f, 1.0f), ProfilePageComponents);
	AddFlatPanel(TEXT("ProfileCenterLight"), FVector(9.0f, -4.0f, 1.84f), 72.0f, 58.0f, FLinearColor(0.99f, 0.985f, 0.99f, 1.0f), ProfilePageComponents);
	AddFlatPanel(TEXT("ProfileTitleBar"), FVector(-55.0f, -27.0f, 2.0f), 4.6f, 11.5f, FLinearColor(0.45f, 0.13f, 0.27f, 1.0f), ProfilePageComponents);
	AddFlatPanel(TEXT("ProfilePhotoFrame"), FVector(-40.0f, 14.0f, 2.0f), 29.0f, 38.0f, FLinearColor(0.98f, 0.98f, 0.985f, 1.0f), ProfilePageComponents);
	AddFlatPanel(TEXT("ProfilePhotoMatte"), FVector(-40.0f, 14.0f, 2.04f), 25.5f, 34.0f, FLinearColor(0.92f, 0.92f, 0.93f, 1.0f), ProfilePageComponents);
	AddFlatPanel(TEXT("ProfileCityULogoPlate"), FVector(43.0f, -26.0f, 2.02f), 26.0f, 10.5f, FLinearColor(0.58f, 0.14f, 0.26f, 1.0f), ProfilePageComponents);
	AddFlatPanel(TEXT("ProfileCityUAccent"), FVector(33.0f, -25.2f, 2.04f), 8.0f, 6.6f, FLinearColor(0.78f, 0.33f, 0.24f, 1.0f), ProfilePageComponents);
	AddFlatPanel(TEXT("ProfileBottomLine"), FVector(0.0f, 39.2f, 2.0f), 122.0f, 0.7f, FLinearColor(0.18f, 0.17f, 0.18f, 1.0f), ProfilePageComponents);

	UProceduralMeshComponent* EmblemOuter = NewObject<UProceduralMeshComponent>(this, TEXT("EmblemOuterRing"));
	EmblemOuter->SetupAttachment(SceneRoot);
	EmblemOuter->RegisterComponent();
	EmblemOuter->SetRelativeLocation(FVector(0.0f, -3.0f, 3.4f));
	EmblemOuter->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CreateCylinderMesh(EmblemOuter, 22.0f, 1.1f, CityWhite);
	EmblemPageComponents.Add(EmblemOuter);

	AddFlatPanel(TEXT("EmblemLabelPanel"), FVector(0.0f, 25.0f, 1.9f), 62.0f, 13.0f, FLinearColor(1.0f, 1.0f, 1.0f, 1.0f), EmblemPageComponents);
	AddFlatPanel(TEXT("EmblemWaveA"), FVector(-4.5f, -1.0f, 5.4f), 20.0f, 3.0f, FLinearColor(0.72f, 0.86f, 0.96f, 1.0f), EmblemPageComponents);
	AddFlatPanel(TEXT("EmblemWaveB"), FVector(4.5f, 4.0f, 5.5f), 18.0f, 3.0f, FLinearColor(0.96f, 0.98f, 1.0f, 1.0f), EmblemPageComponents);
	AddFlatPanel(TEXT("CustomPanel"), FVector(0.0f, -1.0f, 1.8f), 90.0f, 44.0f, FLinearColor(0.98f, 0.99f, 1.0f, 1.0f), CustomPageComponents);
}

void ACampusARCardActor::BuildTexts()
{
	auto AddPageText = [this](const FString& Name, const FString& Text, const FVector& Location, float Size, const FLinearColor& Color, TArray<USceneComponent*>& PageComponents)
	{
		UTextRenderComponent* TextComponent = AddText(Name, Text, Location, Size, Color);
		PageComponents.Add(TextComponent);
		return TextComponent;
	};

	AddPageText(TEXT("ProfileStudentLabel"), TEXT("STUDENT"), FVector(-49.0f, -30.0f, 3.0f), 4.1f, FLinearColor(0.45f, 0.13f, 0.27f, 1.0f), ProfilePageComponents);
	AddPageText(TEXT("ProfileIdentityLabel"), TEXT("IDENTITY CARD"), FVector(-49.0f, -23.2f, 3.0f), 3.8f, FLinearColor(0.15f, 0.16f, 0.18f, 1.0f), ProfilePageComponents);
	AddPageText(TEXT("ProfileCityULogo"), TEXT("CityU"), FVector(33.2f, -27.8f, 3.0f), 4.3f, CityWhite, ProfilePageComponents);
	AddPageText(TEXT("ProfileUniversityCn"), TEXT("香港城市大学（东莞）"), FVector(27.0f, -16.2f, 3.0f), 1.7f, FLinearColor(0.28f, 0.08f, 0.14f, 1.0f), ProfilePageComponents);
	AddPageText(TEXT("ProfileUniversityEn"), TEXT("City University of Hong Kong\n(Dongguan)"), FVector(27.0f, -12.8f, 3.0f), 1.35f, FLinearColor(0.35f, 0.07f, 0.14f, 1.0f), ProfilePageComponents);
	AddPageText(TEXT("ProfileName"), TEXT("Biaoyao ZHANG"), FVector(-4.0f, 3.0f, 3.0f), 4.0f, FLinearColor(0.13f, 0.14f, 0.16f, 1.0f), ProfilePageComponents);
	AddPageText(TEXT("ProfileNameCn"), TEXT("张飙垚"), FVector(-4.0f, 12.0f, 3.0f), 4.3f, FLinearColor(0.13f, 0.14f, 0.16f, 1.0f), ProfilePageComponents);
	AddPageText(TEXT("ProfileStudentId"), TEXT("72404867"), FVector(-4.0f, 25.8f, 3.0f), 3.6f, FLinearColor(0.13f, 0.14f, 0.16f, 1.0f), ProfilePageComponents);
	AddPageText(TEXT("ProfileDegree"), TEXT("Postgraduate"), FVector(-4.0f, 34.2f, 3.0f), 3.2f, FLinearColor(0.13f, 0.14f, 0.16f, 1.0f), ProfilePageComponents);

	AddPageText(TEXT("EmblemTitle"), TEXT("School Emblem Model"), FVector(-27.0f, 22.0f, 3.0f), 3.0f, CityBlue, EmblemPageComponents);
	AddPageText(TEXT("EmblemText"), TEXT("Use two fingers to scale or rotate the AR emblem."), FVector(-28.0f, 28.0f, 3.0f), 1.9f, CityInk, EmblemPageComponents);
	AddPageText(TEXT("EmblemCoreText"), TEXT("CITYUHK\nDG"), FVector(-7.5f, -9.0f, 6.0f), 3.0f, CityWhite, EmblemPageComponents);

	AddPageText(TEXT("CustomTitle"), TEXT("Custom Page"), FVector(-33.0f, -14.0f, 3.0f), 4.0f, CityBlue, CustomPageComponents);
	AddPageText(TEXT("CustomPlaceholder"), TEXT("Reserved for personal content,\nportfolio media, video, links,\nor future project display."), FVector(-33.0f, -3.0f, 3.0f), 2.5f, CityInk, CustomPageComponents);
}

void ACampusARCardActor::BuildMenu()
{
	MenuHitBoxes.Add(AddMenuButton(TEXT("Profile"), FVector(-76.0f, -18.0f, 3.0f), 0));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagProfile);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Website"), FVector(-76.0f, -6.5f, 3.0f), 1));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagWebsite);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Emblem"), FVector(-76.0f, 5.0f, 3.0f), 2));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagEmblem);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Custom"), FVector(-76.0f, 16.5f, 3.0f), 3));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagCustom);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Reset"), FVector(-76.0f, 30.0f, 3.0f), 4));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagReset);
}

void ACampusARCardActor::BuildPhotoWidget()
{
	PhotoWidgetComponent = NewObject<UWidgetComponent>(this, TEXT("ProfilePhotoWidget"));
	PhotoWidgetComponent->SetupAttachment(SceneRoot);
	PhotoWidgetComponent->RegisterComponent();
	PhotoWidgetComponent->SetWidgetClass(UCampusPhotoWidget::StaticClass());
	PhotoWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	PhotoWidgetComponent->SetDrawSize(FVector2D(320.0f, 420.0f));
	PhotoWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	PhotoWidgetComponent->SetTwoSided(true);
	PhotoWidgetComponent->SetRelativeLocation(FVector(-40.0f, 14.0f, 3.0f));
	PhotoWidgetComponent->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	PhotoWidgetComponent->SetRelativeScale3D(FVector(0.080f));
	PhotoWidgetComponent->InitWidget();
	ProfilePageComponents.Add(PhotoWidgetComponent);
}

void ACampusARCardActor::BuildWebsiteWidget()
{
	WebsiteWidgetComponent = NewObject<UWidgetComponent>(this, TEXT("WebsiteVideoWidget"));
	WebsiteWidgetComponent->SetupAttachment(SceneRoot);
	WebsiteWidgetComponent->RegisterComponent();
	WebsiteWidgetComponent->SetWidgetClass(UCampusVideoWidget::StaticClass());
	WebsiteWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	WebsiteWidgetComponent->SetDrawSize(FVector2D(1600.0f, 900.0f));
	WebsiteWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	WebsiteWidgetComponent->SetTwoSided(true);
	WebsiteWidgetComponent->SetRelativeLocation(FVector(4.0f, -1.0f, 3.35f));
	WebsiteWidgetComponent->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	WebsiteWidgetComponent->SetRelativeScale3D(FVector(0.0775f));
	WebsiteWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WebsiteWidgetComponent->InitWidget();

	WebsiteMediaSoundComponent = NewObject<UMediaSoundComponent>(this, TEXT("WebsiteMediaSoundComponent"));
	WebsiteMediaSoundComponent->SetupAttachment(WebsiteWidgetComponent);
	WebsiteMediaSoundComponent->bAutoActivate = false;
	WebsiteMediaSoundComponent->bAllowSpatialization = false;
	WebsiteMediaSoundComponent->bIsUISound = true;
	WebsiteMediaSoundComponent->SetVolumeMultiplier(1.0f);
	if (UCampusVideoWidget* VideoWidget = Cast<UCampusVideoWidget>(WebsiteWidgetComponent->GetUserWidgetObject()))
	{
		WebsiteMediaSoundComponent->SetMediaPlayer(VideoWidget->GetMediaPlayer());
	}
	WebsiteMediaSoundComponent->RegisterComponent();

	WebsitePageComponents.Add(WebsiteWidgetComponent);
}

void ACampusARCardActor::BuildClickSound()
{
	constexpr int32 SampleRate = 44100;
	constexpr float DurationSeconds = 0.55f;
	const int32 NumSamples = static_cast<int32>(SampleRate * DurationSeconds);

	TArray<int16> Samples;
	Samples.SetNumUninitialized(NumSamples);
	for (int32 Index = 0; Index < NumSamples; ++Index)
	{
		const float T = static_cast<float>(Index) / SampleRate;
		const float Envelope = FMath::Clamp(1.0f - (T / DurationSeconds), 0.0f, 1.0f);
		const float Tone = FMath::Sin(2.0f * PI * 660.0f * T) * 0.6f + FMath::Sin(2.0f * PI * 990.0f * T) * 0.25f;
		Samples[Index] = static_cast<int16>(Tone * Envelope * 12000.0f);
	}

	ClickSound = NewObject<USoundWaveProcedural>(this, TEXT("CampusCardGeneratedIntroTone"));
	ClickSound->SetSampleRate(SampleRate);
	ClickSound->NumChannels = 1;
	ClickSound->Duration = DurationSeconds;
	ClickSound->SampleByteSize = sizeof(int16);
	ClickSound->bLooping = false;
	ClickSound->QueueAudio(reinterpret_cast<const uint8*>(Samples.GetData()), Samples.Num() * sizeof(int16));
}

void ACampusARCardActor::CreatePlaneMesh(UProceduralMeshComponent* Mesh, float Width, float Height, const FLinearColor& Color, bool bVertical, int32 SectionIndex)
{
	if (!Mesh)
	{
		return;
	}

	const float HalfW = Width * 0.5f;
	const float HalfH = Height * 0.5f;

	TArray<FVector> Vertices;
	if (bVertical)
	{
		Vertices = { FVector(-HalfW, 0.0f, -HalfH), FVector(HalfW, 0.0f, -HalfH), FVector(HalfW, 0.0f, HalfH), FVector(-HalfW, 0.0f, HalfH) };
	}
	else
	{
		Vertices = { FVector(-HalfW, -HalfH, 0.0f), FVector(HalfW, -HalfH, 0.0f), FVector(HalfW, HalfH, 0.0f), FVector(-HalfW, HalfH, 0.0f) };
	}

	const TArray<int32> Triangles = { 0, 2, 1, 0, 3, 2 };
	const FVector Normal = bVertical ? FVector::YAxisVector : FVector::UpVector;
	const TArray<FVector> Normals = { Normal, Normal, Normal, Normal };
	const TArray<FVector2D> UVs = { FVector2D(0.0f, 0.0f), FVector2D(1.0f, 0.0f), FVector2D(1.0f, 1.0f), FVector2D(0.0f, 1.0f) };
	const TArray<FColor> Colors = { Color.ToFColor(true), Color.ToFColor(true), Color.ToFColor(true), Color.ToFColor(true) };
	const TArray<FProcMeshTangent> Tangents = { FProcMeshTangent(1.0f, 0.0f, 0.0f), FProcMeshTangent(1.0f, 0.0f, 0.0f), FProcMeshTangent(1.0f, 0.0f, 0.0f), FProcMeshTangent(1.0f, 0.0f, 0.0f) };

	Mesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UVs, Colors, Tangents, false);
	ApplySolidColorMaterial(Mesh, SectionIndex, Color);
}

void ACampusARCardActor::CreateBoxMesh(UProceduralMeshComponent* Mesh, const FVector& Extents, const FLinearColor& Color, int32 SectionIndex)
{
	if (!Mesh)
	{
		return;
	}

	const FVector E = Extents;
	TArray<FVector> V = {
		FVector(-E.X, -E.Y, -E.Z), FVector(E.X, -E.Y, -E.Z), FVector(E.X, E.Y, -E.Z), FVector(-E.X, E.Y, -E.Z),
		FVector(-E.X, -E.Y, E.Z), FVector(E.X, -E.Y, E.Z), FVector(E.X, E.Y, E.Z), FVector(-E.X, E.Y, E.Z)
	};
	TArray<int32> T = {
		0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6,
		0, 4, 5, 0, 5, 1, 1, 5, 6, 1, 6, 2,
		2, 6, 7, 2, 7, 3, 3, 7, 4, 3, 4, 0
	};
	TArray<FVector> N;
	TArray<FVector2D> UV;
	TArray<FColor> C;
	TArray<FProcMeshTangent> Tangents;
	for (int32 Index = 0; Index < V.Num(); ++Index)
	{
		N.Add(FVector::UpVector);
		UV.Add(FVector2D::ZeroVector);
		C.Add(Color.ToFColor(true));
		Tangents.Add(FProcMeshTangent(1.0f, 0.0f, 0.0f));
	}

	Mesh->CreateMeshSection(SectionIndex, V, T, N, UV, C, Tangents, false);
	ApplySolidColorMaterial(Mesh, SectionIndex, Color);
}

void ACampusARCardActor::CreateCylinderMesh(UProceduralMeshComponent* Mesh, float Radius, float Height, const FLinearColor& Color, int32 Segments, int32 SectionIndex)
{
	if (!Mesh)
	{
		return;
	}

	const int32 ClampedSegments = FMath::Max(Segments, 12);
	const float HalfHeight = Height * 0.5f;
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV;
	TArray<FColor> Colors;
	TArray<FProcMeshTangent> Tangents;

	Vertices.Add(FVector(0.0f, 0.0f, HalfHeight));
	Vertices.Add(FVector(0.0f, 0.0f, -HalfHeight));

	for (int32 Index = 0; Index < ClampedSegments; ++Index)
	{
		const float Angle = 2.0f * PI * static_cast<float>(Index) / static_cast<float>(ClampedSegments);
		const float X = FMath::Cos(Angle) * Radius;
		const float Y = FMath::Sin(Angle) * Radius;
		Vertices.Add(FVector(X, Y, HalfHeight));
		Vertices.Add(FVector(X, Y, -HalfHeight));
	}

	for (int32 Index = 0; Index < ClampedSegments; ++Index)
	{
		const int32 Next = (Index + 1) % ClampedSegments;
		const int32 TopA = 2 + Index * 2;
		const int32 BottomA = TopA + 1;
		const int32 TopB = 2 + Next * 2;
		const int32 BottomB = TopB + 1;

		Triangles.Add(0);
		Triangles.Add(TopA);
		Triangles.Add(TopB);
		Triangles.Add(1);
		Triangles.Add(BottomB);
		Triangles.Add(BottomA);
		Triangles.Add(TopA);
		Triangles.Add(BottomA);
		Triangles.Add(BottomB);
		Triangles.Add(TopA);
		Triangles.Add(BottomB);
		Triangles.Add(TopB);
	}

	for (int32 Index = 0; Index < Vertices.Num(); ++Index)
	{
		Normals.Add(FVector::UpVector);
		UV.Add(FVector2D::ZeroVector);
		Colors.Add(Color.ToFColor(true));
		Tangents.Add(FProcMeshTangent(1.0f, 0.0f, 0.0f));
	}

	Mesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UV, Colors, Tangents, false);
	ApplySolidColorMaterial(Mesh, SectionIndex, Color);
}

void ACampusARCardActor::CreateRoundedRectMesh(UProceduralMeshComponent* Mesh, float Width, float Height, float Radius, const FLinearColor& Color, int32 SegmentsPerCorner, int32 SectionIndex)
{
	if (!Mesh)
	{
		return;
	}

	const float HalfW = Width * 0.5f;
	const float HalfH = Height * 0.5f;
	const float ClampedRadius = FMath::Clamp(Radius, 0.0f, FMath::Min(HalfW, HalfH));
	const int32 ClampedSegments = FMath::Max(SegmentsPerCorner, 3);

	TArray<FVector> Vertices;
	Vertices.Add(FVector::ZeroVector);

	auto AddCorner = [&Vertices, ClampedRadius, ClampedSegments](const FVector2D& Center, float StartDegrees, float EndDegrees)
	{
		for (int32 SegmentIndex = 0; SegmentIndex <= ClampedSegments; ++SegmentIndex)
		{
			const float Alpha = static_cast<float>(SegmentIndex) / static_cast<float>(ClampedSegments);
			const float AngleRadians = FMath::DegreesToRadians(FMath::Lerp(StartDegrees, EndDegrees, Alpha));
			Vertices.Add(FVector(
				Center.X + FMath::Cos(AngleRadians) * ClampedRadius,
				Center.Y + FMath::Sin(AngleRadians) * ClampedRadius,
				0.0f));
		}
	};

	AddCorner(FVector2D(HalfW - ClampedRadius, HalfH - ClampedRadius), 0.0f, 90.0f);
	AddCorner(FVector2D(-HalfW + ClampedRadius, HalfH - ClampedRadius), 90.0f, 180.0f);
	AddCorner(FVector2D(-HalfW + ClampedRadius, -HalfH + ClampedRadius), 180.0f, 270.0f);
	AddCorner(FVector2D(HalfW - ClampedRadius, -HalfH + ClampedRadius), 270.0f, 360.0f);

	TArray<int32> Triangles;
	for (int32 VertexIndex = 1; VertexIndex < Vertices.Num(); ++VertexIndex)
	{
		const int32 NextVertexIndex = VertexIndex == Vertices.Num() - 1 ? 1 : VertexIndex + 1;
		Triangles.Add(0);
		Triangles.Add(NextVertexIndex);
		Triangles.Add(VertexIndex);
	}

	TArray<FVector> Normals;
	TArray<FVector2D> UV;
	TArray<FColor> Colors;
	TArray<FProcMeshTangent> Tangents;
	for (const FVector& Vertex : Vertices)
	{
		Normals.Add(FVector::UpVector);
		UV.Add(FVector2D((Vertex.X + HalfW) / Width, (Vertex.Y + HalfH) / Height));
		Colors.Add(Color.ToFColor(true));
		Tangents.Add(FProcMeshTangent(1.0f, 0.0f, 0.0f));
	}

	Mesh->CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UV, Colors, Tangents, false);
	ApplySolidColorMaterial(Mesh, SectionIndex, Color);
}

UTextRenderComponent* ACampusARCardActor::AddText(const FString& Name, const FString& Text, const FVector& Location, float Size, const FLinearColor& Color)
{
	UTextRenderComponent* TextComponent = NewObject<UTextRenderComponent>(this, *Name);
	TextComponent->SetupAttachment(SceneRoot);
	TextComponent->RegisterComponent();
	TextComponent->SetText(FText::FromString(Text));
	TextComponent->SetRelativeLocation(Location);
	TextComponent->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	TextComponent->SetWorldSize(Size);
	TextComponent->SetTextRenderColor(Color.ToFColor(true));
	TextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Left);
	TextComponent->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	return TextComponent;
}

UBoxComponent* ACampusARCardActor::AddMenuButton(const FString& Label, const FVector& Location, int32 ButtonIndex)
{
	UProceduralMeshComponent* ButtonMesh = NewObject<UProceduralMeshComponent>(this, *FString::Printf(TEXT("ButtonMesh_%s"), *Label));
	ButtonMesh->SetupAttachment(SceneRoot);
	ButtonMesh->RegisterComponent();
	ButtonMesh->SetRelativeLocation(Location);
	ButtonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CreateRoundedRectMesh(ButtonMesh, 16.8f, 7.2f, 3.6f, FLinearColor::White, 12, ButtonIndex);

	UTextRenderComponent* MenuText = AddText(FString::Printf(TEXT("Menu_%s"), *Label), Label, Location + FVector(-5.8f, 0.5f, 0.85f), 1.45f, FLinearColor::Black);
	MenuTexts.Add(MenuText);

	UBoxComponent* HitBox = NewObject<UBoxComponent>(this, *FString::Printf(TEXT("Hit_%s"), *Label));
	HitBox->SetupAttachment(SceneRoot);
	HitBox->RegisterComponent();
	HitBox->SetRelativeLocation(Location);
	HitBox->SetBoxExtent(FVector(8.8f, 4.0f, 2.0f));
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitBox->SetCollisionObjectType(ECC_WorldDynamic);
	HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	return HitBox;
}

void ACampusARCardActor::SetPage(ECampusCardPage NewPage)
{
	CurrentPage = NewPage;
	RefreshPageText();
}

void ACampusARCardActor::SetPageComponentsVisible(const TArray<USceneComponent*>& Components, bool bVisible)
{
	for (USceneComponent* Component : Components)
	{
		if (!Component)
		{
			continue;
		}

		Component->SetVisibility(bVisible, true);
		Component->SetHiddenInGame(!bVisible, true);
	}
}

void ACampusARCardActor::RefreshPageText()
{
	const bool bShowCardBase = CurrentPage != ECampusCardPage::Website;
	MainCardMesh->SetVisibility(bShowCardBase, true);
	MainCardMesh->SetHiddenInGame(!bShowCardBase, true);
	BaseMesh->SetVisibility(bShowCardBase, true);
	BaseMesh->SetHiddenInGame(!bShowCardBase, true);

	SetPageComponentsVisible(ProfilePageComponents, CurrentPage == ECampusCardPage::Profile);
	SetPageComponentsVisible(WebsitePageComponents, CurrentPage == ECampusCardPage::Website);
	SetPageComponentsVisible(EmblemPageComponents, CurrentPage == ECampusCardPage::Emblem);
	SetPageComponentsVisible(CustomPageComponents, CurrentPage == ECampusCardPage::Custom);

	if (WebsiteMediaSoundComponent)
	{
		if (CurrentPage == ECampusCardPage::Website)
		{
			WebsiteMediaSoundComponent->Activate(true);
			if (UCampusVideoWidget* VideoWidget = Cast<UCampusVideoWidget>(WebsiteWidgetComponent->GetUserWidgetObject()))
			{
				VideoWidget->OpenMedia();
			}
		}
		else
		{
			if (WebsiteWidgetComponent)
			{
				if (UCampusVideoWidget* VideoWidget = Cast<UCampusVideoWidget>(WebsiteWidgetComponent->GetUserWidgetObject()))
				{
					VideoWidget->CloseMedia();
				}
			}
			WebsiteMediaSoundComponent->Deactivate();
		}
	}
}

void ACampusARCardActor::PlayIntroSound()
{
	if (!ClickSound)
	{
		BuildClickSound();
	}

	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ClickSound, 0.8f);
		BuildClickSound();
	}
}
