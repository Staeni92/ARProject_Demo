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
	const FName ButtonTagSkills(TEXT("Skills"));
	const FName ButtonTagProjects(TEXT("Projects"));
	const FName ButtonTagContact(TEXT("Contact"));
	const FName ButtonTagAudio(TEXT("Audio"));
	const FName ButtonTagReset(TEXT("Reset"));

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

	const float Pulse = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * 2.5f);
	LogoMesh->SetRelativeRotation(LogoMesh->GetRelativeRotation() + FRotator(0.0f, DeltaSeconds * 35.0f, 0.0f));
	InfoPanelMesh->SetRelativeLocation(FVector(0.0f, -2.0f, 64.0f + Pulse * 2.0f));
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

bool ACampusARCardActor::HandleWorldTap(const FVector& RayStart, const FVector& RayEnd)
{
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CampusCardButtonTrace), true);

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
			else if (ButtonTag == ButtonTagSkills)
			{
				SetPage(ECampusCardPage::Skills);
			}
			else if (ButtonTag == ButtonTagProjects)
			{
				SetPage(ECampusCardPage::Projects);
			}
			else if (ButtonTag == ButtonTagContact)
			{
				SetPage(ECampusCardPage::Contact);
			}
			else if (ButtonTag == ButtonTagAudio)
			{
				PlayIntroSound();
			}
			else if (ButtonTag == ButtonTagReset)
			{
				ResetCard();
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
	CreatePlaneMesh(MainCardMesh, 72.0f, 44.0f, FLinearColor(0.02f, 0.75f, 1.0f, 1.0f), false);
	MainCardMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 2.0f));

	CreatePlaneMesh(InfoPanelMesh, 86.0f, 52.0f, FLinearColor(0.0f, 0.25f, 0.35f, 1.0f), true);

	CreateBoxMesh(BaseMesh, FVector(42.0f, 8.0f, 2.0f), FLinearColor(0.0f, 0.85f, 1.0f, 1.0f));
	BaseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));

	CreateBoxMesh(LogoMesh, FVector(8.0f, 8.0f, 8.0f), FLinearColor(0.0f, 1.0f, 0.85f, 1.0f));
	LogoMesh->SetRelativeLocation(FVector(-31.0f, -4.0f, 70.0f));
}

void ACampusARCardActor::BuildTexts()
{
	AddText(TEXT("CardTitle"), TEXT("AR CAMPUS ID"), FVector(-31.0f, -23.0f, 5.0f), 4.2f, FLinearColor(0.0f, 1.0f, 0.95f));
	AddText(TEXT("CardSubtitle"), TEXT("Scan-ready profile card"), FVector(-31.0f, -16.5f, 5.0f), 2.3f, FLinearColor(0.75f, 0.95f, 1.0f));

	PageTexts.Add(AddText(TEXT("PageHeader"), TEXT(""), FVector(-22.0f, -5.0f, 67.0f), 4.0f, FLinearColor(0.0f, 1.0f, 0.95f)));
	PageTexts.Add(AddText(TEXT("PageLine1"), TEXT(""), FVector(-22.0f, -5.0f, 59.0f), 2.8f, FLinearColor(0.9f, 1.0f, 1.0f)));
	PageTexts.Add(AddText(TEXT("PageLine2"), TEXT(""), FVector(-22.0f, -5.0f, 53.0f), 2.8f, FLinearColor(0.9f, 1.0f, 1.0f)));
	PageTexts.Add(AddText(TEXT("PageLine3"), TEXT(""), FVector(-22.0f, -5.0f, 47.0f), 2.8f, FLinearColor(0.9f, 1.0f, 1.0f)));
	PageTexts.Add(AddText(TEXT("PageLine4"), TEXT(""), FVector(-22.0f, -5.0f, 41.0f), 2.8f, FLinearColor(0.9f, 1.0f, 1.0f)));

	AddText(TEXT("LogoText"), TEXT("ID"), FVector(-35.0f, -11.0f, 83.0f), 5.0f, FLinearColor(0.0f, 1.0f, 0.85f));
}

void ACampusARCardActor::BuildMenu()
{
	MenuHitBoxes.Add(AddMenuButton(TEXT("Profile"), FVector(-36.0f, -6.0f, 30.0f), 0));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagProfile);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Skills"), FVector(-20.0f, -6.0f, 30.0f), 1));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagSkills);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Projects"), FVector(-4.0f, -6.0f, 30.0f), 2));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagProjects);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Contact"), FVector(14.0f, -6.0f, 30.0f), 3));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagContact);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Audio"), FVector(31.0f, -6.0f, 30.0f), 4));
	MenuHitBoxes.Last()->ComponentTags.Add(ButtonTagAudio);
	MenuHitBoxes.Add(AddMenuButton(TEXT("Reset"), FVector(31.0f, -6.0f, 22.0f), 5));
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
	PhotoWidgetComponent->SetRelativeLocation(FVector(24.0f, -4.0f, 57.0f));
	PhotoWidgetComponent->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	PhotoWidgetComponent->SetRelativeScale3D(FVector(0.105f));
	PhotoWidgetComponent->InitWidget();
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
	CreateBoxMesh(ButtonMesh, FVector(7.6f, 0.6f, 2.2f), FLinearColor(0.0f, 0.42f, 0.55f, 1.0f), ButtonIndex);

	UTextRenderComponent* MenuText = AddText(FString::Printf(TEXT("Menu_%s"), *Label), Label, Location + FVector(-5.4f, 0.0f, 0.0f), 2.4f, FLinearColor(0.0f, 1.0f, 0.95f));
	MenuTexts.Add(MenuText);

	UBoxComponent* HitBox = NewObject<UBoxComponent>(this, *FString::Printf(TEXT("Hit_%s"), *Label));
	HitBox->SetupAttachment(SceneRoot);
	HitBox->RegisterComponent();
	HitBox->SetRelativeLocation(Location + FVector(2.0f, -1.0f, 0.0f));
	HitBox->SetBoxExtent(FVector(8.0f, 4.0f, 3.0f));
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

void ACampusARCardActor::RefreshPageText()
{
	if (PageTexts.Num() < 5)
	{
		return;
	}

	TArray<FString> Lines;
	switch (CurrentPage)
	{
	case ECampusCardPage::Profile:
		Lines = { TEXT("PROFILE"), TEXT("Name: Gavin Cheung"), TEXT("Major: Computer Science"), TEXT("Role: AR project student"), TEXT("Campus ID: 2026 AR demo") };
		break;
	case ECampusCardPage::Skills:
		Lines = { TEXT("SKILLS"), TEXT("C++ / Objective-C"), TEXT("Unreal Engine AR"), TEXT("iOS real-device testing"), TEXT("Interactive UI prototyping") };
		break;
	case ECampusCardPage::Projects:
		Lines = { TEXT("PROJECTS"), TEXT("AR Campus ID Card"), TEXT("3D hologram profile"), TEXT("Touch manipulation"), TEXT("Audio and pop-up menu") };
		break;
	case ECampusCardPage::Contact:
		Lines = { TEXT("CONTACT"), TEXT("Email: gavin@example.com"), TEXT("School: CityU CS5188"), TEXT("Use: club / event networking"), TEXT("Tap Audio for intro tone") };
		break;
	default:
		break;
	}

	for (int32 Index = 0; Index < PageTexts.Num() && Index < Lines.Num(); ++Index)
	{
		PageTexts[Index]->SetText(FText::FromString(Lines[Index]));
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
