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
