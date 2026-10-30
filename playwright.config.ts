import { defineConfig } from "@playwright/test";
export default defineConfig({
  testDir: "tests/browser",
  timeout: 180000,
  workers: 1,
  use: {
    baseURL: "http://127.0.0.1:5180",
    viewport: { width: 1280, height: 800 },
    launchOptions: {
      executablePath:
        process.env.CHROMIUM_PATH ||
        (process.platform === "linux" ? "/usr/bin/chromium" : undefined),
      args: [
        "--no-sandbox",
        "--use-gl=angle",
        "--use-angle=swiftshader",
        "--enable-unsafe-swiftshader",
      ],
    },
    trace: "retain-on-failure",
  },
  webServer: {
    command: "npm run dev -- --port 5180",
    url: "http://127.0.0.1:5180",
    reuseExistingServer: true,
  },
});
