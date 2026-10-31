# Screenshot-first visual record

Reference: the single blue-furred upright werewolf image attached by the user.
Visible observations: long canine muzzle, open dark mouth, amber eyes, thick
shoulders/forearms, swept mane, charcoal cloth, ivory claws and gold wrist bands.
The source image itself is not copied into the distributable; no promotional or
AI-rendered image is presented as an in-engine result.

## Round 1 — actual browser baseline

`round-01/full.png` and `round-01/face.png` (fallback renderer).
Three biggest gaps: missing wolf muzzle caused by an OBJ up-axis mismatch;
slight torso beside oversized mane; foreground columns blocking the asset.
Actions: construct the SDF mesh directly in its authored coordinate system;
broaden the source body and inflate limb surfaces; open the camera-side colonnade.

## Intermediate failures

`round-02/` records a blank post-processing output. `round-04/` and `round-05/`
record failed native software-WebGPU attempts. They are retained as failures,
not accepted comparison images. Device-loss recovery and a direct fallback
rendering path were added; native GPU acceptance remains open.

## Final retained round

`final/full.png` and `final/face.png` are real fallback-browser renders.
Changes: darker blue fur, smaller recessed eyes and nose, dark oral cavity,
visible muzzle/jaw, thicker chest/limbs, surface-derived armor, fitted wrist
ornaments, baked AO, a less obstructed courtyard, and fluted column geometry.

Remaining three largest reference gaps:
1. Fur is broad procedural locks rather than the reference's dense layered groom.
2. Facial anatomy, teeth/gum transitions and hand/claw contact need art refinement.
3. The silhouette and animation poses remain a stylized approximation; stronger
   crouch/digitigrade anatomy and richer muscular transitions are still needed.

The topology was reduced from roughly 170K to 123K triangles after the new detail.
Two runtime character draw groups remain. Physical-GPU frame times have not been
measured; no claim of maintaining 60 FPS is made from software-browser captures.
The final generated GLB passes rig, clip, skin-attribute, AO and size checks.

Production base-path validation also passes: the actual static bundle loads at
`/Codex-Test-Session/moonfang/`, the software WebGPU device-loss flow recovers to
a nonblank WebGL2 render, and no HTTP/JavaScript errors remain after recovery.

## Continued loop: iterations 6–13

Each directory `iteration-06-before/` through `iteration-13-after/` contains actual
fixed-camera comparison renders. Failed/intermediate appearances are retained.

- 6: replaced broad plates with undercoat and surface-derived shorts, added body
  fur flow/UVs and a shared reference pose. Clump roots were too bulky.
- 7: flattened the clumps and changed the nose/eyes; posing exposed a binding error.
- 8: attached the skull and facial features to the same head bone, authored swept
  digits and curved talons, reduced coarse chest clumps.
- 9: measured and fixed inward skull triangle winding. Previous images were
  exposing the inside of the skull. An outward-face regression now checks this.
- 10: attached tooth roots to ray-tested jaw surfaces; moved eyes into skull
  sockets; narrowed features, retuned UV detail and lighting.
- 11: corrected occlusion handling to indirect light, increased bake samples,
  tested separate shading inputs. Rejected a white-material export regression.
- 12: restored exported blue material factors, changed head UV projection,
  implemented masked fur clumps, removed incorrect purple exterior mouth patches.
- 13: filled the collar beneath the groom, added a heavier trapezius silhouette,
  narrowed the skull and jaw, repositioned the eyes and made the shorts charcoal.

Latest geometry: 61,027 triangles, versus the earlier 122,594. Two character draw
groups remain. Extra texture sampling means triangle reduction alone does not
prove a frame-time improvement. Native WebGPU still resets on the cloud software
device; physical RTX 4090 visual/performance acceptance cannot be run here.

`public/review/` provides a portable before/after board and local-only reference
image selection. The supplied chat image is not silently replaced with another
reference or an AI-generated render. The reference itself is not embedded because
no attachment file was available to this workspace.

**Status: not an accepted reference match.** The pose, palette and silhouette are
closer, but the groom, fine facial anatomy and hands still visibly differ. Do not
mark the goal complete from these captures or the passing functional tests.

- 14: added cheek-sideburn clumps, preserved the shared pose under Cast/Hit clips,
  and relaxed the limbs during Death. Captured and inspected all six clips in
  `iteration-14-poses/`; the screenshot contact sheet is rendered from those
  actual images, not a concept render.

Final continued-loop mesh: 67,033 triangles; the main character still has two
draw groups (the separate two-strip cloth mesh uses one additional draw).
Four asset/regression checks and two browser cases pass. Source skull winding
and exported blue material factors have dedicated regression checks.

The final comparison is **still not reference-accurate**. The fine groom,
facial sculpt and hand anatomy visibly differ. Native WebGPU/RTX 4090 appearance
and the 60 FPS gate cannot be checked here: the cloud has no NVIDIA GPU device,
and its software WebGPU device resets. A native-renderer screenshot and timing
readout from the target PC are needed before target-renderer acceptance or
further tuning against that rendering can be assessed.

Final native-startup diagnosis: the added fur channels initially exceeded the
WebGPU minimum of eight vertex-buffer streams. Floating-point channels are now
interleaved, with a separate integer skin-index stream (two streams total).
A browser regression checks the guaranteed buffer limit. Native validation errors
and device loss now both trigger explicit fallback recovery. The subsequent
software-WebGPU attempt reported "Device was destroyed"; fallback startup and
rendering succeeded. This does not establish native RTX 4090 correctness.
