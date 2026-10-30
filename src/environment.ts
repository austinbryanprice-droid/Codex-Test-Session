import * as T from "three/webgpu";
import { mergeGeometries } from "three/addons/utils/BufferGeometryUtils.js";
export function environment(scene: T.Scene) {
  function flutedColumn() {
    const g = new T.CylinderGeometry(0.28, 0.34, 3.8, 96, 3);
    const p = g.attributes.position;
    for (let i = 0; i < p.count; i++) {
      const x = p.getX(i),
        z = p.getZ(i),
        a = Math.atan2(x, z),
        f = 1 + 0.045 * Math.cos(a * 16);
      p.setXYZ(i, x * f, p.getY(i), z * f);
    }
    g.computeVertexNormals();
    return g;
  }
  const batches = new Map<T.Material, T.BufferGeometry[]>();
  const marble = new T.MeshStandardMaterial({
      color: 0xddd4bc,
      roughness: 0.8,
    }),
    stone = new T.MeshStandardMaterial({ color: 0x829192, roughness: 0.95 }),
    bronze = new T.MeshStandardMaterial({
      color: 0x80552c,
      metalness: 0.85,
      roughness: 0.3,
    }),
    dark = new T.MeshStandardMaterial({ color: 0x354e55, roughness: 0.8 }),
    grass = new T.MeshStandardMaterial({ color: 0x6c8758, roughness: 1 });
  const matrix = new T.Matrix4(),
    q = new T.Quaternion();
  function add(
    g: T.BufferGeometry,
    m: T.Material,
    x: number,
    y: number,
    z: number,
    sx = 1,
    sy = 1,
    sz = 1,
    ry = 0,
  ) {
    q.setFromAxisAngle(T.Object3D.DEFAULT_UP, ry);
    matrix.compose(new T.Vector3(x, y, z), q, new T.Vector3(sx, sy, sz));
    if (g.index) g = g.toNonIndexed();
    g.applyMatrix4(matrix);
    const list = batches.get(m) || [];
    list.push(g);
    batches.set(m, list);
  }
  add(new T.CylinderGeometry(6.6, 6.8, 0.28, 64), marble, 0, -0.18, 0);
  add(new T.CylinderGeometry(1.9, 2, 0.22, 64), dark, 0, 0.02, 0);
  add(
    new T.TorusGeometry(1.86, 0.025, 6, 64).rotateX(-Math.PI / 2),
    bronze,
    0,
    0.135,
    0,
    1,
    1,
    1,
    0,
  );
  // Terrace rings, geometric inlays, and deliberately oversized columns.
  for (let i = 0; i < 16; i++) {
    const a = (i / 16) * Math.PI * 2;
    if (Math.sin(a) > 0.15) continue;
    const x = Math.cos(a) * 5.8,
      z = Math.sin(a) * 5.8;
    add(new T.CylinderGeometry(0.49, 0.54, 0.22, 12), marble, x, 0.11, z);
    add(flutedColumn(), marble, x, 2.1, z);
    add(new T.CylinderGeometry(0.5, 0.38, 0.22, 12), marble, x, 4.12, z);
    add(
      new T.BoxGeometry(2.3, 0.34, 0.8),
      marble,
      x,
      4.4,
      z,
      1,
      1,
      1,
      -a + Math.PI / 2,
    );
    add(new T.CylinderGeometry(0.36, 0.38, 0.08, 12), bronze, x, 3.95, z);
  }
  for (let i = 0; i < 60; i++) {
    const a = (i / 60) * Math.PI * 2;
    add(
      new T.BoxGeometry(0.025, 0.012, 0.45),
      bronze,
      Math.cos(a) * 2.8,
      0.005,
      Math.sin(a) * 2.8,
      1,
      1,
      1,
      -a,
    );
  }
  for (let i = 0; i < 9; i++)
    add(
      new T.BoxGeometry(2.3, 0.15, 1.4),
      marble,
      0,
      -0.2 - i * 0.12,
      7 + i * 1.1,
    );
  // Distant layered cliffs stay beyond the pale 30m fog start.
  for (let i = 0; i < 36; i++) {
    const a = (i / 36) * Math.PI * 2,
      r = 32 + (i % 4) * 4;
    add(
      new T.DodecahedronGeometry(1, 0),
      stone,
      Math.cos(a) * r,
      1.5,
      Math.sin(a) * r,
      3 + (i % 4),
      5 + (i % 6),
      3,
      i * 2.3,
    );
  }
  const flamePositions: T.Vector3[] = [];
  for (const [x, z] of [
    [-3.6, -2],
    [3.6, -2],
    [-3.8, 3],
    [3.8, 3],
  ]) {
    add(new T.CylinderGeometry(0.3, 0.45, 1.1, 10), dark, x, 0.45, z);
    add(new T.CylinderGeometry(0.48, 0.25, 0.22, 12), bronze, x, 1.05, z);
    flamePositions.push(new T.Vector3(x, 1.43, z));
  }
  // Banners and fountain create a small, readable test arena.
  const cloth = new T.MeshStandardMaterial({
    color: 0x315c68,
    roughness: 1,
    side: T.DoubleSide,
  });
  for (const s of [-1, 1]) {
    add(new T.CylinderGeometry(0.025, 0.025, 4, 8), bronze, s * 4.5, 2, -3.5);
    add(new T.BoxGeometry(1.1, 1.6, 0.05), cloth, s * 4.5, 2.7, -3.5);
    add(new T.IcosahedronGeometry(0.12, 1), bronze, s * 4.5, 4.1, -3.5);
  }
  add(new T.CylinderGeometry(1.2, 1.3, 0.45, 40), marble, 0, 0.1, -4.4);
  add(
    new T.CylinderGeometry(0.95, 0.95, 0.03, 40),
    new T.MeshStandardMaterial({
      color: 0x358b9b,
      metalness: 0.4,
      roughness: 0.13,
    }),
    0,
    0.35,
    -4.4,
  );
  add(new T.CylinderGeometry(0.15, 0.3, 1.5, 16), marble, 0, 0.8, -4.4);
  add(new T.SphereGeometry(0.3, 16, 12), bronze, 0, 1.65, -4.4);
  for (let i = 0; i < 160; i++) {
    const a = i * 2.3999,
      r = 6.9 + (i % 13) * 0.33;
    add(
      new T.ConeGeometry(0.13, 0.38, 4),
      grass,
      Math.cos(a) * r,
      0.08,
      Math.sin(a) * r,
    );
  }
  for (const [mat, list] of batches) {
    const merged = mergeGeometries(list);
    if (!merged) throw Error("Static arena batching failed");
    const mesh = new T.Mesh(merged, mat);
    mesh.castShadow = mat !== grass;
    mesh.receiveShadow = true;
    mesh.name = "arena-batch";
    scene.add(mesh);
    list.forEach((g) => g.dispose());
  }
  const flameMat = new T.MeshStandardMaterial({
    color: 0xff9f3e,
    emissive: 0xff751a,
    emissiveIntensity: 4,
    roughness: 1,
  });
  const fire = new T.InstancedMesh(
    new T.IcosahedronGeometry(0.22, 1),
    flameMat,
    flamePositions.length * 3,
  );
  fire.castShadow = false;
  scene.add(fire);
  function update(t: number) {
    for (let i = 0; i < flamePositions.length * 3; i++) {
      const p = flamePositions[Math.floor(i / 3)];
      matrix.makeScale(0.75, 0.9 + 0.3 * Math.sin(t * 8 + i), 0.75);
      matrix.setPosition(
        p.x + Math.sin(i * 2) * 0.1,
        p.y + (i % 3) * 0.14,
        p.z,
      );
      fire.setMatrixAt(i, matrix);
    }
    fire.instanceMatrix.needsUpdate = true;
  }
  return { update, fire };
}
