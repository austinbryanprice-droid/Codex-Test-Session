import { test, expect } from "@playwright/test";
test("renders a skinned character, six distinct clips, controls and export", async ({
  page,
}) => {
  const errors: string[] = [];
  page.on("pageerror", (e) => errors.push(e.message));
  await page.goto("/?webgl=1&still=1");
  const canvas = page.locator("canvas[data-ready=true]");
  await expect(canvas).toBeVisible({ timeout: 120000 });
  const stats = await page.evaluate(() => {
    const l = (window as any).lab;
    return {
      groups: l.hero.geometry.groups.length,
      surfaces: l.hero.material.length,
      clips: [...l.actions.keys()],
      triangles: l.hero.geometry.index.count / 3,
      vertexBuffers: new Set(
        Object.values(l.hero.geometry.attributes).map((a: any) =>
          a.isInterleavedBufferAttribute ? a.data : a,
        ),
      ).size,
    };
  });
  expect(stats.groups).toBe(2);
  expect(stats.vertexBuffers).toBeLessThanOrEqual(8);
  expect(stats.surfaces).toBe(2);
  expect(stats.triangles).toBeLessThan(150000);
  expect(stats.clips).toHaveLength(6);
  for (const label of [
    "Predatory run",
    "Lunar howl",
    "Rending strike",
    "Hit reaction",
    "Fallen guardian",
    "Guardian idle",
  ]) {
    await page.getByRole("button", { name: new RegExp(label) }).click();
    const result = await page.evaluate(() => {
      const l = (window as any).lab;
      const before = l.hero.skeleton.bones
        .map((b: any) => b.quaternion.toArray())
        .flat()
        .join(",");
      l.mixer.update(0.3);
      const after = l.hero.skeleton.bones
        .map((b: any) => b.quaternion.toArray())
        .flat()
        .join(",");
      return before !== after;
    });
    expect(result).toBe(true);
  }
  await page.getByRole("button", { name: /Inspect the face/ }).click();
  await expect(
    page.getByRole("button", { name: /View full character/ }),
  ).toBeVisible();
  await page.getByRole("button", { name: "Moonlit", exact: true }).click();
  await expect(
    page.getByRole("button", { name: "Moonlit", exact: true }),
  ).toHaveClass(/active/);
  await page.locator("#quality").selectOption("low");
  await canvas.focus();
  await page.keyboard.press("KeyH");
  await expect(page.locator("aside")).toBeHidden();
  await page.getByRole("button", { name: /Show interface/ }).click();
  const [download] = await Promise.all([
    page.waitForEvent("download"),
    page.getByRole("button", { name: /Export character/ }).click(),
  ]);
  expect(download.suggestedFilename()).toBe("moonfang.glb");
  expect(errors).toEqual([]);
  await expect(page.getByRole("alert")).toBeHidden();
});
test("missing required character shows an explicit error", async ({ page }) => {
  await page.route("**/moonfang.glb*", (route) =>
    route.fulfill({ status: 404, body: "missing" }),
  );
  await page.goto("/?webgl=1");
  await expect(page.getByRole("alert")).toContainText("could not start", {
    timeout: 60000,
  });
  await expect(page.locator("canvas[data-ready=true]")).toHaveCount(0);
});
