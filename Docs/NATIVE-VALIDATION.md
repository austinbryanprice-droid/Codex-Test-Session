# UE 5.8.4 validation status

Verified in cloud:
- Blender FBX export and round-trip import succeed for all seven required mesh sets.
- Ground measures 180 metres; all imported meshes have UVs and bound material slots.
- All required photographic material inputs are present and staged.
- Python scripts parse successfully; project/config files target UE 5.8.4.
- Actual simulation: 139 assertions, ten scenarios; sanitizers enabled.

Not run (requires your installed UE/Windows/GPU):
- UnrealBuildTool, UHT, shader compilation, editor Python imports and saved map.
- Cook/package and launching a Windows executable.
- Native graphics screenshots, shadows/reflections, collision, input and FPS.
- Dedicated-server replication and multiplayer acceptance.

First workstation gate:
1. Run Setup. Retain Saved/Logs; require Saved/CedarAssets.ready.
2. Open /Game/Cedar/Maps/CedarHollow, wait for compilation; press Play.
3. Inspect grass transparency, vegetation UV scale, ground contact, cabin seams,
   normal-map direction, water reflections and contact shadows.
4. Walk onto the dock, fly above the forest, switch all three lighting moods.
5. Capture actual 1440p renders and measure the RTX 4090 with stat gpu/stat unit.
6. Package a Development client and repeat outside the editor.

Do not claim this gate passed from script syntax checks or Blender export alone.
