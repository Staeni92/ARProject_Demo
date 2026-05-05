#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CampusEncyclopediaWidget.generated.h"

UCLASS()
class ARPROJECT_API UCampusEncyclopediaWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
