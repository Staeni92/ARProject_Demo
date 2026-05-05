#include "CampusPhotoWidget.h"

#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Widgets/SBoxPanel.h"

TSharedRef<SWidget> UCampusPhotoWidget::RebuildWidget()
{
	PhotoImage = NewObject<UImage>(this, TEXT("CampusProfilePhotoImage"));
	if (PhotoTexture)
	{
		PhotoImage->SetBrushFromTexture(PhotoTexture, true);
	}
	return PhotoImage->TakeWidget();
}

void UCampusPhotoWidget::SetPhotoTexture(UTexture2D* InTexture)
{
	PhotoTexture = InTexture;
	if (PhotoImage && PhotoTexture)
	{
		PhotoImage->SetBrushFromTexture(PhotoTexture, true);
	}
}
