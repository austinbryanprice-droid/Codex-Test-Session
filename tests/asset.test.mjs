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

test("wolf skull outer faces point outward, preventing inside-out renders", async () => {
  const text = await readFile(
    new URL("../art-source/head.obj", import.meta.url),
    "utf8",
  );
  const vertices = [],
    faces = [];
  for (const line of text.split("\n")) {
    if (line.startsWith("v "))
      vertices.push(line.split(/\s+/).slice(1).map(Number));
    if (line.startsWith("f "))
      faces.push(
        line
          .split(/\s+/)
          .slice(1)
          .map((n) => Number(n) - 1),
      );
  }
  let checked = 0,
    outward = 0;
  for (const face of faces) {
    const [a, b, c] = face.map((i) => vertices[i]);
    const y = (a[1] + b[1] + c[1]) / 3,
      z = (a[2] + b[2] + c[2]) / 3;
    if (y <= 0.22 || z < 3.1 || z > 3.4) continue;
    const normalY =
      (b[2] - a[2]) * (c[0] - a[0]) - (b[0] - a[0]) * (c[2] - a[2]);
    checked++;
    if (normalY > 0) outward++;
  }
  assert.ok(checked > 100);
  assert.ok(
    outward / checked > 0.95,
    `${outward}/${checked} outward rear faces`,
  );
});
test("exported fur retains its blue PBR factors rather than white exporter defaults", () => {
  const surfaces = doc.materials.filter((m) =>
    /hide|fur|mane|roots/.test(m.name),
  );
  assert.ok(surfaces.length >= 3);
  for (const m of surfaces) {
    const c = m.pbrMetallicRoughness.baseColorFactor;
    assert.ok(c, `${m.name} missing color`);
    assert.ok(c[2] > c[0] * 1.4, m.name);
    assert.ok(c[2] < 0.4, m.name);
  }
});
