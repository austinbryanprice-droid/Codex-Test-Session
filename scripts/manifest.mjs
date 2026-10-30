import { readFile, writeFile } from "node:fs/promises";
import { createHash } from "node:crypto";
const url = new URL("../public/assets/moonfang.glb", import.meta.url),
  b = await readFile(url),
  d = JSON.parse(b.subarray(20, 20 + b.readUInt32LE(12)).toString());
const data = {
  name: "Moonfang",
  version: 1,
  license: "CC0-1.0",
  url: "moonfang.glb",
  metresTall: 3.8,
  bytes: b.length,
  sha256: createHash("sha256").update(b).digest("hex"),
  joints: d.skins[0].joints.length,
  clips: d.animations.map((a) => a.name),
  triangles: d.meshes
    .flatMap((m) => m.primitives)
    .reduce((n, p) => n + d.accessors[p.indices].count / 3, 0),
  runtimeDrawGroups: 2,
};
await writeFile(
  new URL("../public/assets/moonfang-manifest.json", import.meta.url),
  JSON.stringify(data, null, 2) + "\n",
);
console.log(data);
