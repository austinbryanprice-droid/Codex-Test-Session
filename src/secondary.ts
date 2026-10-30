import * as T from "three/webgpu";
/** Two small hip cloth strips: fixed-step Verlet, pinned to the shared rig. */
export function secondary(root: T.Object3D, skeleton: T.Skeleton) {
  const strips = 2,
    rows = 7,
    n = strips * rows * 2,
    position = new Float32Array(n * 3),
    previous = new Float32Array(n * 3),
    indices: number[] = [];
  const anchors = [
    skeleton.getBoneByName("thigh.L"),
    skeleton.getBoneByName("thigh.R"),
  ];
  const world = new T.Vector3(),
    anchor = new T.Vector3(),
    inverse = new T.Matrix4();
  let initialized = false,
    accumulator = 0;
  for (let s = 0; s < strips; s++)
    for (let r = 0; r < rows - 1; r++) {
      const a = s * rows * 2 + r * 2;
      indices.push(a, a + 2, a + 1, a + 1, a + 2, a + 3);
    }
  const geometry = new T.BufferGeometry();
  geometry.setAttribute("position", new T.BufferAttribute(position, 3));
  geometry.setIndex(indices);
  const cloth = new T.Mesh(
    geometry,
    new T.MeshStandardMaterial({
      color: 0x384551,
      roughness: 1,
      side: T.DoubleSide,
    }),
  );
  cloth.frustumCulled = false;
  root.add(cloth);
  function update(dt: number, t: number) {
    root.updateWorldMatrix(true, false);
    inverse.copy(root.matrixWorld).invert();
    accumulator = Math.min(accumulator + dt, 0.05);
    while (accumulator >= 1 / 60) {
      accumulator -= 1 / 60;
      for (let s = 0; s < strips; s++) {
        const bone = anchors[s];
        if (!bone) continue;
        bone.getWorldPosition(world);
        anchor.copy(world).applyMatrix4(inverse);
        anchor.y += 0.04;
        anchor.z -= 0.12;
        for (let r = 0; r < rows; r++)
          for (let k = 0; k < 2; k++) {
            const i = (s * rows * 2 + r * 2 + k) * 3;
            if (!initialized || r === 0) {
              position[i] = anchor.x + (k - 0.5) * 0.16;
              position[i + 1] = anchor.y - r * 0.055;
              position[i + 2] = anchor.z;
              previous[i] = position[i];
              previous[i + 1] = position[i + 1];
              previous[i + 2] = position[i + 2];
              continue;
            }
            const x = position[i],
              y = position[i + 1],
              z = position[i + 2];
            position[i] +=
              (x - previous[i]) * 0.94 + 0.00005 * Math.sin(t * 2 + s);
            position[i + 1] += (y - previous[i + 1]) * 0.94 - 0.0006;
            position[i + 2] +=
              (z - previous[i + 2]) * 0.94 +
              0.00012 * Math.sin(t * 3 + r * 0.2);
            previous[i] = x;
            previous[i + 1] = y;
            previous[i + 2] = z;
          }
        for (let iteration = 0; iteration < 3; iteration++)
          for (let r = 1; r < rows; r++)
            for (let k = 0; k < 2; k++) {
              const i = (s * rows * 2 + r * 2 + k) * 3,
                j = i - 6;
              const x = position[i] - position[j],
                y = position[i + 1] - position[j + 1],
                z = position[i + 2] - position[j + 2],
                length = Math.hypot(x, y, z) || 1,
                scale = 0.055 / length;
              position[i] = position[j] + x * scale;
              position[i + 1] = position[j + 1] + y * scale;
              position[i + 2] = Math.min(
                anchor.z + 0.02,
                position[j + 2] + z * scale,
              );
            }
      }
      initialized = true;
    }
    geometry.attributes.position.needsUpdate = true;
    geometry.computeVertexNormals();
  }
  return { update };
}
