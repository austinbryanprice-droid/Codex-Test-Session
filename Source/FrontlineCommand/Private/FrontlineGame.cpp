#include "FrontlineGame.h"
#include "FrontlineHUD.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerInput.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UFrontlineGameInstance::Init(){Super::Init();FailureHandle=GEngine->OnNetworkFailure().AddUObject(this,&UFrontlineGameInstance::NetworkFailure);}
void UFrontlineGameInstance::Shutdown(){if(GEngine)GEngine->OnNetworkFailure().Remove(FailureHandle);Super::Shutdown();}
void UFrontlineGameInstance::NetworkFailure(UWorld*,UNetDriver*,ENetworkFailure::Type,const FString& Error){NetworkMessage=Error;UGameplayStatics::OpenLevel(this,TEXT("/Engine/Maps/Entry"));}
AFrontlineCamera::AFrontlineCamera(){PrimaryActorTick.bCanEverTick=false;Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));RootComponent=Camera;Camera->SetRelativeRotation(FRotator(-60,0,0));Camera->FieldOfView=55;SetActorLocation(FVector(-18000,0,24000));}
AFrontlineGameMode::AFrontlineGameMode(){PrimaryActorTick.bCanEverTick=true;DefaultPawnClass=AFrontlineCamera::StaticClass();PlayerControllerClass=AFrontlineController::StaticClass();}
void AFrontlineGameMode::BeginPlay(){Super::BeginPlay();BuildFrontlineTerrain(GetWorld());}
void AFrontlineGameMode::PreLogin(const FString& Options,const FString& Address,const FUniqueNetIdRepl& UniqueId,FString& Error){
 Super::PreLogin(Options,Address,UniqueId,Error);if(!Error.IsEmpty())return;
 FString Token=UGameplayStatics::ParseOption(Options,TEXT("Session"));
 if(!Token.IsEmpty()){bool Allowed=false;for(auto& P:Battle.players)if(P.token==TCHAR_TO_UTF8(*Token)&&!P.connected&&P.occupied&&Battle.phase!=frontline::Phase::Finished&&(Battle.phase==frontline::Phase::Lobby||P.grace>0))Allowed=true;if(!Allowed)Error=TEXT("Reconnect session unavailable");}
 else if(Battle.phase!=frontline::Phase::Lobby|| (Battle.players[0].occupied&&Battle.players[1].occupied))Error=TEXT("Server full or match already started");
}
FString AFrontlineGameMode::InitNewPlayer(APlayerController* Player,const FUniqueNetIdRepl& Id,const FString& Options,const FString& Portal){
 FString Error=Super::InitNewPlayer(Player,Id,Options,Portal);if(!Error.IsEmpty())return Error;
 auto* PC=Cast<AFrontlineController>(Player);if(!PC)return TEXT("Unsupported controller");
 FString Token=UGameplayStatics::ParseOption(Options,TEXT("Session"));if(Token.IsEmpty())Token=FGuid::NewGuid().ToString(EGuidFormats::Digits);
 int Team=Battle.join(TCHAR_TO_UTF8(*Token));if(Team<0)return TEXT("Server full or session unavailable");PC->ServerTeam=Team;PC->SessionToken=Token;return {};
}
void AFrontlineGameMode::PostLogin(APlayerController* NewPlayer){Super::PostLogin(NewPlayer);if(auto* PC=Cast<AFrontlineController>(NewPlayer))PC->ClientSession(PC->SessionToken,PC->ServerTeam);}
void AFrontlineGameMode::Logout(AController* Controller){if(auto* PC=Cast<AFrontlineController>(Controller))Battle.disconnect(PC->ServerTeam);Super::Logout(Controller);}
void AFrontlineGameMode::Tick(float Delta){Super::Tick(Delta);Accumulator+=FMath::Min(Delta,.25f);while(Accumulator>=.05f){Battle.tick(.05f);Accumulator-=.05f;}SnapshotClock+=Delta;if(SnapshotClock>=.1f){SnapshotClock=0;SendFrames();}}
void AFrontlineGameMode::SendFrames(){for(FConstPlayerControllerIterator It=GetWorld()->GetPlayerControllerIterator();It;++It){auto* PC=Cast<AFrontlineController>(It->Get());if(!PC||PC->ServerTeam<0)continue;
 FFrontlineFrame Frame;Frame.Team=PC->ServerTeam;Frame.Phase=int(Battle.phase);Frame.Winner=Battle.winner;Frame.TicketsA=Battle.players[0].tickets;Frame.TicketsB=Battle.players[1].tickets;Frame.Credits=int(Battle.players[Frame.Team].credits);Frame.Elapsed=Battle.elapsed;Frame.OpponentGrace=Battle.players[1-Frame.Team].grace;
 for(auto& P:Battle.players){Frame.PlayersConnected+=P.connected;Frame.PlayersReady+=P.ready;}
 for(auto& O:Battle.objectives){Frame.Owners.Add(O.owner);Frame.Capture.Add(O.capture/8);}
 for(auto& S:Battle.smoke)Frame.Smoke.Add(FVector2D(S.position.x,S.position.y));
 auto View=Battle.view(Frame.Team);for(auto& U:View.units){FFrontlineUnitView V;V.Id=U.id;V.Team=U.team;V.Kind=int(U.kind);V.Position=FVector2D(U.position.x,U.position.y);V.Heading=U.heading;V.Health=U.hp;V.Suppression=U.suppression;V.Ammo=U.ammo;Frame.Units.Add(V);}
 for(auto& C:View.contacts){FFrontlineContact V;V.Position=FVector2D(C.position.x,C.position.y);V.Age=C.age;Frame.Contacts.Add(V);}
 PC->ClientFrame(Frame);
}}
AFrontlineController::AFrontlineController(){bShowMouseCursor=true;PrimaryActorTick.bCanEverTick=true;}
void AFrontlineController::BeginPlay(){Super::BeginPlay();if(IsLocalController()){BuildFrontlineTerrain(GetWorld());Interface=CreateWidget<UFrontlineHUD>(this,UFrontlineHUD::StaticClass());Interface->AddToViewport();SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));if(auto* GI=GetGameInstance<UFrontlineGameInstance>())Feedback=GI->NetworkMessage;}}
void AFrontlineController::ClientSession_Implementation(const FString& Token,int32 Team){if(auto* GI=GetGameInstance<UFrontlineGameInstance>()){if(GetNetMode()==NM_Client)GI->ReconnectToken=Token;}Frame.Team=Team;}
void AFrontlineController::ClientFrame_Implementation(const FFrontlineFrame& NewFrame){Frame=NewFrame;RefreshVisuals();}
void AFrontlineController::ClientFeedback_Implementation(const FString& Message){Feedback=Message;}
void AFrontlineController::ServerReady_Implementation(){if(auto* GM=GetWorld()->GetAuthGameMode<AFrontlineGameMode>())GM->Battle.ready(ServerTeam);}
void AFrontlineController::ServerPurchase_Implementation(int32 Kind){double Now=GetWorld()->GetTimeSeconds();if(Now-LastPurchase<.15)return;LastPurchase=Now;if(auto* GM=GetWorld()->GetAuthGameMode<AFrontlineGameMode>())ClientFeedback(UTF8_TO_TCHAR(GM->Battle.purchase(ServerTeam,frontline::Kind(Kind)).c_str()));}
void AFrontlineController::ServerOrder_Implementation(const TArray<int32>& Ids,int32 Action,FVector2D Position,int32 Target,bool Queue){double Now=GetWorld()->GetTimeSeconds();if(Now-LastCommand<.04||Ids.Num()>20)return;LastCommand=Now;
 if(auto* GM=GetWorld()->GetAuthGameMode<AFrontlineGameMode>()){std::vector<int> Units;for(int Id:Ids)Units.push_back(Id);frontline::Order O;O.action=frontline::Action(Action);O.destination={float(Position.X),float(Position.Y)};O.target=Target;ClientFeedback(UTF8_TO_TCHAR(GM->Battle.command(ServerTeam,Units,O,Queue).c_str()));}}
void AFrontlineController::ServerRematch_Implementation(){auto* GM=GetWorld()->GetAuthGameMode<AFrontlineGameMode>();if(!GM||GM->Battle.phase!=frontline::Phase::Finished)return;auto Players=GM->Battle.players;GM->Battle=frontline::Battle{};for(int T=0;T<2;++T)if(Players[T].connected){GM->Battle.players[T]=frontline::Player{};GM->Battle.players[T].occupied=true;GM->Battle.players[T].connected=true;GM->Battle.players[T].token=Players[T].token;}GM->SendFrames();}
void AFrontlineController::Connect(const FString& Address){FString Clean=Address.TrimStartAndEnd();if(Clean.IsEmpty()||Clean.Len()>253){Feedback=TEXT("Enter a host name or IPv4 address, optionally :7777");return;}for(TCHAR C:Clean)if(!FChar::IsAlnum(C)&&C!='.'&&C!='-'&&C!=':'){Feedback=TEXT("Invalid server address");return;}
 auto* GI=GetGameInstance<UFrontlineGameInstance>();FString URL=Clean;if(GI){if(GI->LastAddress!=Clean)GI->ReconnectToken.Empty();GI->LastAddress=Clean;if(!GI->ReconnectToken.IsEmpty())URL+=TEXT("?Session=")+GI->ReconnectToken;}Feedback=TEXT("Connecting...");ClientTravel(URL,TRAVEL_Absolute);}
void AFrontlineController::Ready(){ServerReady();}
void AFrontlineController::Purchase(int32 Kind){ServerPurchase(Kind);}
void AFrontlineController::SetupInputComponent(){Super::SetupInputComponent();InputComponent->BindKey(EKeys::LeftMouseButton,IE_Pressed,this,&AFrontlineController::LeftDown);InputComponent->BindKey(EKeys::LeftMouseButton,IE_Released,this,&AFrontlineController::LeftUp);InputComponent->BindKey(EKeys::RightMouseButton,IE_Pressed,this,&AFrontlineController::RightDown);InputComponent->BindKey(EKeys::MouseScrollUp,IE_Pressed,this,&AFrontlineController::ZoomIn);InputComponent->BindKey(EKeys::MouseScrollDown,IE_Pressed,this,&AFrontlineController::ZoomOut);}
bool AFrontlineController::Ground(FVector2D& Out)const{FVector O,D;if(!DeprojectMousePositionToWorld(O,D)||FMath::IsNearlyZero(D.Z))return false;float T=-O.Z/D.Z;if(T<0)return false;FVector P=O+D*T;Out=FVector2D(P.X/100,P.Y/100);return frontline::valid({float(Out.X),float(Out.Y)});}
void AFrontlineController::LeftDown(){if(Frame.Phase!=1)return;float MX=0,MY=0;GetMousePosition(MX,MY);DragStart=FVector2D(MX,MY);
 float Scale=UWidgetLayoutLibrary::GetViewportScale(this);int32 Width=0,Height=0;GetViewportSize(Width,Height);FVector2D Mouse=DragStart/Scale;float MapY=Height/Scale-230;
 if(Mouse.X>=20&&Mouse.X<=230&&Mouse.Y>=MapY&&Mouse.Y<=MapY+210){if(auto* P=GetPawn()){auto L=P->GetActorLocation();L.X=(500-(Mouse.Y-MapY)/.21)*100-L.Z*.57735;L.Y=((Mouse.X-20)/.21-500)*100;P->SetActorLocation(L);}return;}
 Dragging=true;}
void AFrontlineController::LeftUp(){if(!Dragging)return;Dragging=false;float MX=0,MY=0;GetMousePosition(MX,MY);FVector2D End(MX,MY);if(!IsInputKeyDown(EKeys::LeftShift))Selected.Empty();
 bool Box=FVector2D::Distance(DragStart,End)>8;float Best=30;int Pick=-1;
 for(auto& U:Frame.Units)if(U.Team==Frame.Team){FVector2D Screen;ProjectWorldLocationToScreen(FVector(U.Position.X*100,U.Position.Y*100,150),Screen);if(Box){if(Screen.X>=FMath::Min(DragStart.X,End.X)&&Screen.X<=FMath::Max(DragStart.X,End.X)&&Screen.Y>=FMath::Min(DragStart.Y,End.Y)&&Screen.Y<=FMath::Max(DragStart.Y,End.Y))Selected.Add(U.Id);}else{float D=FVector2D::Distance(Screen,End);if(D<Best){Best=D;Pick=U.Id;}}}if(Pick>=0)Selected.Add(Pick);
}
void AFrontlineController::Order(int32 Action,FVector2D Position,int32 Target){TArray<int32> Ids=Selected.Array();if(Ids.Num())ServerOrder(Ids,Action,Position,Target,IsInputKeyDown(EKeys::LeftShift));}
void AFrontlineController::RightDown(){FVector2D P;if(Frame.Phase!=1||!Ground(P))return;int Target=-1,Action=PendingAction;PendingAction=0;if(Action==0)for(auto& U:Frame.Units)if(FVector2D::Distance(P,U.Position)<12){if(U.Team!=Frame.Team){Target=U.Id;Action=1;}else if(U.Kind==5){Target=U.Id;Action=5;}break;}Order(Action,P,Target);}
void AFrontlineController::ZoomIn(){if(auto* P=GetPawn()){auto L=P->GetActorLocation();L.Z=FMath::Max(6000.,L.Z-2000);P->SetActorLocation(L);}}
void AFrontlineController::ZoomOut(){if(auto* P=GetPawn()){auto L=P->GetActorLocation();L.Z=FMath::Min(55000.,L.Z+2000);P->SetActorLocation(L);}}
void AFrontlineController::PlayerTick(float Delta){Super::PlayerTick(Delta);if(!IsLocalController()||!GetPawn()||Frame.Phase!=1)return;
 FVector Move=FVector::ZeroVector;Move.X=(IsInputKeyDown(EKeys::W)?1:0)-(IsInputKeyDown(EKeys::S)?1:0);Move.Y=(IsInputKeyDown(EKeys::D)?1:0)-(IsInputKeyDown(EKeys::A)?1:0);FVector P=GetPawn()->GetActorLocation()+Move*Delta*15000;P.X=FMath::Clamp(P.X,-60000.,50000.);P.Y=FMath::Clamp(P.Y,-50000.,50000.);GetPawn()->SetActorLocation(P);
 if(WasInputKeyJustPressed(EKeys::Q)){PendingAction=2;Feedback=TEXT("Attack-move: right-click destination");}if(WasInputKeyJustPressed(EKeys::E)){PendingAction=6;Feedback=TEXT("Mortar smoke: right-click destination");}if(WasInputKeyJustPressed(EKeys::H))Order(4,FVector2D::ZeroVector);if(WasInputKeyJustPressed(EKeys::X))Order(3,FVector2D::ZeroVector);
 const FKey Keys[]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight,EKeys::Nine};for(int I=0;I<9;++I)if(WasInputKeyJustPressed(Keys[I])){if(IsInputKeyDown(EKeys::LeftControl))Groups.Add(I,Selected.Array());else if(auto* Group=Groups.Find(I)){Selected.Empty();for(int Id:*Group)for(auto& U:Frame.Units)if(U.Id==Id&&U.Team==Frame.Team)Selected.Add(Id);}}
}
void AFrontlineController::RefreshVisuals(){TSet<int32> Present;for(auto& U:Frame.Units){Present.Add(U.Id);AActor* Actor=Visuals.FindRef(U.Id);if(!Actor){auto* Mesh=GetWorld()->SpawnActor<AStaticMeshActor>();Mesh->SetMobility(EComponentMobility::Movable);Mesh->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,U.Kind==0||U.Kind==3||U.Kind==4?TEXT("/Engine/BasicShapes/Capsule.Capsule"):TEXT("/Engine/BasicShapes/Cube.Cube")));Mesh->SetActorScale3D(U.Kind==2?FVector(6,3.5,2):U.Kind==0||U.Kind==3||U.Kind==4?FVector(1,1,2):FVector(4,2,2));Mesh->SetActorEnableCollision(false);if(auto* M=LoadObject<UMaterialInterface>(nullptr,U.Team==0?TEXT("/Game/Generated/M_TeamA.M_TeamA"):TEXT("/Game/Generated/M_TeamB.M_TeamB")))Mesh->GetStaticMeshComponent()->SetMaterial(0,M);Actor=Mesh;Visuals.Add(U.Id,Actor);}Actor->SetActorLocation(FVector(U.Position.X*100,U.Position.Y*100,110));Actor->SetActorRotation(FRotator(0,FMath::RadiansToDegrees(U.Heading),0));}
 TArray<int32> Remove;for(auto& Pair:Visuals)if(!Present.Contains(Pair.Key)){if(Pair.Value)Pair.Value->Destroy();Remove.Add(Pair.Key);Selected.Remove(Pair.Key);}for(int Id:Remove)Visuals.Remove(Id);
}
