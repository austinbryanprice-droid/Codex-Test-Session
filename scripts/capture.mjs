import { chromium } from "@playwright/test";
import { mkdir, writeFile } from "node:fs/promises";
const dir = `reference/capture-${new Date().toISOString().replaceAll(":", "-")}`;
await mkdir(dir, { recursive: true });
const browser = await chromium.launch({
  executablePath:
    process.env.CHROMIUM_PATH ||
    (process.platform === "linux" ? "/usr/bin/chromium" : undefined),
  args: [
    "--no-sandbox",
    "--use-gl=angle",
    "--use-angle=swiftshader",
    "--enable-unsafe-swiftshader",
  ],
});
try {
  const page = await browser.newPage({
    viewport: { width: 1440, height: 1000 },
  });
  const errors = [];
  page.on("pageerror", (e) => errors.push(e.message));
  await page.goto(
    process.env.LAB_URL || "http://127.0.0.1:5180/?webgl=1&still=1",
  );
  await page.locator("canvas[data-ready=true]").waitFor({ timeout: 180000 });
  await page.waitForFunction(
    () => Number(document.querySelector("canvas").dataset.frame) > 3,
  );
  await page.screenshot({ path: `${dir}/full.png`, timeout: 120000 });
  await page.getByRole("button", { name: /Inspect the face/ }).click();
  await page.waitForTimeout(1500);
  await page.screenshot({ path: `${dir}/face.png`, timeout: 120000 });
  await writeFile(`${dir}/errors.json`, JSON.stringify(errors));
  if (errors.length) throw Error(JSON.stringify(errors));
  console.log(dir);
} finally {
  await browser.close();
}
