# Validation and implementation boundaries

## Executed in this cloud

`Scripts/test-core.sh`: **139 assertions passed across 10 scenarios**, with `-Wall -Wextra -Wpedantic -Werror`, AddressSanitizer and UndefinedBehaviorSanitizer. Coverage includes:

- Two-player readiness, full server, unit prices, insufficient funds, invalid roster IDs, and 20-unit cap.
- Ownership validation, invalid/non-finite destinations, unknown commands, hidden-target rejection, queue limit, movement, and stop.
- Visibility-filtered snapshots, historical enemy positions that do not follow hidden movement, and contact expiry.
- Objective captures, majority ticket bleed, timeout victory, ties, and contested capture.
- Disconnect grace, successful reconnection, expiry victory, and expired-session rejection.
- Directional armor, damage, ammunition use, smoke line-of-sight blocking and expiry, and resupply.

## Not executed

Unreal Header Tool or C++ module compilation; asset-generation script in Unreal; Windows packaging; two-client dedicated-server sessions; network packet-loss testing; visual inspection; GPU/frame-time benchmarks; UI checks at 1080p/1440p. Those checks are prerequisites for calling this a playable slice.

## Known gaps against the requested plan

- The battlefield, infantry, and vehicles use original primitive geometry. Licensed production models, squad member animation, suspension, recoil, impacts, destruction, and audio are not integrated.
- Simulation smoke blocks visibility, but volumetric smoke presentation is not implemented.
- Unit statistics currently live in the engine-independent roster. A presentation data-asset type exists, but data-driven rules and presentation-asset consumption are unfinished.
- Movement uses direct steering with obstacle sidestepping. Unreal navigation, robust route planning, local unit avoidance, and unreachable-route reporting are not implemented. Blocked destinations are rejected.
- River/bridge scenery is illustrative; it does not constrain navigation. Circular simulation obstacles approximate the rendered buildings.
- No production visual benchmark or polished authored map exists yet. Unit snapshot presentation currently snaps at 10 Hz; interpolation remains necessary.
- HUD has selection, controls, minimap, and details, but path/order markers, in-game key remapping, audio sliders, and full accessibility/settings UI remain unfinished. Standard UMG DPI scaling is used.
- Reconnect tokens survive network disconnects in the running client, not application restarts. Tokens are not persistent accounts or production-grade authentication. Public service hardening is out of scope.
- Windows build/asset scripts are unexecuted instructions and may require corrections after the first real Unreal build.

## Required follow-up acceptance

1. Unblock engine access/storage, compile all three targets, and correct UHT/compiler errors before claiming source integration works.
2. Run asset generation and inspect normal-height camera readability, selection, minimap coordinate mapping, and widgets at both target resolutions.
3. Run two packaged clients against the server. Test ready/start, purchase/commands, match results, rematch, disconnect/reconnect, full server, bad address, and server shutdown.
4. Capture network traffic or instrument replication to confirm hidden units/commands never reach opponents. Test `Net PktLag=50` and `Net PktLoss=1` on controlled test connections, then repeat on real 100 ms round-trip links.
5. Replace prototype art and complete the above gameplay/UI gaps. Test 40 units plus smoke and combat effects using Unreal Insights and `stat unit`/`stat gpu` on the reference Windows PC. Record resolution, settings, hardware, frame-time distribution, and test build.
6. Accept 1440p/60 FPS only after measurements meet the target. Simulation test success does not establish graphical or multiplayer readiness.
