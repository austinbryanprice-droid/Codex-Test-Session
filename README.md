# Frontline Command — Unreal Engine 5.8.4

The active native project restores the RTS C++ simulation and adds **Cedar Hollow**,
a small playable woodland graphics benchmark. UE 5.8.4 is the version reported by
the project owner. The browser game remains a separate earlier prototype.

## Open on your Windows PC

1. Install Visual Studio 2022 with **Game development with C++**, the MSVC toolchain
   and Windows SDK required by your UE 5.8.4 installation. Unreal's build output
   is authoritative about the supported compiler version.
2. Download this complete project and extract it into a writable local folder
   (for example `C:\Games\FrontlineCommand`). Keep `ArtSource` with it.
3. Double-click **Start-Cedar-Hollow.cmd**. It checks UE 5.8.4, builds the editor
   module, imports the art, creates the scene and opens Unreal Editor.
4. Wait for shader compilation, then press **Play**. Click the viewport to capture
   the mouse. Walk with WASD; Shift speeds up; F toggles flight; Q/E descends/rises;
   Tab releases the mouse. Keys 1/2/3 change golden-hour/overcast/moonlit lighting.
   Escape stops the editor play session.

If Unreal is installed elsewhere, run this in PowerShell from the project folder:

```powershell
.\Scripts\Setup-Windows.ps1 -EngineRoot 'D:\Epic Games\UE_5.8' -Mode Setup
.\Scripts\Setup-Windows.ps1 -EngineRoot 'D:\Epic Games\UE_5.8' -Mode Open
```

Setup needs your actual Unreal installation. It does not install an engine or
require a source-engine checkout. Re-running Setup rebuilds the generated Cedar
level; keep custom art/levels outside `/Game/Cedar`. The script removes its
completion marker first and creates it only after the entire scene succeeds.
Any import/build failure leaves setup incomplete and logs in `Saved/Logs`.

## Native graphics work

- DX12/Shader Model 6, Lumen GI and reflections with hardware ray tracing enabled,
  virtual shadow maps, and Nanite on terrain/architecture.
- Original 180-metre terrain mesh with collision, a closed cabin, rounded roof,
  window recesses, dock and shore rocks. UVs are authored before FBX export.
- CC0 photographed base color, DirectX-converted normal maps and separate ORM
  channels; masked, two-sided vegetation materials; 170 tree placements,
  understory and a native hierarchical instanced grass component with culling.
- Dynamic sun/sky atmosphere, volumetric height fog, bloom, exposure, reflections,
  warm cabin lighting, a native first-person explorer and a minimal command HUD.

This is a **source project and generated import inputs**, not a precompiled game.
The cloud has Blender and a C++ compiler but no Unreal Editor, Windows toolchain,
or RTX GPU. UE compilation, Python API execution, material appearance and GPU
performance have not been validated. Do not describe the visuals as accepted or
photorealistic until inspecting actual UE captures. Pond shading uses an opaque
rippled PBR surface; shoreline foam, water depth absorption and swimming remain
unfinished. The cabin interior, foliage wind, audio ambience and production UMG
menus are subsequent work. Changing engines does not automatically complete the
multiplayer game: the inherited integration remains unverified in UE 5.8.4.

## Package and validate

After successful Setup, run `Scripts/Setup-Windows.ps1 -Mode Package` to build and
package a Development Windows client under `Builds`. A dedicated-server build is
separate and requires a matching source engine. The existing server integration
uses the Frontline game mode, not the Cedar exploration game mode.

The standalone C++ simulation can be checked on Linux using
`./Scripts/test-core.sh`: **139 assertions across ten scenarios pass** with
Address/UndefinedBehavior sanitizers. This does not establish UE build success.

Capture identical views of the pond, cabin and forest at 1440p on the RTX 4090.
Use `stat unit`, `stat gpu` and Unreal Insights; wait for shader compilation and
record resolution, scalability and frame time. Test walking collision, the dock,
lighting changes and free flight. Report the first actual Unreal error with
`Saved/Logs` if Setup fails. See `Docs/NATIVE-VALIDATION.md` for the acceptance list.

`ArtSource/ASSET-PROVENANCE.md` retains upstream sources and licenses. These free
CC0 assets and original generated meshes may be distributed with the project.
Unreal itself is not redistributed. Regeneration inside the full development
repository uses `blender -b -t 2 -P unreal/Scripts/prepare-art.py`.
