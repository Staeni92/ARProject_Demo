#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CampusVideoWidget.generated.h"

class UFileMediaSource;
class UImage;
class UMediaPlayer;
class UMediaTexture;

UCLASS()
class ARPROJECT_API UCampusVideoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMediaPlayer* GetMediaPlayer();
	bool OpenMedia();
	void CloseMedia();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void EnsureMediaObjects();

	UPROPERTY()
	UImage* VideoImage;

	UPROPERTY()
	UMediaPlayer* MediaPlayer;

	UPROPERTY()
	UMediaTexture* MediaTexture;

	UPROPERTY()
	UFileMediaSource* MediaSource;

	bool bMediaOpened = false;
};
