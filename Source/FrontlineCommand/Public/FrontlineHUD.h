#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FrontlineHUD.generated.h"
class UEditableTextBox; class UVerticalBox; class UTextBlock; class UButton;
UCLASS()
class FRONTLINECOMMAND_API UFrontlineHUD : public UUserWidget {
 GENERATED_BODY()
 public:
 virtual void NativeConstruct() override;
 virtual void NativeTick(const FGeometry&,float) override;
 virtual int32 NativePaint(const FPaintArgs&,const FGeometry&,const FSlateRect&,FSlateWindowElementList&,int32,const FWidgetStyle&,bool)const override;
 UPROPERTY() TObjectPtr<UEditableTextBox> Address;
 UPROPERTY() TObjectPtr<UVerticalBox> Lobby;
 UPROPERTY() TObjectPtr<UVerticalBox> Roster;
 UPROPERTY() TObjectPtr<UTextBlock> Status;
 UPROPERTY() TObjectPtr<UButton> RematchButton;
 UPROPERTY() TObjectPtr<UButton> ReadyButton;
 UFUNCTION() void Connect(); UFUNCTION() void Ready(); UFUNCTION() void Rematch();
 UFUNCTION() void Rifle(); UFUNCTION() void Recon(); UFUNCTION() void Tank(); UFUNCTION() void AntiTank(); UFUNCTION() void Mortar(); UFUNCTION() void Supply();
 UFUNCTION() void Low(); UFUNCTION() void High();
};
