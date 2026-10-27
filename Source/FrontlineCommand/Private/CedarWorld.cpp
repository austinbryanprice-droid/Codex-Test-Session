#include "CedarWorld.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Canvas.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ACedarExplorer::ACedarExplorer() {
 GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
 Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("ExplorerCamera"));
 Camera->SetupAttachment(GetCapsuleComponent()); Camera->SetRelativeLocation(FVector(0,0,64));
 Camera->bUsePawnControlRotation=true; Camera->FieldOfView=80;
 GetCharacterMovement()->MaxWalkSpeed=330; GetCharacterMovement()->BrakingDecelerationFlying=1200;
 GetCharacterMovement()->MaxFlySpeed=650;
 bUseControllerRotationYaw=true;
}
void ACedarExplorer::Forward(float V) { if(Controller) AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),V); }
void ACedarExplorer::Right(float V) { if(Controller) AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V); }
void ACedarExplorer::Rise(float V) { if(GetCharacterMovement()->IsFlying()) AddMovementInput(FVector::UpVector,V); }
void ACedarExplorer::ToggleFlight() {
 auto* M=GetCharacterMovement();
 M->SetMovementMode(M->IsFlying()?MOVE_Falling:MOVE_Flying);
}
void ACedarExplorer::Sprint() { GetCharacterMovement()->MaxWalkSpeed=650; GetCharacterMovement()->MaxFlySpeed=1600; }
void ACedarExplorer::Walk() { GetCharacterMovement()->MaxWalkSpeed=330; GetCharacterMovement()->MaxFlySpeed=650; }
void ACedarExplorer::ReleaseMouse() {
 if(auto* P=Cast<APlayerController>(Controller)) {
  P->bShowMouseCursor=!P->bShowMouseCursor;
  if(P->bShowMouseCursor) { P->SetInputMode(FInputModeGameAndUI()); }
  else { P->SetInputMode(FInputModeGameOnly()); }
 }
}
void ACedarExplorer::Golden(){SetLighting(0);} void ACedarExplorer::Overcast(){SetLighting(1);} void ACedarExplorer::Moonlight(){SetLighting(2);}
void ACedarExplorer::SetLighting(int32 Mood) {
 for(TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It) if(It->ActorHasTag(TEXT("CedarSun"))) {
  It->SetActorRotation(FRotator(Mood==1?-65:-18,-32,0));
  auto* L=Cast<UDirectionalLightComponent>(It->GetLightComponent());
  if(!L) continue;
  L->SetIntensity(Mood==2?0.5f:Mood==1?16000.f:65000.f);
  L->SetLightColor(Mood==2?FLinearColor(.42,.61,1):Mood==1?FLinearColor(.83,.9,1):FLinearColor(1,.79,.53));
 }
 for(TActorIterator<AExponentialHeightFog> It(GetWorld());It;++It) if(It->ActorHasTag(TEXT("CedarFog"))) {
  It->GetComponent()->SetFogDensity(Mood==1?.018f:Mood==2?.012f:.007f);
 }
}
void ACedarExplorer::SetupPlayerInputComponent(UInputComponent* I) {
 Super::SetupPlayerInputComponent(I);
 I->BindAxis("CedarForward",this,&ACedarExplorer::Forward); I->BindAxis("CedarRight",this,&ACedarExplorer::Right);
 I->BindAxis("CedarRise",this,&ACedarExplorer::Rise);
 I->BindAxis("CedarLookX",this,&APawn::AddControllerYawInput); I->BindAxis("CedarLookY",this,&APawn::AddControllerPitchInput);
 I->BindAction("CedarFlight",IE_Pressed,this,&ACedarExplorer::ToggleFlight);
 I->BindAction("CedarSprint",IE_Pressed,this,&ACedarExplorer::Sprint); I->BindAction("CedarSprint",IE_Released,this,&ACedarExplorer::Walk);
 I->BindAction("CedarMouse",IE_Pressed,this,&ACedarExplorer::ReleaseMouse);
 I->BindAction("CedarGolden",IE_Pressed,this,&ACedarExplorer::Golden); I->BindAction("CedarOvercast",IE_Pressed,this,&ACedarExplorer::Overcast); I->BindAction("CedarMoonlight",IE_Pressed,this,&ACedarExplorer::Moonlight);
}
ACedarGameMode::ACedarGameMode() { DefaultPawnClass=ACedarExplorer::StaticClass(); HUDClass=ACedarHUD::StaticClass(); }
void ACedarHUD::DrawHUD() {
 Super::DrawHUD(); if(!Canvas) return;
 DrawRect(FLinearColor(.018,.028,.024,.78),24,24,430,104);
 DrawText(TEXT("CEDAR HOLLOW  /  UNREAL ENVIRONMENT LAB"),FLinearColor(.92,.85,.67),42,38,GEngine->GetSmallFont(),1.15f);
 DrawText(TEXT("WASD  walk   |   mouse  look   |   Shift  faster"),FLinearColor(.86,.89,.84),42,67,GEngine->GetSmallFont());
 DrawText(TEXT("F  fly   |   Q/E  down/up   |   Tab  release mouse"),FLinearColor(.86,.89,.84),42,87,GEngine->GetSmallFont());
 DrawText(TEXT("1  Golden hour    2  Overcast    3  Moonlight"),FLinearColor(.92,.85,.67),42,107,GEngine->GetSmallFont());
 if(auto* P=UGameplayStatics::GetPlayerPawn(GetWorld(),0)) if(auto* C=Cast<ACedarExplorer>(P))
  DrawText(C->GetCharacterMovement()->IsFlying()?TEXT("FREE FLIGHT"):TEXT("ON FOOT"),FLinearColor(.92,.85,.67),30,Canvas->SizeY-38,GEngine->GetSmallFont());
}

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
ACedarGrass::ACedarGrass() {
 Instances=CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Grass"));
 SetRootComponent(Instances);Instances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Instances->SetCullDistances(4500,7000);PrimaryActorTick.bCanEverTick=false;
}
void ACedarGrass::Rebuild() {
 Instances->ClearInstances();Instances->SetStaticMesh(GrassMesh);if(!GrassMesh)return;
 FRandomStream R(916);
 for(int32 I=0;I<11000;++I) {
  float X=R.FRandRange(-48,48),Y=R.FRandRange(-48,48);
  float Q=FMath::Sqrt(FMath::Square(X/11)+FMath::Square((Y-4)/8));
  if(Q<1.4f || (X>-23 && X<-10 && Y>7 && Y<18) || (FMath::Abs(X)<2 && Y>-10 && Y<3)) continue;
  float Base=.4f+FMath::Sin(X*.13f)*FMath::Cos(Y*.12f)*.4f+FMath::Max(0.0,FVector2D(X,Y).Size()-32.0)*.1f;
  float T=FMath::Clamp((Q-.9f)/.45f,0.f,1.f), Z=-.95f+(Base+.95f)*T*T*(3-2*T);
  float S=R.FRandRange(.6f,1.1f);
  Instances->AddInstance(FTransform(FRotator(0,R.FRandRange(0,360),0),FVector(X*100,Y*100,Z*100),FVector(S)));
 }
}
