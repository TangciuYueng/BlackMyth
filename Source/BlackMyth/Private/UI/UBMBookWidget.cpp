// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UBMBookWidget.h"
#include "Components/TextBlock.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Engine/Font.h"

void UBMBookWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Split the narrative into pages of roughly six short lines each (Chinese localized content)
    Pages.Add(FText::FromString(TEXT(
        "五百年前，灵山雷音殿，莲台宝光渐次熄灭。\n"
        "我叩首，受封“斗战胜佛”。金箍从头顶自然脱落，发出金石断裂之音。\n"
        "诸佛颂唱，梵音如海。师父眼中最后一点属于金蝉子的灵光，在那片金色汪洋里彻底沉没。\n"
        "那是我一生功成的顶点，也是真相溺毙的时刻。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "他们都说，我护送玄奘法师取得真经，普渡众生。"
        "却无人知晓，灵山赐下的经卷其实无字。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "阿傩、伽叶递过经书时，唇角有慈悲的笑意。包袱展开的刹那，\n"
        "我火眼金睛看见经文如退潮般消失，只留满纸空白。\n"
        "师父的手在颤抖——他早就知道。或者说，金蝉子的那一部分知道。\n"
        "西行十万八千里，九死一生，取回的是一场空。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "八戒忽然笑出声来，笑得涕泪横流，被护法金刚冷眼按下。\n"
        "沙僧把头埋进经卷，肩胛骨耸动如将崩的山峦。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "如来声如洪钟：“法不可轻传，亦不可空取。无字是真经\n"
        "需以因果为墨，岁月为笔，自行书写。”\n"
        "那一刻我明白了，我们取的不是经，是“取经”这件事本身。\n"
        "我们的苦难、我们的战斗、我们的名号，就是他们要的“真经”。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "一场演给三界看的大戏，证明佛法无边，证明叛逆者可被驯服。\n"
        "紧箍咒念的不是约束，是遗忘。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "观音说此咒制我凶心。可每当金箍收紧，我打杀的“妖魔”临死前的眼神就会模糊。\n"
        "他们的脸孔在记忆里融化，变成故事书里千篇一律的狰狞模样。\n"
        "直到成佛那日，封印松动，无数真实碎片倒灌回脑海：\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "我打死的第一个“妖精”——那位白衣姑娘，她根本不是要吃师父。\n"
        "她跪在悟空面前，摊开手掌，掌心是用血画出的星图与古神文。\n"
        "她嘶喊着：“大圣！看看这天地真正的脉络！他们用香火愿力编织罗网，我们都在网中！”\n"
        "金箍骤然收紧。\n"
        "我抬手，金箍棒落下时，她化作一缕青烟，最后的声音是：‘你也会被……忘记……’\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "是的，我忘记了。直到此刻才想起。"
        "那些所谓的劫难，多半是灭口。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "火焰山炼的不是我的神通，是要烧毁埋在八百里地下、记载着“上一次天地劫难”的玉简。\n"
        "牛魔王拼死守护的不是罗刹女，是祖先传承的、关于“世界如何被重置”的记忆。\n"
        "黄风怪的三昧神风，本是为了吹散灵山脚下堆积的、写满罪业的愿力尘埃……\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "我们每过一难，就将一部分真实世界“涂抹”成该有的样子。\n"
        "而我自己，就是那支蘸满了谎言的笔。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "如今，佛光开始腐朽。\n"
        "三百年过去了。我从斗战胜佛的金身里醒来，发现灵山赐予的果位是一个精致的囚笼。\n"
        "佛国寂静无声，诸佛垂目如泥塑，周身散发出的不是檀香，而是铁锈与枯萎莲花的气味。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "人间异动：寺庙佛像流泪，诵经声会腐蚀血肉，曾被抹去的残影重现人间。\n"
        "那些被我们亲手埋葬的真相，正在尸变。\n"
        "它们从故事的坟墓里爬出来，带着强烈的执念与怨恨，要找回自己的名字。\n"
        "我剥下这身灿烂的佛衣，露出底下五百年前未被驯服的皮毛。\n"
        "金箍棒在手中苏醒，发出饥渴的嗡鸣。它记得每一滴真正该流的血。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "我要重新走一遍西行路。这次不是取经，是掘坟。\n"
        "挖开每一处我们曾“降妖除魔”的圣地，把里面埋藏的秘密、谎言与罪恶，统统挖出来。\n"
        "让该成佛的成佛。让该下地狱的下地狱。\n"
        "而我，齐天大圣孙悟空，要在这成佛与地狱之间——\n"
        "为那些被我们亲手写进鬼故事里的亡魂，讨一个公道。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "第一关的山地，是我棒杀第一位“真相诉说者”的故地。风中的呜咽，是她未散的执念。\n"
        "第二关的巨石阵，是上古遗民观测天庭轨迹的祭坛，也是灵山镇压“天外之音”的封印。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "这一次，我的金箍棒不为护法，只为打碎这尊他们为我塑造的、名为‘斗战胜佛’的琉璃金身。\n"
        "\n"
        "(按 Enter 翻页)"
    )));
    Pages.Add(FText::FromString(TEXT(
        "键位设置\n"
        "W 向前移动                                                                    A 向左移动\n"
        "S 向后移动                                                                     D 向右移动\n"
        "Crtl 向后闪避                                                                 Shift 奔跑\n"
        "鼠标左键 普通攻击                                                       鼠标右键 技能1\n"
        "Q 技能2                                                                          E 技能3\n"
        "Tab 暂停菜单                                                                I 背包\n"
        "Enter 跳过剧情                 \n"
        "1-9（不在背包界面） 使用法器                                 1-9（在背包界面） 购买法器\n"
        "M 金币作弊                                                                    L 等级作弊\n"
        "\n"
        "(按 Enter 退出)"
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
