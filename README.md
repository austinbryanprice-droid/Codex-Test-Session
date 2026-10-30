# Moonfang — single-character browser lab

An original blue werewolf character in a small mythic courtyard, implemented with
**Three.js 0.180 WebGPURenderer, TypeScript and Vite**. This is a separate asset
inspection demo; it does not replace the military RTS or Unreal project.

## Run

Requires Node 24. Run `npm ci`, then `npm run dev` and open the address printed by
Vite **on that same computer**. `npm run build` creates static output in `dist/`.
`npm run build -- --base=/Codex-Test-Session/moonfang/` targets the published
repository subdirectory. HTTPS or localhost is required for native WebGPU.

- Drag to orbit, scroll to zoom; WASD moves the character within the arena.
- Six animation buttons; Space attacks and R performs the lunar howl.
- Face inspection, three fixed lighting rigs, Low/Balanced/Ultra resolution presets.
- H hides the interface. Export downloads the complete reusable GLB with rig/clips.

## Character pipeline

`art-source/makehuman-base.obj` is the official CC0 hm08 source, retaining its
explicit license header. The body topology is morphed in code and armor plates
are extracted from its surfaces, offset, thickened and bevelled. The wolf skull
uses an original signed-distance field meshed with marching cubes; mane locks,
ears, claws, teeth and clothing are swept or extruded geometry. There is no
hand-sculpted mesh. All generated art is CC0; see ASSET-LICENSES.md.

The original shared humanoid rig has 26 joints and six original baked clips:
Idle, Run, Cast, Attack, Hit and Death. No third-party rig or animation has been
retargeted. Body and plate weights are shared; loincloth parts follow the thighs.
Two additional strips use fixed-step Verlet motion. There is no held weapon in
this clawed character. Finger bones are included; production finger contact and
all clip intersections still need detailed visual review.

The generated GLB is about 4.2 MiB, with 122,594 triangles. Source PBR materials
remain separate in the export. Runtime batches them into two draw groups, with
per-vertex color, roughness, metalness and ray-tested baked occlusion. A procedural
studio environment gives metal reflections; eyes have modeled irises and pupils.
The arena is merged by material; fires use a shared instanced mesh.

## Rendering and measurement

Native WebGPU is requested first with a high-performance preference. A compatible
WebGL2 backend is offered automatically when WebGPU is absent or its device is
lost, and is labelled explicitly. `?webgl=1` forces that backend. The cloud software
WebGPU device repeatedly reset; native GPU rendering is therefore **not validated
on an RTX 4090 or other physical GPU**. The fallback is browser-tested.

The native path has separate glow-only rendering for fire/spells and bloom;
sunlit marble is excluded. FXAA follows composition. The fallback uses direct
MSAA rendering without that bloom stack. A final sharpening pass is not yet
implemented. Three shadow cascades are used; far cascades update every fourth
frame; grass casts no shadows. The character is one caster with two material draws.
The distant animation cadence drops to 24 Hz; a full offscreen character scheduler
is deferred because this scene contains only one character.

The overlay counts scene/shadow submissions via render callbacks and remaining
screen draws from renderer totals. CPU time is **JavaScript simulation/render
submission**, not total frame latency. GPU duration is queried separately when
supported and otherwise shown as unavailable. FPS uses actual frame intervals,
including slow cloud rendering; simulation steps are capped independently.
Warm-up compiles the fixed scene and renders the post stack before interaction.
Switching clips and light colors uses the same materials/light set.

**60 FPS is a target, not a measured acceptance result.** Check the overlay on a
physical GPU at the desired resolution. Cloud SwiftShader screenshots establish
rendering/layout only and run far below 60 FPS. Fine facial anatomy, groomed fur,
cloth collision and hand-crafted surface detail still fall short of the supplied
reference. This is a stylized indie prototype, not an accepted reference match.

## Verification and reproduction

- `npm test`: GLB rig/clips, skin attributes, baked AO and asset budgets.
- `npx playwright test`: real browser controls, distinct clip poses, lighting,
  inspection, HUD, export, and a failed required download. Install Chromium using
  `npx playwright install chromium`, or set CHROMIUM_PATH to your browser.
- `npm run build`: TypeScript and production bundle.
- See `reference/REVIEW.md` and the retained before/after browser screenshots.

To regenerate geometry, install Python NumPy/scikit-image and Blender 4.3, then:

```sh
python scripts/generate-head.py
blender -b -t 2 -P scripts/build-character.py
npm test
```

The supplied source inputs and generated GLB permit offline use; no asset server,
account, paid pack or hosted API is required. Keep the license notices when hosting.
