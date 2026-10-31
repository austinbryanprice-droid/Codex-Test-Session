import { readFile, writeFile } from "node:fs/promises";
import { zipSync, strToU8 } from "three/addons/libs/fflate.module.js";
const read = (p) => readFile(new URL(p, import.meta.url));
const files = {};
for (const name of [
  "moonfang.glb",
  "moonfang-manifest.json",
  "fur-detail.png",
  "fur-normal.png",
  "fur-cutout.png",
])
  files[name] = new Uint8Array(await read("../public/assets/" + name));
for (const [name, path] of [
  ["ASSET-LICENSES.md", "../ASSET-LICENSES.md"],
  ["CODE-LICENSE.txt", "../LICENSE"],
  ["MAKEHUMAN-CC0.txt", "../art-source/MAKEHUMAN-CC0.txt"],
])
  files[name] = new Uint8Array(await read(path));
files["READ-ME.txt"] = strToU8(
  `Moonfang asset revision 2\n\nOriginal CC0 character derived from the official CC0 MakeHuman base.\nThe GLB contains the complete 26-joint skeleton, six animation clips, PBR materials and embedded fur normal maps.\n\nFor the demo's cutout groom, apply fur-cutout.png (R channel, alpha threshold .35) only where the custom vertex attribute _FURCARD is 1; all other surfaces remain opaque. Apply the _AO attribute to indirect ambient occlusion, not to albedo. Fur detail and normal use UV0 repeated 2.5 times; the cutout uses UV0 without tiling. Preserve skin indices/weights when merging.\n\nThe demo converts original material colors, roughness and metalness to vertex attributes and batches the character into two draw groups. Portable implementation is in src/main.ts on the moonfang-character-lab repository branch. A generic GLB viewer will show the solid clumps unless it implements the cutout attribute.\n\nSource: https://github.com/austinbryanprice-droid/Codex-Test-Session/tree/moonfang-character-lab\nThis revision is not an accepted reference match and has not passed physical-GPU 60 FPS validation.\n`,
);
await writeFile(
  new URL("../public/assets/moonfang-pack.zip", import.meta.url),
  zipSync(files, { level: 6 }),
);
console.log("Asset pack generated");
