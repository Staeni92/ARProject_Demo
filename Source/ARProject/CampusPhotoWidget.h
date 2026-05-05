#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CampusPhotoWidget.generated.h"

class UImage;
class UTexture2D;

UCLASS()
class ARPROJECT_API UCampusPhotoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPhotoTexture(UTexture2D* InTexture);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	UImage* PhotoImage;

	UPROPERTY()
	UTexture2D* PhotoTexture;
};
