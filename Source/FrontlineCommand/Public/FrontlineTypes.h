#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FrontlineTypes.generated.h"
class UStaticMesh;
class UMaterialInterface;
USTRUCT(BlueprintType)
struct FFrontlineUnitView {
 GENERATED_BODY()
 UPROPERTY(BlueprintReadOnly) int32 Id=0;
 UPROPERTY(BlueprintReadOnly) int32 Team=0;
 UPROPERTY(BlueprintReadOnly) int32 Kind=0;
 UPROPERTY(BlueprintReadOnly) FVector2D Position=FVector2D::ZeroVector;
 UPROPERTY(BlueprintReadOnly) float Heading=0;
 UPROPERTY(BlueprintReadOnly) float Health=0;
 UPROPERTY(BlueprintReadOnly) float Suppression=0;
 UPROPERTY(BlueprintReadOnly) int32 Ammo=0;
};
USTRUCT(BlueprintType)
struct FFrontlineContact {
 GENERATED_BODY()
 UPROPERTY(BlueprintReadOnly) FVector2D Position=FVector2D::ZeroVector;
 UPROPERTY(BlueprintReadOnly) float Age=0;
};
USTRUCT(BlueprintType)
struct FFrontlineFrame {
 GENERATED_BODY()
 UPROPERTY(BlueprintReadOnly) TArray<FFrontlineUnitView> Units;
 UPROPERTY(BlueprintReadOnly) TArray<FFrontlineContact> Contacts;
 UPROPERTY(BlueprintReadOnly) TArray<int32> Owners;
 UPROPERTY(BlueprintReadOnly) TArray<float> Capture;
 UPROPERTY(BlueprintReadOnly) TArray<FVector2D> Smoke;
 UPROPERTY(BlueprintReadOnly) int32 Team=-1;
 UPROPERTY(BlueprintReadOnly) int32 Phase=0;
 UPROPERTY(BlueprintReadOnly) int32 Winner=-2;
 UPROPERTY(BlueprintReadOnly) int32 TicketsA=1000;
 UPROPERTY(BlueprintReadOnly) int32 TicketsB=1000;
 UPROPERTY(BlueprintReadOnly) int32 Credits=0;
 UPROPERTY(BlueprintReadOnly) float Elapsed=0;
 UPROPERTY(BlueprintReadOnly) int32 PlayersConnected=0;
 UPROPERTY(BlueprintReadOnly) int32 PlayersReady=0;
 UPROPERTY(BlueprintReadOnly) float OpponentGrace=0;
};
// Presentation override points for licensed art. Built-in geometry works without assets.
UCLASS(BlueprintType)
class FRONTLINECOMMAND_API UFrontlineUnitPresentation : public UPrimaryDataAsset {
 GENERATED_BODY()
 public:
 UPROPERTY(EditAnywhere,BlueprintReadOnly) FText DisplayName;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) TSoftObjectPtr<UStaticMesh> Mesh;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) TSoftObjectPtr<UMaterialInterface> Material;
 UPROPERTY(EditAnywhere,BlueprintReadOnly) FVector VisualScale=FVector(1);
};
