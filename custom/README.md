# CuckooTilt customisation

This folder holds the manufacturer (CuckooTilt) specific code, firmware and UI.
It lives on the `custom` branch, based on `dev`, so that the manufacturer's
customisation is kept apart from the standard GravityMon code and `dev` can be
merged into it with as few conflicts as possible. `dev` itself has no
manufacturer specific code.

**Status: experimental.** The firmware and the UI both build. The firmware has not
been run on a device, and the UI has only been run against the mock server. The
firmware embeds the custom UI (see [UI](#ui)), also not tried on a device.

## Quick start

Firmware (output in `bin/`). The UI is built first, from the files in
`custom/html/` (they are committed, like `html/` is):

```bash
(cd custom/ui && npm run build:firmware)   # only when the UI changed
pio run -e cuckoo_32c3
```

UI (first time: [set up the UI](#set-up-the-ui)), then in two terminals in `custom/ui`:

```bash
cd custom/ui
npm run mock    # mock device on http://localhost:3000
npm run dev     # UI dev server, normally http://localhost:5173
```

## Firmware

### Build

```bash
pio run -e cuckoo_32c3
```

| File in `bin/` | Content |
|---|---|
| `cuckoo_32c3.bin` | Firmware |
| `cuckoo_32c3_partitions.bin` | Partition table |

The standard `gravity-32c3_cuckoo` target is unchanged and still writes
`firmware32c3cuckoo.bin`, so the two builds cannot be mixed up. The local file
name is only for the build output. For OTA updates the device asks the
manufacturer's server for `firmware32c3.bin`, which is the name the server uses.

The firmware embeds the custom UI from `custom/html/` (`index.html`,
`app.js.gz`, `app.css.gz`, `favicon.ico.gz`), not the standard UI from `html/`,
which stays identical to GravityMon's (CI commits it on the upstream branches).
`npm run build:firmware` in `custom/ui` builds the UI and copies the two
generated files there; `index.html` and the icon are static. The framework looks
the files up as `_binary_html_*`, the names PlatformIO gives to files in `html/`;
files in `custom/html/` get `_binary_custom_html_*` names, so `[env:cuckoo_32c3]`
aliases them with linker `--defsym` flags. Built size: flash 88.4% (the standard
UI build is 93.5%).

### How it works

| Where | What |
|---|---|
| `platformio.ini` | `[env:cuckoo_32c3]` extends `gravity-32c3_cuckoo`, swaps the main file (`build_src_filter = +<*> -<main_gravitymon.cpp> +<../custom/main_cuckoo.cpp>`), sets the espframework overrides and embeds `custom/html/`. |
| `custom/.gitignore` | Ignores `ui/node_modules` and `ui/dist`, so the repository `.gitignore` is not changed |
| `custom/copy_firmware.py` | Own post build step for `cuckoo_32c3` (set with `extra_scripts` in the env): writes `bin/cuckoo_32c3.bin` and `bin/cuckoo_32c3_partitions.bin`. `script/copy_firmware.py` and `script/create_versionjson.py` are not used for this target, so `bin/version.json` is not touched |
| `custom/main_cuckoo.cpp` | Copy of `src/main_gravitymon.cpp` with the manufacturer's changes, each marked `CUCKOO:` |
| `custom/config_cuckoo.hpp` | `CuckooConfig`, a subclass of `GravitymonConfig` |
| `ui/` | Nothing changed here. The custom UI needs one generic change that is in `dev`: `config.registerLoadCallback()`, see [UI](#ui) |

The folder is called `custom/` because `cuckoo/` is already in `.gitignore`.
The main file is outside `src/` so that the other targets do not compile two
`setup()`/`loop()` functions. No other target is affected.

No firmware source file in `src/` is changed. The rest of the firmware only sees
the config through pointers and interfaces, so a `CuckooConfig` can be used where
a `GravitymonConfig` is expected.

### What is customised

#### Behaviour (`main_cuckoo.cpp`)

Tunables are at the top of the file.

| Change | Tunable |
|---|---|
| AP name `CuckooTilt`, no password | |
| Device name prefix `cuckootilt` (mDNS name, `name` in the payload, e.g. `cuckootilt19a54c`) | `kAppName` |
| Lying flat (85-95 degrees) does not enter config mode, config mode is only entered from the battery voltage | |
| Storage mode below 15 degrees (standard is 5) | `kStorageAngleDeg` |
| Storage mode wakes up after 1 hour instead of sleeping indefinitely | `kStorageWakeSec` |
| Restart after 5 minutes in config / wifi setup mode | `kConfigInactivityRestartMs` |
| Sleep 300 s (standard 60 s) when wifi is missing | `kNoWifiRetrySleepSec` |
| Battery saving (1 hour sleep at low battery) is disabled | |
| Charging pin enters wifi mode instead of storage mode | Only active on boards that define `PIN_CHARGING`, which `CUCKOO_C3` does not |
| Bluetooth only sends when the Bluetooth switch is on | |
| OTA firmware file `firmware32c3.bin` | |

#### Config (`config_cuckoo.hpp`)

**Defaults.** Set in the constructor, i.e. before the config file is loaded, so
stored values and user changes still win. A factory reset removes the file and
the defaults apply again.

| Setting | Default | Standard |
|---|---|---|
| Storage sleep | on | off |
| Battery saving | off | on |
| Voltage config | 4.3 | 4.15 |
| Temperature sensor resolution | 11 bit | 9 bit |
| Gravity formula | preset polynomial | empty |

**Extra settings.** Saved in the config file and exposed in the config API.

| Key | Default |
|---|---|
| `http_post_sleep_interval` | 900 s |
| `ble_sleep_interval` | 120 s |
| `use_http_post` | true |
| `use_ble` | false |

**Manufacturer server.** Defaults taken from the manufacturer's own firmware
binary (found by looking at its strings): OTA URL
`http://api.ckbrew.com/rest/public/`, HTTP post target
`http://api.ckbrew.com/rest/public/deviceData` with header
`Content-Type: application/json`. Like the other defaults, stored values win.

`isWifiPushActive()` is replaced so that HTTP post targets only count when
`use_http_post` is on.

**Default payload.** `writeDefaultTemplates()` stores the manufacturer's JSON
format for the two HTTP post targets. It adds `http-post-sleep-interval`,
`ble-sleep-interval`, `ip-address`, `app-ver` and `run-mode` to the standard
iSpindle compatible format. By default it is only stored when the file is
missing (first boot / after a factory reset), so a format saved by the user is
kept. Set `kForceDefaultTemplates = true` in `main_cuckoo.cpp` to replace it on
every boot.

## UI

`custom/ui/` is a separate, smaller UI for the manufacturer's simplified
"WIFI mode" experience (in that mode the device has no sensor readings, so the
manufacturer's UI only offers a few pages from a home page with cards). It reuses
the stores, the component library, the build config and the standard translations
from `../ui`. The shell, the router, the i18n setup and all the pages are its own.

### Set up the UI

Needed once. Run from the repository root.

```bash
(cd ui && npm ci)                                  # install the dependencies
ln -s ../../ui/node_modules custom/ui/node_modules # custom/ui uses the same install
```

and create `ui/.env.development` (it is git-ignored) with the address of the
device the UI should talk to. The mock server from `npm run mock` is on port 3000:

```
VITE_APP_HOST=http://localhost:3000/
```

`custom/ui/node_modules` and `custom/ui/dist` are ignored by `custom/.gitignore`.

### Run and build

Run from `custom/ui`:

| Command | What |
|---|---|
| `npm run mock` | Mock device on `http://localhost:3000`. It is `mock-server.js`, the standard mock server (`ui/mock-server`) with the CuckooTilt values seeded first: the four extra settings, the manufacturer's server in HTTP Post 1, an empty HTTP Post 2, the other `CuckooConfig` defaults, hardware `cuckoo` and WIFI mode. Saved changes are kept in memory until the mock is restarted. The standard mock is not changed |
| `npm run dev` | UI dev server, normally `http://localhost:5173` (it prints the address). It reads `ui/.env.development`, the same file as the standard UI. To try it against a real device, set `VITE_APP_HOST` to the device address and do not start the mock |
| `npm run build` | Production build in `custom/ui/dist` |
| `npm run build:firmware` | The same build, and copies `app.js.gz` and `app.css.gz` to `custom/html/` for the firmware |

### What is in it

| What | Where |
|---|---|
| Shell (`App.vue`) and the pages, ported from the manufacturer's UI so the look and layout are the same: home with the eight cards (WIFI, HTTP Post, Bluetooth, Gravity Formula, Firmware, Battery, Device ID with Copy ID, System Tools), WiFi (with the collapsed WIFI Direct block, Copy ID, Restart), HTTP Post, Bluetooth, Gravity Formula, Battery, System tools (firmware update, About, restore defaults), About | `src/App.vue`, `src/views/`, `src/components/CuckooCard.vue` |
| Text on the ported pages: the standard translations are reused where the text is the same (`device_wifi`, `push_common`, `push_bluetooth`, `push_wifi_direct`, `gravity_formula`, `device_hardware`), the rest is under `cuckoo.*` | `src/i18n.js`, `src/locales/` |
| Copy ID (works over plain http) and the "(14 min 50 s)" interval label | `src/utils.js` |
| Top bar without a menu (brand, device name, "Save needed", dark mode). The library menu bar always shows the menu button, also with no menu items, and its "Save needed" text is not translatable | `src/components/CuckooNavBar.vue` |
| "Back to main" button on every page except the home page | `src/App.vue` |
| Settings that only exist in `CuckooConfig` (`use_http_post`, `http_post_sleep_interval`, `use_ble`, `ble_sleep_interval`) | `src/cuckooConfig.js` (registered with `config.registerLoadCallback()`, one request, see below) |
| Embedded languages: en, fr, es, de, zh | `src/i18n.js`, `src/locales/` |
| UI feature flags (`global.ui.*`): only `enableGravity` and `enableScanForStrongestAp` are on, all others are off | `src/main.js`, set before the app starts |
| Firmware upload page, a copy of the standard page with the platform, version, hardware and file name line always shown (as on the manufacturer's page). The text is the standard `firmware` translations | `src/views/FirmwareView.vue` |
| Footer "CuckooTilt · by Cuckoo (ckbrew.com) · Based on GravityMon © Magnus Persson" on every page, and an About page with the MIT license text and "Copyright (c) 2021-2026 Magnus Persson" (the license requires these notices to stay in the About screen). The license text itself stays English in every language. The wording about the commercial license is the manufacturer's own and should be confirmed | `src/App.vue`, `src/views/AboutView.vue` |

**Differences from the manufacturer's UI.** Their UI is a self-contained copy
(own components, own stores, English only). This one keeps their pages and
layout but uses the shared stores and the component library, and adds
the languages and the language selector. Not copied: the CuckooTilt Bluetooth
format (value 8), which only exists in their UI branch `feat/20260909-fixes` and
is not in the firmware, and the "voltage factor calculator under tools" hint on
the battery page (this UI has no tools page). Their menu bar is not copied either:
in WIFI mode it only shows the brand, "Save needed", the spinner and the dark
mode switch, which is what `CuckooNavBar.vue` does. Their full menu only shows
when `status.wifi_setup` is false, which this UI never offers.

**`registerLoadCallback`.** A second generic change in the standard UI, also in
`dev`: `config.registerLoadCallback(fn)` in `ui/src/modules/configStore.js` (plus a
unit test). `config.load()` calls `fn(json, store)` with the full `/api/config`
response. `src/cuckooConfig.js` uses it to `$patch` the four `CuckooConfig`
settings into the config store state, so they load with the same request as the
standard settings and are saved by `config.saveAll()` in the same POST (the
change detection compares the whole state). A store only exposes the state keys
it declared, so the pages reach the added settings as `cuckoo.use_http_post` etc.,
where `cuckoo` (from `src/cuckooConfig.js`) is `config.$state`.

**Why a custom shell?** The library's input components look up `BsInputBase` by
name, so it must be registered even though no page uses it (`src/main.js`).
Without it every input loses its label, help text and spacing.

**Language change in the production build.** The standard build config sets
`rolldownOptions.treeshake.propertyReadSideEffects: false`, which drops property
reads whose result is not used. vue-i18n follows the active language by reading
its locale refs in exactly that way, so in the production build only the
components that read `config.locale` themselves (the top bar and the language
list in `App.vue`) were redrawn when the language was changed, and the pages kept
the old language until they were opened again. Everything worked in the dev
server, where nothing is dropped. `vite.config.js` sets the option back to `true`
for this UI (+2 KB script, about 0.6 KB gzipped). Found and checked with headless
Chrome on a production build against the mock (switch language on the WiFi page,
on the home page, and on the system tools and firmware pages). The standard UI
uses the same setting, so it may have the same problem when its language is
changed on a page that does not read `config.locale`; not checked.

### Size

Measured size (gzipped, what is embedded in the firmware):

| | Standard UI | CuckooTilt UI |
|---|---|---|
| Script | 208 KB | 114 KB |
| Stylesheet | 19.7 KB | 19.3 KB |
| Languages | English only (others are optional packs) | included above, all five |

The size grows with every page that is added.

### HTTP Post 1 and 2

The manufacturer's simplified HTTP Post page edits **HTTP Post 2**
(`http_post2_target` and its format), while their firmware has the
manufacturer's server as the default of **HTTP Post 1**, which that page never
shows. The working assumption (not confirmed by the manufacturer) is:

- **Post 1** is the manufacturer's own server. It is set by default and is not
  shown in the simplified UI.
- **Post 2** is an optional server of the user's own, edited on the HTTP Post
  page. It is empty by default in the manufacturer's default configuration.

The UI follows this and edits Post 2 (`config.http_post2_target`). The
`use_http_post` switch then decides whether wifi is used for HTTP pushing at
all: in `CuckooConfig::isWifiPushActive()` it governs both post targets, but
it does not stop an already connected device from sending to them, because the
push code sends to every configured target.

## Not done / known gaps

**UI**

- `custom/html/app.js.gz` and `app.css.gz` are build output that is committed, so
  they must be rebuilt (`npm run build:firmware`) after every UI change and after
  a merge that touches `ui/`. The CI of the repository does not do that for this branch.
  The firmware build with the embedded UI links, but the UI served by the device
  has not been looked at.
- The WIFI-mode pages of the manufacturer's UI are all ported. Not ported: the
  voltage factor calculator (tools) and the full configuration-mode pages
  (settings, hardware, gyro, push types), which their UI only shows when
  `status.wifi_setup` is false.
- The Bluetooth page lists no CuckooTilt format (value 8): it is only in the
  manufacturer's UI branch, the firmware has no such format. To confirm.
- Only checked in a browser against the mock (headless screenshots of the home,
  WiFi, HTTP Post, Bluetooth, formula, battery, system tools and About pages), the
  forms were not saved or tested for validation, and the push test, restore and
  firmware upload only exist as in the shared code.
- `sleep_interval` and the skip interval are calculated when the HTTP Post or
  Bluetooth page is saved (`calculateIntervals()` in `src/cuckooConfig.js`, same
  rules as the manufacturer's UI): both off 3600 s, one on its own interval, both
  on the shorter interval and `http_post_int` = http interval / sleep interval - 1.
  The result was only checked by reading the code. The interval is only
  recalculated by those two pages, and not when the device is changed some other
  way (for example through the API).
- Not tried on a device.
- Test push from the UI leaves `${http-post-sleep-interval}` and
  `${ble-sleep-interval}` unreplaced, because that code path is shared web code.
  Real pushes are correct.

**Firmware**

- **`use_http_post = false`** does not stop pushes to the HTTP post targets if
  wifi is up for another reason.
- **Needs espframework 1.2.19 or later.** The OTA version file name
  (`-D ESPFWK_OTA_VERSION_FILE=\"version32c3.json\"`) and the wifi portal
  timeout (`-D ESPFWK_WIFI_PORTAL_TIMEOUT=600`) are set with build flags in
  `[env:cuckoo_32c3]`. The overrides were added in espframework 1.2.19 (they are
  documented in its README), so the espframework line in the shared `lib_deps`
  of `platformio.ini` is `#1.2.19` (in `dev` too; note that the tag has no
  `v`, unlike `v1.2.17`). Older frameworks ignore the flags
  (update check asks for `version.json`, portal timeout 120 s). Verified by a
  build against the tag (the firmware contains `version32c3.json`), not against
  the manufacturer's server.
- **App name.** `kAppName` only covers the device name and the payload name.
  `CFG_APPNAME` is also used inside the framework (improv and the mDNS "app"
  text record) and a second `-D CFG_APPNAME` or a PlatformIO post script did not
  override it, so those still say `gravitymon`.
- **Console / serial flags.** The manufacturer's fork sets
  `ARDUINO_USB_CDC_ON_BOOT=0` and `USE_SERIAL_PINS`. They are not applied. The
  device has no serial port (the UART pads are for flashing), and the fork's
  flag name does not match the current framework flag (`ESPFWK_USE_SERIAL_PINS`).
- **Board.** The `cuckoo_c3` board is unchanged: RGB LED on GPIO8, as in the
  hardware analysis. The fork's board definition has a plain LED and no RGB, which
  is probably wrong for the hardware.
- **Not ported:** LED pin debug logging, ESP8266 sleep delays, `lib/mpu6050`
  patches (ESP8266 / MPU6050 only, not relevant for this target).

## Questions for the manufacturer

- Is the LED an RGB LED on GPIO8 on the current board (their board file says plain)?
- Should the default payload include the two interval fields? (They can be
  derived from `interval` and the skip count.)
- Which of the console / build flags are required?
- Is HTTP Post 1 the manufacturer's server and HTTP Post 2 the user's own
  optional server, and what should the HTTP Post switch control (see
  [HTTP Post 1 and 2](#http-post-1-and-2))?
- Should `lying flat` really not enter config mode? The hardware analysis
  mentions that the stock firmware blocked configuration mode. Note also that the
  reed switch is wired to EN (it resets the device), and double-reset detection
  does not work reliably on this hardware.
- The manufacturer has two firmware tracks: 13 dBm (OTA URL `.../public/`) and
  8.5 dBm (`.../public-8.5/`). **Decision: keep the reduced transmit power
  (8.5 dBm)**, because the default power gave connection problems. The standard
  framework sets it whenever `ESPFWK_REDUCE_WIFI_POWER` is defined, which this
  target does, so nothing is changed here; the manufacturer's own framework fork
  differs by defaulting to 13 dBm. The 8.5 dBm track would then be the
  matching OTA URL (currently `.../public/`, taken from the 13 dBm binary).
  Note that an OTA update downloads the manufacturer's own `firmware32c3.bin`,
  replacing this build.
- Will the server also serve `version.json`, or only `version32c3.json`? (The
  build asks for `version32c3.json`.)

## Keeping in sync with GravityMon

```bash
git switch custom && git merge dev
```

Files in `custom/` never conflict. Outside `custom/` this branch edits only
`platformio.ini` (the new `[env:cuckoo_32c3]` section at the end of the targets,
including `extra_scripts` that point to `custom/copy_firmware.py`, so no file in
`script/` is changed). Conflicts are only likely if `dev` changes the same spot. The generic UI changes
that the custom UI depends on (`config.registerLoadCallback()`) are in `dev`, not here.

Tried in a scratch clone before the branch was made (these files committed, then
`dev` merged in): the merge of the real upstream changes at the time (the
espframework pin, the typo fix and the `registerLoadCallback` commit) was clean,
including `platformio.ini`.

Two things do **not** follow automatically:

- `custom/main_cuckoo.cpp` started as a copy of the standard main file but has
  the manufacturer's own logic (run modes, timeouts, storage mode), so it is
  maintained by itself and is not updated when `src/main_gravitymon.cpp`
  changes. When `dev` changes that file, look at the change and port by hand
  what is wanted:

  ```bash
  git diff <last merged dev commit> dev -- src/main_gravitymon.cpp
  ```

  All deviations from the standard file are marked `CUCKOO:`, and
  `diff src/main_gravitymon.cpp custom/main_cuckoo.cpp` shows the whole
  difference. The copy was made from the version of the file at commit `e54bc1c`.

- `custom/ui` reuses the stores, the component library and the standard translations
  from `ui/`; all the views are its own. Changes there reach the custom UI when it is built, so rebuild it
  (`npm run build:firmware`) after a merge that touches `ui/`, check the pages and
  commit the new `custom/html/app.js.gz` and `app.css.gz`.
