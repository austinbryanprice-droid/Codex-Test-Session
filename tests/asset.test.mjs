import { test } from "node:test";
import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
const bytes = await readFile(
  new URL("../public/assets/moonfang.glb", import.meta.url),
);
const size = bytes.readUInt32LE(12),
  doc = JSON.parse(bytes.subarray(20, 20 + size).toString());
test("character contains a shared skinned rig and all six playable animation clips", () => {
  assert.equal(bytes.subarray(0, 4).toString(), "glTF");
  assert.equal(doc.skins.length, 1);
  assert.deepEqual(doc.animations.map((a) => a.name).sort(), [
    "Attack",
    "Cast",
    "Death",
    "Hit",
    "Idle",
    "Run",
  ]);
  assert.ok(doc.skins[0].joints.length >= 18);
  for (const animation of doc.animations)
    assert.ok(animation.channels.length >= 18, animation.name);
});
test("every surface carries skinning and baked occlusion within the download budget", () => {
  assert.ok(bytes.length < 10 * 1024 * 1024);
  let triangles = 0;
  for (const mesh of doc.meshes)
    for (const p of mesh.primitives) {
      for (const name of ["POSITION", "NORMAL", "JOINTS_0", "WEIGHTS_0", "_AO"])
        assert.ok(name in p.attributes, name);
      assert.equal(
        doc.accessors[p.attributes._AO].count,
        doc.accessors[p.attributes.POSITION].count,
      );
      triangles += doc.accessors[p.indices].count / 3;
    }
  assert.ok(triangles < 150000, `${triangles} triangles`);
});
