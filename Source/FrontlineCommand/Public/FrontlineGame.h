#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "FrontlineTypes.h"
#include "Core/Battle.h"
#include "FrontlineGame.generated.h"
class UCameraComponent;
class UFrontlineHUD;
UCLASS()
class FRONTLINECOMMAND_API UFrontlineGameInstance : public UGameInstance {
 GENERATED_BODY()
 public:
 FString ReconnectToken, LastAddress, NetworkMessage;
 virtual void Init() override;
 virtual void Shutdown() override;
 void NetworkFailure(UWorld*,UNetDriver*,ENetworkFailure::Type,const FString&);
 FDelegateHandle FailureHandle;
};
UCLASS()
class FRONTLINECOMMAND_API AFrontlineCamera : public APawn {
 GENERATED_BODY()
 public:
 AFrontlineCamera();
 UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
};
UCLASS()
class FRONTLINECOMMAND_API AFrontlineController : public APlayerController {
 GENERATED_BODY()
 public:
 AFrontlineController();
 virtual void BeginPlay() override;
 virtual void PlayerTick(float Delta) override;
 virtual void SetupInputComponent() override;
 UPROPERTY(BlueprintReadOnly) FFrontlineFrame Frame;
 UPROPERTY(BlueprintReadOnly) FString Feedback;
 UPROPERTY() TObjectPtr<UFrontlineHUD> Interface;
 UPROPERTY() TMap<int32,TObjectPtr<AActor>> Visuals;
 TSet<int32> Selected;
 TMap<int32,TArray<int32>> Groups;
 FVector2D DragStart=FVector2D::ZeroVector;
 bool Dragging=false;
 int32 PendingAction=0;
 int32 ServerTeam=-1;
 double LastCommand=0,LastPurchase=0;
 FString SessionToken;
 UFUNCTION(Server,Reliable) void ServerReady();
 UFUNCTION(Server,Reliable) void ServerPurchase(int32 Kind);
 UFUNCTION(Server,Reliable) void ServerOrder(const TArray<int32>& Ids,int32 Action,FVector2D Position,int32 Target,bool Queue);
 UFUNCTION(Server,Reliable) void ServerRematch();
 UFUNCTION(Client,Reliable) void ClientSession(const FString& Token,int32 Team);
 UFUNCTION(Client,Unreliable) void ClientFrame(const FFrontlineFrame& NewFrame);
 UFUNCTION(Client,Reliable) void ClientFeedback(const FString& Message);
 UFUNCTION(BlueprintCallable) void Connect(const FString& Address);
 UFUNCTION(BlueprintCallable) void Ready();
 UFUNCTION(BlueprintCallable) void Purchase(int32 Kind);
 void LeftDown(); void LeftUp(); void RightDown(); void ZoomIn(); void ZoomOut();
 bool Ground(FVector2D& Out)const;
 void Order(int32 Action,FVector2D Position,int32 Target=-1);
 void RefreshVisuals();
};
UCLASS()
class FRONTLINECOMMAND_API AFrontlineGameMode : public AGameModeBase {
 GENERATED_BODY()
 public:
 AFrontlineGameMode();
 frontline::Battle Battle;
 virtual void BeginPlay() override;
 virtual void Tick(float Delta) override;
 virtual void PreLogin(const FString& Options,const FString& Address,const FUniqueNetIdRepl& UniqueId,FString& ErrorMessage) override;
 virtual FString InitNewPlayer(APlayerController*,const FUniqueNetIdRepl&,const FString&,const FString&) override;
 virtual void PostLogin(APlayerController* NewPlayer) override;
 virtual void Logout(AController* Exiting) override;
 float Accumulator=0,SnapshotClock=0;
 void SendFrames();
};
// Builds identical static scenery locally; no hidden combat actors are replicated.
void BuildFrontlineTerrain(UWorld* World);
