#include "FrontlineHUD.h"
#include "FrontlineGame.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "Blueprint/WidgetLayoutLibrary.h"
static UButton* AddButton(UWidgetTree* Tree,UVerticalBox* Box,const FString& Label){auto* B=Tree->ConstructWidget<UButton>();auto* T=Tree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Label));T->SetColorAndOpacity(FSlateColor(FLinearColor(.92,.94,.88)));T->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",14));B->SetBackgroundColor(FLinearColor(.10,.13,.10,.95));B->AddChild(T);B->SetToolTipText(FText::FromString(Label));Box->AddChildToVerticalBox(B);return B;}
void UFrontlineHUD::NativeConstruct(){Super::NativeConstruct();auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
 Lobby=WidgetTree->ConstructWidget<UVerticalBox>();auto* L=Canvas->AddChildToCanvas(Lobby);L->SetPosition(FVector2D(40,110));L->SetSize(FVector2D(420,400));
 auto* Title=WidgetTree->ConstructWidget<UTextBlock>();Title->SetText(FText::FromString(TEXT("FRONTLINE COMMAND\nPRIVATE PLAYTEST")));Title->SetFont(FCoreStyle::GetDefaultFontStyle("Bold",24));Lobby->AddChildToVerticalBox(Title);
 Address=WidgetTree->ConstructWidget<UEditableTextBox>();Address->SetHintText(FText::FromString(TEXT("Server address:7777")));Address->SetText(FText::FromString(TEXT("127.0.0.1:7777")));Lobby->AddChildToVerticalBox(Address);
 AddButton(WidgetTree,Lobby,TEXT("CONNECT / RECONNECT"))->OnClicked.AddDynamic(this,&UFrontlineHUD::Connect);
 ReadyButton=AddButton(WidgetTree,Lobby,TEXT("READY — start when both players are ready"));ReadyButton->OnClicked.AddDynamic(this,&UFrontlineHUD::Ready);
 RematchButton=AddButton(WidgetTree,Lobby,TEXT("RETURN TO READY ROOM"));RematchButton->OnClicked.AddDynamic(this,&UFrontlineHUD::Rematch);
 AddButton(WidgetTree,Lobby,TEXT("GRAPHICS: PERFORMANCE"))->OnClicked.AddDynamic(this,&UFrontlineHUD::Low);
 AddButton(WidgetTree,Lobby,TEXT("GRAPHICS: HIGH"))->OnClicked.AddDynamic(this,&UFrontlineHUD::High);
 Status=WidgetTree->ConstructWidget<UTextBlock>();Status->SetAutoWrapText(true);Lobby->AddChildToVerticalBox(Status);
 Roster=WidgetTree->ConstructWidget<UVerticalBox>();auto* R=Canvas->AddChildToCanvas(Roster);R->SetAnchors(FAnchors(1,0));R->SetAlignment(FVector2D(1,0));R->SetPosition(FVector2D(-24,120));R->SetSize(FVector2D(235,340));
 AddButton(WidgetTree,Roster,TEXT("RIFLE SQUAD  ·  100"))->OnClicked.AddDynamic(this,&UFrontlineHUD::Rifle);
 AddButton(WidgetTree,Roster,TEXT("RECON VEHICLE  ·  150"))->OnClicked.AddDynamic(this,&UFrontlineHUD::Recon);
 AddButton(WidgetTree,Roster,TEXT("BATTLE TANK  ·  350"))->OnClicked.AddDynamic(this,&UFrontlineHUD::Tank);
 AddButton(WidgetTree,Roster,TEXT("ANTI-TANK SQUAD  ·  150"))->OnClicked.AddDynamic(this,&UFrontlineHUD::AntiTank);
 AddButton(WidgetTree,Roster,TEXT("MORTAR TEAM  ·  200"))->OnClicked.AddDynamic(this,&UFrontlineHUD::Mortar);
 AddButton(WidgetTree,Roster,TEXT("SUPPLY TRUCK  ·  125"))->OnClicked.AddDynamic(this,&UFrontlineHUD::Supply);
}
void UFrontlineHUD::NativeTick(const FGeometry& Geometry,float Delta){Super::NativeTick(Geometry,Delta);auto* PC=Cast<AFrontlineController>(GetOwningPlayer());if(!PC)return;bool Active=PC->Frame.Phase==1;Lobby->SetVisibility(Active?ESlateVisibility::Collapsed:ESlateVisibility::Visible);Roster->SetVisibility(Active?ESlateVisibility::Visible:ESlateVisibility::Collapsed);ReadyButton->SetVisibility(PC->Frame.Phase==0?ESlateVisibility::Visible:ESlateVisibility::Collapsed);RematchButton->SetVisibility(PC->Frame.Phase==2?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
 FString Message=FString::Printf(TEXT("Connected: %d / 2     Ready: %d / 2\n%s"),PC->Frame.PlayersConnected,PC->Frame.PlayersReady,*PC->Feedback);if(PC->Frame.Phase==2)Message=(PC->Frame.Winner<0?TEXT("DRAW\n"):PC->Frame.Winner==PC->Frame.Team?TEXT("VICTORY\n"):TEXT("DEFEAT\n"))+Message;Status->SetText(FText::FromString(Message));}
int32 UFrontlineHUD::NativePaint(const FPaintArgs& Args,const FGeometry& G,const FSlateRect& Clip,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool Enabled)const{
 int32 Base=Super::NativePaint(Args,G,Clip,Out,Layer,Style,Enabled);auto* PC=Cast<AFrontlineController>(GetOwningPlayer());if(!PC)return Base;
 auto Box=[&](FVector2D P,FVector2D S,FLinearColor C){FSlateDrawElement::MakeBox(Out,Base+1,G.ToPaintGeometry(S,FSlateLayoutTransform(P)),FCoreStyle::Get().GetBrush("WhiteBrush"),ESlateDrawEffect::None,C);};
 auto Text=[&](FVector2D P,const FString& S,int Size,FLinearColor C){FSlateDrawElement::MakeText(Out,Base+2,G.ToPaintGeometry(FVector2D(1000,40),FSlateLayoutTransform(P)),S,FCoreStyle::GetDefaultFontStyle("Regular",Size),ESlateDrawEffect::None,C);};
 FVector2D Size=G.GetLocalSize();FLinearColor White(.91,.93,.88),Muted(.55,.61,.55),Panel(.035,.045,.04,.94),Friendly(.36,.73,.83),Enemy(.95,.55,.27);
 Box({0,0},{Size.X,75},Panel);Text({28,15},TEXT("FRONTLINE / COMMAND"),19,White);Text({Size.X*.37,16},FString::Printf(TEXT("A  %04d    |    %04d  B"),PC->Frame.TicketsA,PC->Frame.TicketsB),23,White);int Remain=FMath::Max(0,1500-int(PC->Frame.Elapsed));Text({Size.X-160,20},FString::Printf(TEXT("%02d:%02d"),Remain/60,Remain%60),20,White);
 if(PC->Frame.Phase!=1)return Base+2;
 const FVector2D Objectives[]={{-220,-160},{0,80},{230,-110}};
 for(int I=0;I<PC->Frame.Owners.Num();++I){int Owner=PC->Frame.Owners[I];Text({Size.X*.37+I*100,49},FString::Printf(TEXT("[%c] %s"),'A'+I,Owner<0?TEXT("—"):Owner==PC->Frame.Team?TEXT("ALLY"):TEXT("ENEMY")),12,Owner<0?Muted:Owner==PC->Frame.Team?Friendly:Enemy);}
 Text({Size.X-260,88},FString::Printf(TEXT("REINFORCEMENTS   %d"),PC->Frame.Credits),15,White);
 FVector2D Map(20,Size.Y-230);Box(Map,{210,210},Panel);
 auto MapPoint=[&](FVector2D P){return Map+FVector2D((P.Y+500)*.21,(500-P.X)*.21);};
 for(int I=0;I<3;++I){auto P=MapPoint(Objectives[I]);Text(P,FString::Printf(TEXT("%c"),'A'+I),14,White);}
 for(auto& U:PC->Frame.Units){auto P=MapPoint(U.Position);Box(P,{U.Team==PC->Frame.Team?5.:7.,5},U.Team==PC->Frame.Team?Friendly:Enemy);}
 for(auto& C:PC->Frame.Contacts)Text(MapPoint(C.Position),TEXT("?"),10,Muted);
 Box({250,Size.Y-155},{FMath::Max(200.,Size.X-530),135},Panel);Text({270,Size.Y-143},FString::Printf(TEXT("SELECTED  %d"),PC->Selected.Num()),16,White);
 if(PC->Selected.Num()){for(auto& U:PC->Frame.Units)if(PC->Selected.Contains(U.Id)){Text({270,Size.Y-111},FString::Printf(TEXT("%s  |  HP %.0f  |  AMMO %d  |  SUPPRESSION %.0f%%"),UTF8_TO_TCHAR(frontline::roster[U.Kind].name),U.Health,U.Ammo,U.Suppression*100),13,White);break;}}
 Text({270,Size.Y-77},TEXT("RMB order · Shift queue · Q attack-move · E smoke · H hold · X stop"),12,Muted);Text({270,Size.Y-48},PC->Feedback,12,White);
 if(PC->Frame.OpponentGrace>0)Text({Size.X*.36,90},FString::Printf(TEXT("Opponent reconnecting — %.0fs"),PC->Frame.OpponentGrace),16,Enemy);
 for(auto& U:PC->Frame.Units){FVector2D Screen;if(!PC->ProjectWorldLocationToScreen(FVector(U.Position.X*100,U.Position.Y*100,350),Screen))continue;Screen/=UWidgetLayoutLibrary::GetViewportScale(PC);FLinearColor C=U.Team==PC->Frame.Team?Friendly:Enemy;Text(Screen,U.Team==PC->Frame.Team?TEXT("▲"):TEXT("◆"),PC->Selected.Contains(U.Id)?22:14,C);if(PC->Selected.Contains(U.Id)){Box(Screen+FVector2D(-8,22),{42,4},Panel);Box(Screen+FVector2D(-8,22),{42*FMath::Clamp(U.Health/frontline::roster[U.Kind].hp,0.f,1.f),4},C);}}
 if(PC->Dragging){float MX=0,MY=0;PC->GetMousePosition(MX,MY);float Scale=UWidgetLayoutLibrary::GetViewportScale(PC);auto A=PC->DragStart/Scale,B=FVector2D(MX,MY)/Scale;Box({FMath::Min(A.X,B.X),FMath::Min(A.Y,B.Y)},{FMath::Abs(A.X-B.X),FMath::Abs(A.Y-B.Y)},FLinearColor(.4,.7,.5,.18));}
 return Base+2;
}
void UFrontlineHUD::Connect(){CastChecked<AFrontlineController>(GetOwningPlayer())->Connect(Address->GetText().ToString());}
void UFrontlineHUD::Ready(){CastChecked<AFrontlineController>(GetOwningPlayer())->Ready();}
void UFrontlineHUD::Rematch(){CastChecked<AFrontlineController>(GetOwningPlayer())->ServerRematch();}
void UFrontlineHUD::Rifle(){CastChecked<AFrontlineController>(GetOwningPlayer())->Purchase(0);}
void UFrontlineHUD::Recon(){CastChecked<AFrontlineController>(GetOwningPlayer())->Purchase(1);}
void UFrontlineHUD::Tank(){CastChecked<AFrontlineController>(GetOwningPlayer())->Purchase(2);}
void UFrontlineHUD::AntiTank(){CastChecked<AFrontlineController>(GetOwningPlayer())->Purchase(3);}
void UFrontlineHUD::Mortar(){CastChecked<AFrontlineController>(GetOwningPlayer())->Purchase(4);}
void UFrontlineHUD::Supply(){CastChecked<AFrontlineController>(GetOwningPlayer())->Purchase(5);}
void UFrontlineHUD::Low(){auto* S=GEngine->GetGameUserSettings();S->SetOverallScalabilityLevel(1);S->ApplySettings(false);}
void UFrontlineHUD::High(){auto* S=GEngine->GetGameUserSettings();S->SetOverallScalabilityLevel(3);S->ApplySettings(false);}
