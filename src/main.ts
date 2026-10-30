import * as T from "three/webgpu";
import { attribute, pass, mrt, output, emissive } from "three/tsl";
import { bloom } from "three/addons/tsl/display/BloomNode.js";
import { fxaa } from "three/addons/tsl/display/FXAANode.js";
import { OrbitControls } from "three/addons/controls/OrbitControls.js";
import { GLTFLoader } from "three/addons/loaders/GLTFLoader.js";
import { RoomEnvironment } from "three/addons/environments/RoomEnvironment.js";
import { mergeGeometries } from "three/addons/utils/BufferGeometryUtils.js";
import { CSMShadowNode } from "three/addons/csm/CSMShadowNode.js";
import { secondary } from "./secondary";
import { environment } from "./environment";
import "./style.css";
const $ = <E extends HTMLElement = HTMLElement>(id: string) =>
  document.getElementById(id) as E;
const error = (e: unknown) => {
  $("error").hidden = false;
  $("error").textContent =
    `The character lab could not start: ${e instanceof Error ? e.message : String(e)}. Reload to retry. Use a current desktop Chrome or Edge browser.`;
  console.error(e);
};
async function start() {
  const canvas = $<HTMLCanvasElement>("world"),
    query = new URLSearchParams(location.search);
  let deviceLost = false;
  const renderer = new T.WebGPURenderer({
    canvas,
    antialias: true,
    powerPreference: "high-performance",
    forceWebGL: query.has("webgl") || !("gpu" in navigator),
    trackTimestamp: true,
  });
  renderer.onDeviceLost = (info) => {
    deviceLost = true;
    renderer.setAnimationLoop(null);
    if (!query.has("webgl")) {
      const retry = new URL(location.href);
      retry.searchParams.set("webgl", "1");
      retry.searchParams.set("recovered", "1");
      location.replace(retry);
    } else error(info.message);
  };
  await renderer.init();
  renderer.setPixelRatio(Math.min(devicePixelRatio, 1.5));
  renderer.setSize(innerWidth, innerHeight);
  renderer.toneMapping = T.ACESFilmicToneMapping;
  renderer.toneMappingExposure = 1.15;
  renderer.shadowMap.enabled = true;
  renderer.info.autoReset = false;
  $("backend").textContent = (
    renderer.backend as unknown as { isWebGPUBackend?: boolean }
  ).isWebGPUBackend
    ? "WEBGPU"
    : "WEBGL2 FALLBACK";
  let gpuName = "Adapter details unavailable";
  const info = (
    renderer.backend as unknown as {
      device?: {
        adapterInfo?: {
          vendor?: string;
          architecture?: string;
          description?: string;
        };
      };
    }
  ).device?.adapterInfo;
  if (info)
    gpuName =
      [info.vendor, info.architecture, info.description]
        .filter(Boolean)
        .join(" ") || gpuName;
  if (query.has("recovered"))
    gpuName =
      "WebGPU device reset; compatible renderer active. Retry with ?gpu=1 on a supported driver.";
  $("gpu").textContent = /intel|integrated|apple/i.test(gpuName)
    ? `${gpuName} · If a dedicated GPU is available, select High performance for this browser in OS Graphics settings.`
    : gpuName;
  const scene = new T.Scene();
  scene.background = new T.Color(0x9eadae);
  scene.fog = new T.Fog(0xb5c2c0, 30, 85);
  const camera = new T.PerspectiveCamera(
    38,
    innerWidth / innerHeight,
    0.05,
    100,
  );
  camera.position.set(5.7, 3.5, 8.4);
  const controls = new OrbitControls(camera, canvas);
  controls.target.set(0, 1.9, 0);
  controls.enableDamping = true;
  controls.minDistance = 2;
  controls.maxDistance = 17;
  controls.maxPolarAngle = Math.PI * 0.48;
  const pmrem = new T.PMREMGenerator(renderer);
  scene.environmentIntensity = 0.45;
  scene.environment = pmrem.fromScene(new RoomEnvironment(), 0.04).texture;
  pmrem.dispose();
  scene.add(new T.HemisphereLight(0xb9d9eb, 0x87866b, 0.8));
  const sun = new T.DirectionalLight(0xffdda4, 2.3);
  sun.position.set(-6, 10, 6);
  sun.castShadow = true;
  sun.shadow.mapSize.set(1024, 1024);
  sun.shadow.bias = -0.00025;
  sun.shadow.normalBias = 0.035;
  scene.add(sun);
  scene.add(sun.target);
  const csm = new CSMShadowNode(sun, {
    cascades: 3,
    maxFar: 30,
    mode: "practical",
  });
  sun.shadow.shadowNode = csm;
  const rim = new T.DirectionalLight(0x9cc9eb, 1.5);
  rim.position.set(4, 4, -5);
  scene.add(rim);
  const arena = environment(scene);
  // Instrument actual submitted objects. Nested shadow renders identify their camera.
  const counts = { scene: 0, shadow: 0, screen: 0 };
  scene.traverse((obj) => {
    if ((obj as T.Mesh).isMesh) {
      obj.onBeforeRender = (_r, _s, cam) => {
        if (cam === camera) counts.scene++;
      };
      obj.onBeforeShadow = () => {
        counts.shadow++;
      };
    }
  });
  $("progress").setAttribute("value", "35");
  const url = import.meta.env.BASE_URL + "assets/moonfang.glb";
  const gltf = await new GLTFLoader().loadAsync(url, (p) => {
    if (p.total)
      $("progress").setAttribute(
        "value",
        String(35 + (35 * p.loaded) / p.total),
      );
  });
  const character = gltf.scene;
  scene.add(character);
  // Pack the source's distinct PBR surfaces into two draw groups, preserving
  // skin weights and per-vertex color/roughness/metalness. No material-per-part cost.
  const sources: T.SkinnedMesh[] = [];
  character.traverse((o) => {
    if ((o as T.SkinnedMesh).isSkinnedMesh) sources.push(o as T.SkinnedMesh);
  });
  if (!sources.length) throw Error("The required skinned character is missing");
  const first = sources[0],
    groups: T.BufferGeometry[][] = [[], []];
  for (const mesh of sources) {
    const geometry = mesh.geometry.clone();
    const mat = mesh.material as T.MeshStandardMaterial;
    const n = geometry.attributes.position.count;
    const colors = new Float32Array(n * 3),
      rough = new Float32Array(n),
      metal = new Float32Array(n);
    for (let i = 0; i < n; i++) {
      mat.color.toArray(colors, i * 3);
      const tint = /hide|fur|mane|roots/.test(mat.name) ? 0.4 : 1;
      colors[i * 3] *= tint;
      colors[i * 3 + 1] *= tint;
      colors[i * 3 + 2] *= tint;
      const ao = geometry.getAttribute("_ao")?.getX(i) ?? 1;
      colors[i * 3] *= ao;
      colors[i * 3 + 1] *= ao;
      colors[i * 3 + 2] *= ao;
      rough[i] = mat.roughness;
      metal[i] = mat.metalness;
    }
    geometry.setAttribute("color", new T.BufferAttribute(colors, 3));
    geometry.setAttribute("surfaceRoughness", new T.BufferAttribute(rough, 1));
    geometry.setAttribute("surfaceMetalness", new T.BufferAttribute(metal, 1));
    // Unused UV sets vary between source primitives; remove before batching.
    for (const name of Object.keys(geometry.attributes))
      if (
        ![
          "position",
          "normal",
          "skinIndex",
          "skinWeight",
          "color",
          "surfaceRoughness",
          "surfaceMetalness",
        ].includes(name)
      )
        geometry.deleteAttribute(name);
    groups[mat.metalness > 0.5 ? 1 : 0].push(geometry);
    mesh.visible = false;
  }
  const mergedGroups = groups
    .filter((a) => a.length)
    .map((a) => mergeGeometries(a)!);
  const merged = mergeGeometries(mergedGroups, true);
  if (!merged) throw Error("Character batching failed");
  const materials = mergedGroups.map(() => {
    const m = new T.MeshStandardNodeMaterial({ vertexColors: true });
    m.roughnessNode = attribute("surfaceRoughness", "float");
    m.metalnessNode = attribute("surfaceMetalness", "float");
    return m;
  });
  const hero = new T.SkinnedMesh(merged, materials);
  hero.name = "Moonfang • two draw groups";
  hero.bind(first.skeleton, first.bindMatrix);
  hero.castShadow = true;
  hero.receiveShadow = true;
  hero.onBeforeRender = (_r, _s, cam) => {
    if (cam === camera) counts.scene++;
  };
  hero.onBeforeShadow = () => {
    counts.shadow++;
  };
  first.parent!.add(hero);
  const cloth = secondary(character, first.skeleton);
  const mixer = new T.AnimationMixer(character),
    actions = new Map<string, T.AnimationAction>();
  const labels = ["Idle", "Run", "Cast", "Attack", "Hit", "Death"];
  for (const clip of gltf.animations) {
    const label =
      labels.find((n) => clip.name.toLowerCase().includes(n.toLowerCase())) ||
      clip.name;
    actions.set(label, mixer.clipAction(clip));
  }
  if (!actions.has("Idle")) throw Error("Idle animation is missing");
  let active: T.AnimationAction | undefined,
    current = "Idle",
    elapsed = 0,
    frame = 0,
    quality = "balanced",
    face = false,
    hidden = false,
    freeze = query.has("still");
  function animate(name: string) {
    const action = actions.get(name);
    if (!action) return;
    active?.fadeOut(0.18);
    action
      .reset()
      .setLoop(
        name === "Idle" || name === "Run" ? T.LoopRepeat : T.LoopOnce,
        Infinity,
      );
    action.clampWhenFinished = true;
    action.fadeIn(0.18).play();
    active = action;
    current = name;
    $("clips")
      .querySelectorAll("button")
      .forEach((b) => b.classList.toggle("active", b.dataset.clip === name));
  }
  for (const [i, name] of labels.entries()) {
    const button = document.createElement("button");
    button.dataset.clip = name;
    button.innerHTML = `${name === "Cast" ? "Lunar howl" : name === "Attack" ? "Rending strike" : name === "Idle" ? "Guardian idle" : name === "Run" ? "Predatory run" : name === "Hit" ? "Hit reaction" : "Fallen guardian"} <span>0${i + 1}</span>`;
    button.onclick = () => animate(name);
    $("clips").append(button);
  }
  animate("Idle");
  mixer.addEventListener("finished", () => {
    if (current !== "Death") animate("Idle");
  });
  const glow = new T.Mesh(
    new T.TorusGeometry(1.4, 0.025, 6, 64),
    new T.MeshStandardMaterial({
      color: 0x7cceec,
      emissive: 0x55bbff,
      emissiveIntensity: 3,
    }),
  );
  glow.rotation.x = -Math.PI / 2;
  glow.position.y = 0.18;
  scene.add(glow);
  const pp = new T.PostProcessing(renderer);
  const scenePass = pass(scene, camera);
  const glowScene = new T.Scene();
  glowScene.background = new T.Color(0);
  const glowFire = arena.fire.clone();
  glowFire.instanceMatrix = arena.fire.instanceMatrix;
  glowScene.add(glowFire);
  const glowRing = glow.clone();
  glowScene.add(glowRing);
  const glowPass = pass(glowScene, camera);
  const color = scenePass.getTextureNode("output");
  const shine = bloom(glowPass.getTextureNode("output"), 0.35, 0.35, 0);
  pp.outputNode = fxaa(color.add(shine));
  const keys = new Set<string>();
  addEventListener("keydown", (e) => {
    if ((e.target as HTMLElement).matches("select,input,button")) return;
    keys.add(e.code);
    if (e.code === "Space") {
      e.preventDefault();
      animate("Attack");
    }
    if (e.code === "KeyR") animate("Cast");
    if (e.code === "KeyH") toggleHud();
  });
  addEventListener("keyup", (e) => keys.delete(e.code));
  addEventListener("blur", () => keys.clear());
  function toggleHud() {
    hidden = !hidden;
    $("app").classList.toggle("hide", hidden);
    $("show").hidden = !hidden;
  }
  $("hud").onclick = toggleHud;
  $("show").onclick = toggleHud;
  $("camera").onclick = () => {
    face = !face;
    camera.position.set(face ? 1.4 : 5.7, face ? 3.35 : 3.5, face ? 2.4 : 8.4);
    controls.target.set(0, face ? 3.15 : 1.9, 0);
    $("camera").textContent = face
      ? "↙ View full character"
      : "↗ Inspect the face";
  };
  $("export").onclick = () => {
    const a = document.createElement("a");
    a.href = url;
    a.download = "moonfang.glb";
    a.click();
  };
  document.querySelectorAll<HTMLButtonElement>("[data-light]").forEach(
    (b) =>
      (b.onclick = () => {
        document
          .querySelectorAll("[data-light]")
          .forEach((x) => x.classList.toggle("active", x === b));
        const night = b.dataset.light === "moon",
          studio = b.dataset.light === "studio";
        sun.color.set(night ? 0x93b5ff : 0xffdda4);
        sun.intensity = night ? 1.4 : studio ? 2.5 : 3.3;
        rim.intensity = night ? 3 : 2;
        scene.background = new T.Color(
          night ? 0x283e55 : studio ? 0x9ba7ad : 0x9eadae,
        );
      }),
  );
  $<HTMLSelectElement>("quality").onchange = (e) => {
    quality = (e.target as HTMLSelectElement).value;
    renderer.setPixelRatio(
      quality === "low"
        ? 1
        : quality === "ultra"
          ? Math.min(devicePixelRatio, 2)
          : Math.min(devicePixelRatio, 1.5),
    );
    renderer.setSize(innerWidth, innerHeight);
  };
  function resize() {
    camera.aspect = innerWidth / innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(innerWidth, innerHeight);
  }
  addEventListener("resize", resize);
  canvas.addEventListener("webglcontextlost", (e) => {
    e.preventDefault();
    error("Graphics context lost. Reload to restore the scene.");
  });
  $("load-note").textContent =
    "Compiling skin, metal, shadow cascades and emissive-only bloom…";
  $("progress").setAttribute("value", "85");
  await renderer.compileAsync(scene, camera);
  controls.update();
  if (
    (renderer.backend as unknown as { isWebGPUBackend?: boolean })
      .isWebGPUBackend
  ) {
    if (!query.has("direct")) {
      await pp.renderAsync();
      await pp.renderAsync();
    }
  } else {
    renderer.render(scene, camera);
    renderer.render(scene, camera);
  }
  $("loading").style.display = "none";
  canvas.dataset.ready = "true";
  Object.assign(window, {
    lab: {
      scene,
      camera,
      renderer,
      character,
      pp,
      hero,
      controls,
      mixer,
      actions,
    },
  });
  scene.traverse((o) => {
    if ((o as T.Mesh).isMesh) {
      o.onBeforeRender = (_r, _s, c) => {
        if (c === camera) counts.scene++;
      };
      o.onBeforeShadow = () => {
        counts.shadow++;
      };
    }
  });
  const clock = new T.Clock();
  let animationTime = 0;
  let sample = 0,
    cpuSum = 0,
    fpsSum = 0,
    gpuMs: number | undefined,
    pending = false;
  const offset = new T.Vector3();
  renderer.setAnimationLoop(() => {
    if (document.hidden) {
      clock.getDelta();
      return;
    }
    if (deviceLost) return;
    const begin = performance.now(),
      rawDt = clock.getDelta(),
      dt = Math.min(rawDt, 0.05);
    elapsed += dt;
    frame++;
    counts.scene = counts.shadow = 0;
    renderer.info.reset();
    const cascadeLights = (
      csm as unknown as { lights: Array<{ shadow: T.LightShadow }> }
    ).lights;
    for (let i = 0; i < cascadeLights.length; i++) {
      const sh = cascadeLights[i].shadow;
      sh.autoUpdate = false;
      sh.needsUpdate = i === 0 || frame % 4 === 0;
    }
    let moving = false;
    offset.set(
      (keys.has("KeyD") ? 1 : 0) - (keys.has("KeyA") ? 1 : 0),
      0,
      (keys.has("KeyS") ? 1 : 0) - (keys.has("KeyW") ? 1 : 0),
    );
    if (offset.lengthSq()) {
      offset.normalize().multiplyScalar(dt * 2);
      character.position.add(offset);
      character.position.x = T.MathUtils.clamp(character.position.x, -4, 4);
      character.position.z = T.MathUtils.clamp(character.position.z, -4, 4);
      moving = true;
    }
    if (moving && current === "Idle") animate("Run");
    if (
      !moving &&
      current === "Run" &&
      !document.activeElement?.hasAttribute("data-clip")
    )
      animate("Idle");
    animationTime += dt;
    const cadence =
      camera.position.distanceTo(character.position) > 12 ? 1 / 24 : 0;
    if (!freeze && animationTime >= cadence) {
      mixer.update(animationTime);
      animationTime = 0;
    }
    cloth.update(freeze ? 1 / 60 : dt, freeze ? 2 : elapsed);
    arena.update(freeze ? 2 : elapsed);
    glow.scale.setScalar(
      current === "Cast" ? 1 + 0.08 * Math.sin(elapsed * 10) : 0.001,
    );
    glowRing.scale.copy(glow.scale);
    controls.update();
    if (
      !(renderer.backend as unknown as { isWebGPUBackend?: boolean })
        .isWebGPUBackend ||
      query.has("direct")
    )
      renderer.render(scene, camera);
    else pp.render();
    canvas.dataset.frame = String(frame);
    const cpu = performance.now() - begin;
    cpuSum += cpu;
    fpsSum += rawDt;
    sample++;
    if (fpsSum >= 0.5) {
      $("fps").textContent = String(
        Math.round(sample / Math.max(fpsSum, 0.001)),
      );
      $("meter").style.width =
        Math.min(100, (cpuSum / sample / 16.7) * 100) + "%";
      $("timing").textContent =
        `CPU submit ${(cpuSum / sample).toFixed(1)} ms · GPU ${gpuMs === undefined ? "unavailable" : gpuMs.toFixed(1) + " ms"}`;
      $("draws").textContent =
        `SCENE ${counts.scene} · SHADOW ${counts.shadow} · SCREEN ${Math.max(0, renderer.info.render.drawCalls - counts.scene - counts.shadow)}`;
      $("mesh").textContent =
        `${Math.round(renderer.info.render.triangles / 1000)}K triangles / all passes · 2 hero groups`;
      cpuSum = fpsSum = sample = 0;
      if (!pending && renderer.hasFeature("timestamp-query")) {
        pending = true;
        void renderer
          .resolveTimestampsAsync("render")
          .then(() => {
            gpuMs = renderer.info.render.timestamp || undefined;
          })
          .catch(() => {
            gpuMs = undefined;
          })
          .finally(() => (pending = false));
      }
    }
  });
}
void start().catch(error);
