// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UBMBookWidget.h"
#include "Components/TextBlock.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Engine/Font.h"

void UBMBookWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Split the narrative into pages of roughly six short lines each
    Pages.Add(FText::FromString(TEXT(
        "Five hundred years ago, in the Thunder Sound Hall of Lingshan,\n"
        "the precious light of Liantai gradually extinguished.\n\n"
        "I kowtowed and was honored with the title 'Fighting to Overcome Buddha'.\n"
        "The golden hoop fell from my head with a crack of metal and stone.\n\n"
        "The Buddhas chanted; Sanskrit rolled like the sea."
    )));

    Pages.Add(FText::FromString(TEXT(
        "The last light in my master's eyes was that of the Golden Cicada,\n"
        "which sank into the golden ocean.\n\n"
        "That was the pinnacle of my life, and the moment the truth drowned.\n"
        "They said we retrieved the true scriptures to save all beings.\n\n"
        "But the scriptures from Lingshan were wordless."
    )));

    Pages.Add(FText::FromString(TEXT(
        "When Anu and Jiaye handed over the bundle, they smiled with compassion.\n"
        "When it opened, my eyes saw the text vanish like a receding tide,\n\n"
        "leaving only blank paper. My master's hands trembled; he had known.\n"
        "We had traveled 108,000 miles to retrieve nothing.\n\n"
        "It was the act of taking that mattered."
    )));

    Pages.Add(FText::FromString(TEXT(
        "Bajie burst into laughter, tears streaming; King Kong forced him down.\n"
        "Sha Monk buried his head in the scroll, shoulders heaving like mountains.\n\n"
        "The Tathagata's voice rang out like a bell:\n"
        "'The Dharma cannot be transmitted lightly, nor taken empty.'\n\n"
        "'Without words, it must be written with cause and effect and time.'"
    )));

    Pages.Add(FText::FromString(TEXT(
        "I understood then: the pilgrimage itself was the scripture.\n"
        "Our suffering, our battles, and our names were their proof.\n\n"
        "A play for the Three Realms to assert authority and obedience.\n"
        "The tightening mantra did not bind the body so much as erase memory.\n\n"
        "Guanyin said it curbed my fierce heart."
    )));

    Pages.Add(FText::FromString(TEXT(
        "Whenever the hoop tightened, faces of those I struck blurred in memory.\n"
        "Their true shapes melted into grotesque figures in storybooks.\n\n"
        "On the day the seal loosened, shards of reality returned to me.\n"
        "Memories I had been forced to forget flooded back.\n\n"
        "I remembered the first 'fairy' I killed."
    )));

    Pages.Add(FText::FromString(TEXT(
        "She wore white and did not seek to devour our master. She knelt and opened her palm,\n"
        "revealing a star chart and ancient script written in blood.\n\n"
        "'Great Saint! See the veins of this world! They weave a web with incense and vows!'\n"
        "The golden hoop tightened; my cudgel fell and she became smoke.\n\n"
        "Her last words: 'You too will be forgotten.'"
    )));

    Pages.Add(FText::FromString(TEXT(
        "Yes¡ªI had forgotten until now. Many 'calamities' were silencing the truth.\n"
        "Flame Mountain burned jade slips buried deep that recorded the last calamity.\n\n"
        "The Bull Demon King guarded ancestral memories, not a single rakshasi.\n"
        "The Yellow Wind's Samadhi Wind cleared the vow-dust that hid karmic ledgers.\n\n"
        "Each trial painted over a piece of reality."
    )));

    Pages.Add(FText::FromString(TEXT(
        "I was the pen dipped in lies. Now the Buddha's light decays.\n\n"
        "Three hundred years later I woke from the golden body to find a cage.\n"
        "The Buddha realm was silent; statues hung like clay and smelled of rust.\n\n"
        "Temples wept blood; chanting could rot flesh in places."
    )));

    Pages.Add(FText::FromString(TEXT(
        "Remnants of the 'demons' we erased were re-forming in the world.\n\n"
        "Truths we buried were mutating, crawling from story-graves with rage.\n"
        "They demanded names returned and histories untampered.\n\n"
        "I tore off the Buddha robe and revealed untamed fur beneath."
    )));

    Pages.Add(FText::FromString(TEXT(
        "My golden cudgel awoke with a hungry hum; it remembered rightful blood.\n\n"
        "I would retrace the westward route¡ªnot to save, but to exhume.\n"
        "Open every holy site where we 'conquered demons' and unearth the lies within.\n\n"
        "Let those who should be Buddhas become Buddhas; let the guilty burn."
    )));

    Pages.Add(FText::FromString(TEXT(
        "I, Sun Wukong, will seek justice for the souls we wrote into ghost stories.\n\n"
        "The first level's mountain is where I silenced the first truth-teller.\n"
        "The wind's sobbing is her lingering obsession.\n\n"
        "The Stonehenge of the second level is a seal against voices from beyond."
    )));

    Pages.Add(FText::FromString(TEXT(
        "This time my golden cudgel is not to protect but to shatter the glass-gilded idol\n"
        "they made called 'Fighting to Overcome Buddha.'\n\n"
        "(Press Enter to advance)"
    )));
}

void UBMBookWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentPageIndex = 0;
	UpdatePageContent();

	// Set focus to this widget so it can receive key events
	bIsFocusable = true;
    SetKeyboardFocus();

    // Diagnostics: log if TextBook isn't bound
    if (!TextBook)
    {
        UE_LOG(LogTemp, Warning, TEXT("UBMBookWidget: TextBook is not bound. Make sure WBP_Book has a TextBlock named 'TextBook' and its parent class is UBMBookWidget."));
    }

    // If a DefaultChineseFont is assigned in the editor, apply it to the TextBook to ensure Chinese renders
    if (TextBook && DefaultChineseFont.IsValid())
    {
        if (UFont* LoadedFont = DefaultChineseFont.Get())
        {
            FSlateFontInfo FI(LoadedFont, 28);
            TextBook->SetFont(FI);
            UE_LOG(LogTemp, Log, TEXT("UBMBookWidget: Applied DefaultChineseFont '%s' to TextBook."), *LoadedFont->GetName());
        }
        else
        {
            UObject* Obj = DefaultChineseFont.LoadSynchronous();
            if (UFont* SyncFont = Cast<UFont>(Obj))
            {
                FSlateFontInfo FI(SyncFont, 28);
                TextBook->SetFont(FI);
                UE_LOG(LogTemp, Log, TEXT("UBMBookWidget: Loaded and applied DefaultChineseFont '%s' to TextBook."), *SyncFont->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("UBMBookWidget: DefaultChineseFont failed to load or is not a UFont."));
            }
        }
    }
}

FReply UBMBookWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		ShowNextPage();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UBMBookWidget::ShowNextPage()
{
	if (CurrentPageIndex < Pages.Num() - 1)
	{
		CurrentPageIndex++;
		UpdatePageContent();
	}
	else
	{
		CloseBook();
	}
}

void UBMBookWidget::CloseBook()
{
    // Restore game input and hide cursor when book is closed
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameOnly GameOnly;
        PC->SetInputMode(GameOnly);
        PC->bShowMouseCursor = false;
    }

    RemoveFromParent();
}

void UBMBookWidget::UpdatePageContent()
{
	if (TextBook && Pages.IsValidIndex(CurrentPageIndex))
	{
		TextBook->SetText(Pages[CurrentPageIndex]);
	}
}
