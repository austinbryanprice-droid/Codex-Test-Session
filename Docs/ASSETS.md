# Asset provenance

No third-party military models, textures, animations, or audio have been downloaded or purchased.

| Content | Source | Status |
| --- | --- | --- |
| Generated battlefield layout and material definitions | Original project code in FrontlineTerrain.cpp and create-editor-assets.py | Source present; Unreal output ungenerated |
| Cube, sphere/capsule, cylinder, base shapes | Unreal Engine `/Engine/BasicShapes` | Governed by applicable Epic/Unreal terms; supplied with the engine |
| Default font and brushes | Unreal Engine Slate CoreStyle | Supplied with engine; applicable Epic terms |
| Production vehicles, infantry, environment, VFX, sound | None | Must be licensed and integrated later |

Record vendor, product URL, license/version, purchaser entitlement, acquisition date, and redistribution constraints before adding any purchased asset. Keep receipts outside public source control. Git LFS patterns cover `.uasset`, `.umap`, `.fbx`, `.wav`, and `.png`; install and configure Git LFS before committing binary assets. LFS has not been configured or exercised in this cloud.
