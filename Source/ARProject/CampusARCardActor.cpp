#include "CampusARCardActor.h"

#include "CampusPhotoWidget.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
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

	UMaterialInterface* LoadVertexColorMaterial()
	{
		static UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineDebugMaterials/VertexColorMaterial.VertexColorMaterial"));
		return Material;
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
	CreatePlaneMesh(MainCardMesh, 136.0f, 82.0f, FLinearColor(0.88f, 0.93f, 0.98f, 1.0f), false);
	MainCardMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.6f));

	CreatePlaneMesh(InfoPanelMesh, 78.0f, 48.0f, CityWhite, false);
	InfoPanelMesh->SetRelativeLocation(FVector(14.0f, -2.0f, 1.4f));
	ProfilePageComponents.Add(InfoPanelMesh);

	CreateBoxMesh(BaseMesh, FVector(66.0f, 40.0f, 0.45f), FLinearColor(0.72f, 0.82f, 0.91f, 1.0f));
	BaseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

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

	AddFlatPanel(TEXT("ProfilePhotoFrame"), FVector(-34.0f, -6.0f, 1.9f), 26.0f, 32.0f, FLinearColor(0.90f, 0.94f, 0.98f, 1.0f), ProfilePageComponents);
	AddFlatPanel(TEXT("ProfileInfoCard"), FVector(19.0f, -5.0f, 1.9f), 45.0f, 32.0f, FLinearColor(0.98f, 0.99f, 1.0f, 1.0f), ProfilePageComponents);
	AddFlatPanel(TEXT("ProfileBanner"), FVector(0.0f, -29.0f, 1.9f), 110.0f, 8.0f, CityBlue, ProfilePageComponents);

	AddFlatPanel(TEXT("WebsiteTopNav"), FVector(26.0f, -23.5f, 1.8f), 78.0f, 5.5f, CityBlue, WebsitePageComponents);
	AddFlatPanel(TEXT("WebsiteHero"), FVector(17.0f, -6.0f, 1.9f), 44.0f, 18.0f, CityLightBlue, WebsitePageComponents);
	AddFlatPanel(TEXT("WebsiteHeroImage"), FVector(17.0f, -6.0f, 2.0f), 24.0f, 12.0f, FLinearColor(0.12f, 0.42f, 0.78f, 1.0f), WebsitePageComponents);
	AddFlatPanel(TEXT("WebsiteApplyCard"), FVector(49.0f, -6.0f, 1.9f), 20.0f, 18.0f, FLinearColor(0.92f, 0.96f, 1.0f, 1.0f), WebsitePageComponents);
	AddFlatPanel(TEXT("WebsiteAboutCard"), FVector(15.0f, 13.0f, 1.9f), 36.0f, 15.0f, FLinearColor(0.98f, 0.99f, 1.0f, 1.0f), WebsitePageComponents);
	AddFlatPanel(TEXT("WebsiteRankCardA"), FVector(43.0f, 10.0f, 1.9f), 18.0f, 10.0f, FLinearColor(0.89f, 0.94f, 1.0f, 1.0f), WebsitePageComponents);
	AddFlatPanel(TEXT("WebsiteRankCardB"), FVector(43.0f, 22.0f, 1.9f), 18.0f, 10.0f, FLinearColor(0.89f, 0.94f, 1.0f, 1.0f), WebsitePageComponents);
	AddFlatPanel(TEXT("LeftPreviewShip"), FVector(-48.0f, -17.0f, 1.7f), 27.0f, 16.0f, FLinearColor(0.06f, 0.34f, 0.68f, 1.0f), WebsitePageComponents);
	AddFlatPanel(TEXT("LeftPreviewCampus"), FVector(-48.0f, 4.0f, 1.7f), 27.0f, 16.0f, FLinearColor(0.28f, 0.60f, 0.88f, 1.0f), WebsitePageComponents);
	AddFlatPanel(TEXT("LeftPreviewResearch"), FVector(-48.0f, 25.0f, 1.7f), 27.0f, 16.0f, FLinearColor(0.78f, 0.88f, 0.97f, 1.0f), WebsitePageComponents);

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

	AddPageText(TEXT("ProfileTitle"), TEXT("Student Profile"), FVector(-47.0f, -30.0f, 3.0f), 3.6f, CityWhite, ProfilePageComponents);
	AddPageText(TEXT("ProfileName"), TEXT("Name: Gavin Cheung"), FVector(0.0f, -16.5f, 3.0f), 3.3f, CityBlue, ProfilePageComponents);
	AddPageText(TEXT("ProfileStudentId"), TEXT("Student ID: 2026 AR Demo"), FVector(0.0f, -8.5f, 3.0f), 2.4f, CityInk, ProfilePageComponents);
	AddPageText(TEXT("ProfileDegree"), TEXT("Level: Postgraduate"), FVector(0.0f, -2.5f, 3.0f), 2.4f, CityInk, ProfilePageComponents);
	AddPageText(TEXT("ProfileSchool"), TEXT("City University of Hong Kong (Dongguan)"), FVector(0.0f, 4.0f, 3.0f), 2.1f, CityInk, ProfilePageComponents);
	AddPageText(TEXT("ProfileHint"), TEXT("Tap the buttons below to switch AR content."), FVector(-46.0f, 23.0f, 3.0f), 2.0f, FLinearColor(0.20f, 0.28f, 0.38f), ProfilePageComponents);

	AddPageText(TEXT("WebsiteBrand"), TEXT("City University of Hong Kong (Dongguan)"), FVector(-53.0f, -35.0f, 3.0f), 3.6f, CityBlue, WebsitePageComponents);
	AddPageText(TEXT("WebsiteUrl"), TEXT("www.cityu-dg.edu.cn/en/home.html"), FVector(-18.0f, -29.8f, 3.0f), 2.0f, FLinearColor(0.18f, 0.26f, 0.38f), WebsitePageComponents);
	AddPageText(TEXT("WebsiteNav"), TEXT("Home    About Us    Academic    Research    Campus    Contact"), FVector(-6.0f, -23.8f, 3.1f), 1.8f, CityWhite, WebsitePageComponents);
	AddPageText(TEXT("LeftLabelShip"), TEXT("News"), FVector(-59.0f, -13.0f, 3.0f), 2.4f, CityWhite, WebsitePageComponents);
	AddPageText(TEXT("LeftLabelCampus"), TEXT("Life at CityUHK (DG)"), FVector(-59.0f, 8.0f, 3.0f), 2.0f, CityWhite, WebsitePageComponents);
	AddPageText(TEXT("LeftLabelResearch"), TEXT("Research"), FVector(-59.0f, 29.0f, 3.0f), 2.2f, CityBlue, WebsitePageComponents);
	AddPageText(TEXT("HeroTitle"), TEXT("Thrive at the Heart of the GBA"), FVector(-2.5f, -12.0f, 3.0f), 2.5f, CityWhite, WebsitePageComponents);
	AddPageText(TEXT("HeroSubtitle"), TEXT("A modern research university in Dongguan"), FVector(-2.5f, -7.8f, 3.0f), 1.6f, CityWhite, WebsitePageComponents);
	AddPageText(TEXT("ApplyNow"), TEXT("Apply Now"), FVector(42.0f, -9.0f, 3.0f), 3.0f, CityBlue, WebsitePageComponents);
	AddPageText(TEXT("ApplyText"), TEXT("Admissions and\nprogram updates"), FVector(41.0f, -3.5f, 3.0f), 1.6f, CityInk, WebsitePageComponents);
	AddPageText(TEXT("AboutPreview"), TEXT("About Us"), FVector(0.0f, 9.0f, 3.0f), 2.4f, CityBlue, WebsitePageComponents);
	AddPageText(TEXT("AboutPreviewText"), TEXT("CityUHK (DG) connects global education,\ninnovation, and the Greater Bay Area."), FVector(0.0f, 14.2f, 3.0f), 1.5f, CityInk, WebsitePageComponents);
	AddPageText(TEXT("RankingA"), TEXT("#54\nBest Global\nUniversities"), FVector(36.0f, 7.8f, 3.0f), 1.75f, CityBlue, WebsitePageComponents);
	AddPageText(TEXT("RankingB"), TEXT("#7\nQS Asia University\nRankings"), FVector(36.0f, 19.8f, 3.0f), 1.75f, CityBlue, WebsitePageComponents);

	AddPageText(TEXT("EmblemTitle"), TEXT("School Emblem Model"), FVector(-27.0f, 22.0f, 3.0f), 3.0f, CityBlue, EmblemPageComponents);
	AddPageText(TEXT("EmblemText"), TEXT("Use two fingers to scale or rotate the AR emblem."), FVector(-28.0f, 28.0f, 3.0f), 1.9f, CityInk, EmblemPageComponents);
	AddPageText(TEXT("EmblemCoreText"), TEXT("CITYUHK\nDG"), FVector(-7.5f, -9.0f, 6.0f), 3.0f, CityWhite, EmblemPageComponents);

	AddPageText(TEXT("CustomTitle"), TEXT("Custom Page"), FVector(-33.0f, -14.0f, 3.0f), 4.0f, CityBlue, CustomPageComponents);
	AddPageText(TEXT("CustomPlaceholder"), TEXT("Reserved for personal content,\nportfolio media, video, links,\nor future project display."), FVector(-33.0f, -3.0f, 3.0f), 2.5f, CityInk, CustomPageComponents);
}

void ACampusARCardActor::BuildMenu()
{
	MenuHitBoxes.Add(AddMenuButton(TEXT("Profile"), FVector(-34.0f, 36.0f, 3.0f), 0));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagProfile);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Website"), FVector(-14.0f, 36.0f, 3.0f), 1));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagWebsite);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Emblem"), FVector(6.0f, 36.0f, 3.0f), 2));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagEmblem);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Custom"), FVector(26.0f, 36.0f, 3.0f), 3));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagCustom);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Reset"), FVector(52.0f, 36.0f, 3.0f), 4));
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
	PhotoWidgetComponent->SetRelativeLocation(FVector(-34.0f, -6.0f, 3.0f));
	PhotoWidgetComponent->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	PhotoWidgetComponent->SetRelativeScale3D(FVector(0.075f));
	PhotoWidgetComponent->InitWidget();
	ProfilePageComponents.Add(PhotoWidgetComponent);
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
	if (UMaterialInterface* Material = LoadVertexColorMaterial())
	{
		Mesh->SetMaterial(SectionIndex, Material);
	}
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
	if (UMaterialInterface* Material = LoadVertexColorMaterial())
	{
		Mesh->SetMaterial(SectionIndex, Material);
	}
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
	if (UMaterialInterface* Material = LoadVertexColorMaterial())
	{
		Mesh->SetMaterial(SectionIndex, Material);
	}
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
	ButtonMesh->SetRelativeLocation(Location + FVector(2.0f, -1.0f, 0.0f));
	ButtonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CreateBoxMesh(ButtonMesh, FVector(7.6f, 3.2f, 0.45f), FLinearColor(1.0f, 1.0f, 1.0f, 1.0f), ButtonIndex);

	UTextRenderComponent* MenuText = AddText(FString::Printf(TEXT("Menu_%s"), *Label), Label, Location + FVector(-5.4f, 0.0f, 0.75f), 1.9f, CityBlue);
	MenuTexts.Add(MenuText);

	UBoxComponent* HitBox = NewObject<UBoxComponent>(this, *FString::Printf(TEXT("Hit_%s"), *Label));
	HitBox->SetupAttachment(SceneRoot);
	HitBox->RegisterComponent();
	HitBox->SetRelativeLocation(Location + FVector(2.0f, -1.0f, 0.0f));
	HitBox->SetBoxExtent(FVector(8.0f, 4.0f, 2.0f));
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
	SetPageComponentsVisible(ProfilePageComponents, CurrentPage == ECampusCardPage::Profile);
	SetPageComponentsVisible(WebsitePageComponents, CurrentPage == ECampusCardPage::Website);
	SetPageComponentsVisible(EmblemPageComponents, CurrentPage == ECampusCardPage::Emblem);
	SetPageComponentsVisible(CustomPageComponents, CurrentPage == ECampusCardPage::Custom);
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
