#include "CampusEncyclopediaWidget.h"

#include "Styling/SlateTypes.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FSlateBrush* WhiteBrush()
	{
		return FCoreStyle::Get().GetBrush("WhiteBrush");
	}

	TSharedRef<SWidget> BuildEncyclopediaCard(const FText& Title, const FText& Body)
	{
		return SNew(SBox)
			.WidthOverride(700.0f)
			.HeightOverride(820.0f)
			[
				SNew(SBorder)
				.BorderImage(WhiteBrush())
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(FMargin(34.0f, 28.0f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 18.0f)
					[
						SNew(STextBlock)
						.Text(Title)
						.ColorAndOpacity(FSlateColor(FLinearColor::Black))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SNew(STextBlock)
							.Text(Body)
							.ColorAndOpacity(FSlateColor(FLinearColor::Black))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 23))
							.AutoWrapText(true)
							.WrapTextAt(620.0f)
							.LineHeightPercentage(1.16f)
						]
					]
				]
			];
	}
}

TSharedRef<SWidget> UCampusEncyclopediaWidget::RebuildWidget()
{
	const FText OverviewTitle = FText::FromString(TEXT("1. Overview"));
	const FText OverviewBody = FText::FromString(TEXT("City University of Hong Kong (Dongguan), abbreviated as CityUHK (Dongguan), is located in Songshan Lake, Dongguan, Guangdong, People's Republic of China, near Pingshan Village in Dalang Town. It is an independent legal-person university jointly established by City University of Hong Kong and Dongguan University of Technology through a Chinese-foreign cooperative education model. The university was approved by the Ministry of Education of the People's Republic of China for preparation in June 2022, approved for formal establishment in April 2024, and officially opened in September 2024. It currently has four schools: the School of Science, the School of Engineering, the School of Medicine and Life Sciences, and the School of Management."));
	const FText HistoryTitle = FText::FromString(TEXT("2. History"));
	const FText HistoryBody = FText::FromString(TEXT("In March 2020, City University of Hong Kong planned to establish a campus in the Songshan Lake High-tech Industrial Development Zone in Dongguan under the name City University of Hong Kong (Dongguan).\n\nOn the morning of April 22, 2021, City University of Hong Kong (Dongguan) held its groundbreaking ceremony.\n\nOn June 2, 2022, the Ministry of Education of the People's Republic of China approved the preparation for establishing City University of Hong Kong (Dongguan).\n\nOn April 16, 2024, the Ministry of Education issued a letter approving the formal establishment of City University of Hong Kong (Dongguan). In September of the same year, the university officially opened. In its first year, the university admitted 120 undergraduate students and 442 master's students. In November 2024, the university was approved to admit students from Hong Kong, Macao, and Taiwan starting in 2025 through the Joint Entrance Examination for students from Hong Kong, Macao, and Taiwan, as well as Taiwan GSAT results."));

	return SNew(SBorder)
		.BorderImage(WhiteBrush())
		.BorderBackgroundColor(FLinearColor(0.88f, 0.90f, 0.92f, 1.0f))
		.Padding(FMargin(32.0f, 40.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 28.0f, 0.0f)
			[
				BuildEncyclopediaCard(OverviewTitle, OverviewBody)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				BuildEncyclopediaCard(HistoryTitle, HistoryBody)
			]
		];
}
