#include "CampusVideoWidget.h"

#include "Components/Image.h"
#include "FileMediaSource.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"

UMediaPlayer* UCampusVideoWidget::GetMediaPlayer()
{
	EnsureMediaObjects();
	return MediaPlayer;
}

bool UCampusVideoWidget::OpenMedia()
{
	EnsureMediaObjects();
	if (!bMediaOpened)
	{
		bMediaOpened = MediaPlayer->OpenSource(MediaSource);
#if PLATFORM_IOS
		MediaPlayer->SetNativeVolume(1.0f);
#endif
	}
	return bMediaOpened;
}

void UCampusVideoWidget::CloseMedia()
{
	if (MediaPlayer)
	{
		MediaPlayer->Close();
	}
	bMediaOpened = false;
}

void UCampusVideoWidget::EnsureMediaObjects()
{
	if (!MediaPlayer)
	{
		MediaPlayer = NewObject<UMediaPlayer>(this, TEXT("CampusLoopMediaPlayer"));
		MediaPlayer->PlayOnOpen = true;
		MediaPlayer->SetLooping(true);
#if PLATFORM_IOS
		MediaPlayer->NativeAudioOut = true;
#endif
	}

	if (!MediaTexture)
	{
		MediaTexture = NewObject<UMediaTexture>(this, TEXT("CampusLoopMediaTexture"));
		MediaTexture->SetMediaPlayer(MediaPlayer);
		MediaTexture->AutoClear = true;
		MediaTexture->ClearColor = FLinearColor::Black;
		MediaTexture->UpdateResource();
	}

	if (!MediaSource)
	{
		MediaSource = NewObject<UFileMediaSource>(this, TEXT("CampusLoopMediaSource"));
		MediaSource->SetFilePath(TEXT("./Movies/campus-preview.mp4"));
	}

}

TSharedRef<SWidget> UCampusVideoWidget::RebuildWidget()
{
	EnsureMediaObjects();

	VideoImage = NewObject<UImage>(this, TEXT("CampusLoopVideoImage"));
	FSlateBrush VideoBrush;
	VideoBrush.SetResourceObject(MediaTexture);
	VideoBrush.ImageSize = FVector2D(1600.0f, 900.0f);
	VideoImage->SetBrush(VideoBrush);

	return VideoImage->TakeWidget();
}
