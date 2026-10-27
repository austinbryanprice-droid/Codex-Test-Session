#include "FrontlineGame.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"
void BuildFrontlineTerrain(UWorld* World){
 for(TActorIterator<AStaticMeshActor> It(World);It;++It)if(It->ActorHasTag(TEXT("FrontlineTerrain")))return;
 if(World->GetNetMode()==NM_DedicatedServer)return;
 auto Shape=[&](const TCHAR* Mesh,FVector Position,FVector Scale,const TCHAR* Material){auto* A=World->SpawnActor<AStaticMeshActor>();A->Tags.Add(TEXT("FrontlineTerrain"));auto* C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Mesh));if(auto* M=LoadObject<UMaterialInterface>(nullptr,Material))C->SetMaterial(0,M);A->SetActorLocation(Position);A->SetActorScale3D(Scale);A->SetActorEnableCollision(false);return A;};
 const TCHAR* Cube=TEXT("/Engine/BasicShapes/Cube.Cube");
 Shape(Cube,{0,0,-80},{1000,1000,1},TEXT("/Game/Generated/M_Ground.M_Ground"));
 Shape(Cube,{0,0,-22},{1000,18,.15},TEXT("/Game/Generated/M_Road.M_Road"));
 Shape(Cube,{0,0,-20},{18,1000,.15},TEXT("/Game/Generated/M_Road.M_Road"));
 Shape(Cube,{6000,0,-30},{32,1000,.2},TEXT("/Game/Generated/M_Water.M_Water"));
 for(float Y:{-22000.f,0.f,23000.f})Shape(Cube,{6000,Y,0},{45,22,.4},TEXT("/Game/Generated/M_Concrete.M_Concrete"));
 frontline::Battle Layout;for(auto& O:Layout.obstacles){Shape(Cube,{O.center.x*100,O.center.y*100,550},{O.radius*1.5,O.radius*1.5,11},TEXT("/Game/Generated/M_Concrete.M_Concrete"));Shape(Cube,{O.center.x*100,O.center.y*100,1120},{O.radius*1.6,O.radius*1.6,.8},TEXT("/Game/Generated/M_Road.M_Road"));}
 for(int X=-4;X<=4;++X)for(int Y=-4;Y<=4;++Y){if(FMath::Abs(X)<=1||FMath::Abs(Y)<=1)continue;Shape(Cube,{float(X*10000),float(Y*10000),-24},{85,75,.1},((X+Y)%2)?TEXT("/Game/Generated/M_Field.M_Field"):TEXT("/Game/Generated/M_Ground.M_Ground"));}
 for(auto& O:Layout.objectives){Shape(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),{O.position.x*100,O.position.y*100,8},{8,8,.15},TEXT("/Game/Generated/M_Objective.M_Objective"));Shape(Cube,{O.position.x*100,O.position.y*100,400},{.3,.3,8},TEXT("/Game/Generated/M_Concrete.M_Concrete"));}
 auto* Sun=World->SpawnActor<ADirectionalLight>();Sun->SetActorRotation(FRotator(-40,-35,0));Sun->GetLightComponent()->SetMobility(EComponentMobility::Movable);Sun->GetLightComponent()->SetIntensity(2.5f);Sun->GetLightComponent()->SetLightColor(FLinearColor(.83,.89,1));
 auto* Sky=World->SpawnActor<ASkyLight>();Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);Sky->GetLightComponent()->SetIntensity(.8f);Sky->GetLightComponent()->SetRealTimeCaptureEnabled(true);
 auto* Fog=World->SpawnActor<AExponentialHeightFog>();Fog->GetComponent()->SetFogDensity(.008f);Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(.48,.55,.61));
}
