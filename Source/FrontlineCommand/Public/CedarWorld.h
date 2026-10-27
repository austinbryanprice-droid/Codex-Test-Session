#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "CedarWorld.generated.h"
class UCameraComponent;

UCLASS()
class FRONTLINECOMMAND_API ACedarExplorer : public ACharacter {
 GENERATED_BODY()
public:
 ACedarExplorer();
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
private:
 UPROPERTY() TObjectPtr<UCameraComponent> Camera;
 void Forward(float Value); void Right(float Value); void Rise(float Value);
 void ToggleFlight(); void Sprint(); void Walk(); void ReleaseMouse();
 void Golden(); void Overcast(); void Moonlight();
 void SetLighting(int32 Mood);
};

UCLASS()
class FRONTLINECOMMAND_API ACedarHUD : public AHUD {
 GENERATED_BODY()
public: virtual void DrawHUD() override;
};

UCLASS()
class FRONTLINECOMMAND_API ACedarGameMode : public AGameModeBase {
 GENERATED_BODY()
public: ACedarGameMode();
};

class UStaticMesh;
class UHierarchicalInstancedStaticMeshComponent;
UCLASS()
class FRONTLINECOMMAND_API ACedarGrass : public AActor {
 GENERATED_BODY()
public:
 ACedarGrass();
 UPROPERTY(EditAnywhere, Category="Cedar") TObjectPtr<UStaticMesh> GrassMesh;
 UPROPERTY(VisibleAnywhere, Category="Cedar") TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Instances;
 UFUNCTION(BlueprintCallable, CallInEditor, Category="Cedar") void Rebuild();
};
