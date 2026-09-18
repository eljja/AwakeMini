# Awake Mini 1.4.0-state1

## Fullscreen and media state experiment

**1.4.0-state1** is on `experiment/presentation-media-state`, separate from main's confirmed Win+B build. Run one portable EXE. Both test features start **OFF** on every launch.

Open **State tests** in settings or **right-click tray → State test settings**. Each checkbox applies **immediately** and operates independently. Minimizing the test dialog leaves enabled features running.

| Test | Actual action | Observable status |
|---|---|---|
| Fullscreen notification | Shows a separate fullscreen window on one monitor and calls `ITaskbarList2::MarkFullscreenWindow(TRUE)` | ON/error code and the QUNS value from `SHQueryUserNotificationState` |
| Media playing status | Registers a Video/Playing SMTC session for this app; no actual video or audio playback | Read-back `Enabled=1 / Playing=3`, or an error code |

**Fullscreen notification does not force Windows global Presentation Settings on.** The documented MarkFullscreenWindow effect is shell treatment of the active window. Inspect QUNS to see whether Windows reports Busy or Presentation; unchanged values are displayed as returned.

**Media state registration differs from real video/audio stream playback.** Reading this app's SMTC properties does not prove visibility in another program's session list or recognition by SSO. If Windows shows the media card, its title is 'Awake Mini - Media status test (no video/audio)'. No playback file or transport-button handler is attached.

### Comparison procedure

1. Exit the older Awake Mini and launch this EXE. The same singleton prevents concurrent instances.
2. To isolate effects, individually turn OFF and Apply the existing **keep-awake, display keep-on and screen-saver/mouse features**, and exit blackout. Leave **Pause all OFF** so experiments can run. Existing selections are preserved; the experiments do not turn them off for you.
3. Record a baseline with both experiments OFF.
4. Enable fullscreen only, observe, then exit using **Restore**, **Esc** or **Alt+F4**.
5. Enable media only, observe under the same conditions, then disable it.
6. For both together, enable media first, then fullscreen. Exiting fullscreen leaves media enabled.
7. Compare actual SSO client/site login status after the same idle duration. Awake Mini does not read SSO login state or report SSO success.

The fullscreen experiment is separate from blackout and adds no power requests or mouse input. **Existing Win+B blackout enables power/mouse requests, so avoid mixing it into isolated tests.** Entering fullscreen exits existing blackout; Win+B exits the fullscreen test and switches to normal blackout.

Pause all, lock/session disconnect, secure-desktop detection, suspend and app exit clear both test features. They do not automatically resume. Media teardown sets Closed, disables SMTC, clears metadata and releases objects. Fullscreen teardown removes the shell marking and hides the window.

No SSO process, network traffic, session token or security policy is accessed. **SSO effects and native Windows state visibility remain unverified.** Validation covers x86/x64 builds, ABI offsets, mocked Win32/WinRT lifecycle and existing Win+B/blackout regressions.


## Toggle black screen with Win+B

While the app is running in the tray, press **Windows key + B** to cover all monitors in black. **Press the same shortcut again to restore** the screen. Both left and right Windows keys are supported.

| Control | Action |
|---|---|
| Win+B | Toggle black screen on / off |
| Black screen in settings or right-click tray → Black screen | Toggle black screen |
| Small Restore button at the bottom-right of the black screen | Return to the original screen |

- Holding B toggles once. Release B and press it again to toggle again.
- While the app runs, black screen replaces the normal Windows Win+B notification-area focus action. Exiting the app restores the default action.
- During blackout, **keep-awake, keep-display-on and mouse input at the existing idle delay** stay enabled. Restoring returns to the original feature choices. No lock or sign-out is requested.
- The tray menu shows **hook installation status, detected count and mask failure count**. If the shortcut stops responding, release all keys and choose **Reconnect Win+B hook**.

On 2026-09-16, the user confirmed Win+B working on their PC and approved promotion to main. That report applies to the earlier **1.3.3-winb1** version, not these new state tests. The exact OS build and executable architecture were not separately recorded.

### Shortcut implementation notes

A dedicated-thread `WH_KEYBOARD_LL` hook consumes B down, repeats and up. Windows key events and other shortcuts pass through; Ctrl/Alt/Shift+B variants are not captured. A brief injected Ctrl down/up masks the Start menu when Win is released. No DLL injection or driver is used.

Installed records successful registration; detected counts toggle requests. Silent Windows hook removal after timeout cannot be directly detected. Lock screens and secure desktops are excluded. Other hooks, privilege differences and remote sessions need separate validation.

A small Windows tray utility providing keep-awake requests, display keep-on, idle mouse input, experimental update pause renewal, and launch at sign-in. No installer or additional runtime is required.

[한국어 README](README.ko.md)

## Executables and language

| File | Language | Target |
|---|---|---|
| [AwakeMini-v1.4.0-state1-x64-auto.exe](downloads/v1.4.0-state1/AwakeMini-v1.4.0-state1-x64-auto.exe?raw=true) | Automatic | 64-bit Windows |
| [AwakeMini-v1.4.0-state1-x64-ko.exe](downloads/v1.4.0-state1/AwakeMini-v1.4.0-state1-x64-ko.exe?raw=true) | Korean | 64-bit Windows |
| [AwakeMini-v1.4.0-state1-x64-en.exe](downloads/v1.4.0-state1/AwakeMini-v1.4.0-state1-x64-en.exe?raw=true) | English | 64-bit Windows |
| [AwakeMini-v1.4.0-state1-x86-auto.exe](downloads/v1.4.0-state1/AwakeMini-v1.4.0-state1-x86-auto.exe?raw=true) | Automatic | 32-bit or 64-bit Windows |
| [AwakeMini-v1.4.0-state1-x86-ko.exe](downloads/v1.4.0-state1/AwakeMini-v1.4.0-state1-x86-ko.exe?raw=true) | Korean | 32-bit or 64-bit Windows |
| [AwakeMini-v1.4.0-state1-x86-en.exe](downloads/v1.4.0-state1/AwakeMini-v1.4.0-state1-x86-en.exe?raw=true) | English | 32-bit or 64-bit Windows |

The automatic build uses Korean when the current user's Windows **display language** is Korean; otherwise it uses English. It does not use the Windows version number or keyboard input language. Restart the app after changing the display language. Fixed-language builds are separate executables and need no command-line options. Run only one executable at a time.


**Complete download:** [Six executables, source, and both READMEs](downloads/v1.4.0-state1/AwakeMini-v1.4.0-state1-source.zip?raw=true)

## Quick start

1. Exit any older Awake Mini instance using its tray menu.
2. Place the chosen EXE in a folder where it will remain and launch it.
3. Double-click the tray icon for settings, or right-click for the menu.
4. Select the features you want and click **Apply**.
5. **Minimize** or the window's X hides the settings window. Use **Exit program** to stop the app.

- Keep PC awake: requests an indefinite system awake state.
- Keep display on: used together with keep-awake.
- Prevent screen saver: after the selected idle period, sends a small mouse movement and return. Default 2 minutes; range 1–1440 minutes.
- Movement is skipped while a key or mouse button is held.
- Mouse and power requests pause on locked or secure desktops.
- Screen saver prevention may not work where synthetic input is restricted.
- Pause all applies to the current session. Other settings are saved.

## Black screen

Use **Win+B**, **Black screen** in settings, or **right-click the tray → Black screen**. All monitors are covered in black and the settings window is hidden. Only a small **Restore** button appears at the bottom-right of the primary monitor. Click **Restore** to return. Settings reappear only if they were visible before entering blackout.

- **Win+B** also toggles black screen on and off.
- The pointer is hidden over the black background and visible over Restore. Mouse movement or background clicks do not dismiss blackout. Alt+F4 remains an emergency exit.
- Keep-system-awake, keep-display-on and idle mouse input are temporarily enabled using the **existing idle delay** (default 2 minutes). Exiting restores the original feature choices.
- Held keys/buttons, recent input, locked/secure desktops and synthetic-input restrictions retain their existing behavior. Update renewal follows its existing ON/OFF and Pause all settings.
- No display-power-off, lock or sign-out request is made. LCD backlights stay on; organization-enforced lock policies still apply.
- Monitor changes reposition the cover and Restore button. Lock, session disconnect, secure desktop, suspend and exit remove the cover.
- Settings remain 206×133 DLU. Restore is 56×24px at 96 DPI and scales with DPI.
- The user confirmed button and Win+B operation. Native Windows UI and mixed-DPI behavior have not been directly tested in the development environment.

## Run at sign-in

Select **Run at sign-in → Apply**. The app starts in the tray after the current user signs in, without opening its settings window. It does not run before sign-in or for other users. Windows may delay startup applications.

- Default: OFF. Clear the checkbox and Apply to remove registration.
- The app records its current, fully quoted EXE path in the current user's Run entry. Other startup entries are left alone.
- After moving the EXE or switching to a new version or language build, enable startup and Apply from the new EXE to update the path.
- Startup uses **standard user privileges**. It does not silently elevate or open a UAC prompt at sign-in.
- To renew update pauses, exit the automatically launched instance and run the EXE as administrator.
- The app does not override a startup entry disabled in Windows Startup Apps or organization restrictions. Its checkbox reflects Run registration.

## Update pause renewal — experimental

Run as administrator and select **Pause updates (test) → Apply**. The settings window shows saved dates and Windows pause status values. Administrator and update-related failure statuses are not repeated in popup dialogs. If settings were saved but renewal was not applied, the button shows **Settings saved**.

- Initially writes now + 7 days; renews to now + 7 days after 24 hours since the last renewal.
- Uses a conservative maximum of 35 days from the original pause start. A shorter configured `SetMaxPauseDays` takes precedence.
- Does not shorten an existing longer pause.
- OFF / Pause all / Exit stops renewal and leaves the last pause expiry in place. Resume immediately through Windows Settings if needed.
- Does not write dates when `SetDisablePauseUXAccess=1`. Unrelated management settings alone do not block the feature.
- Policy read failures, unexpected values, external date changes, or a pending restart are reported as inline status.
- The actual pause action in Windows Settings can cancel some pending restarts. This app's registry date writes do not guarantee the same behavior.

This is an **unsupported experimental implementation** that writes WindowsUpdate `UX/Settings` dates directly. It cannot guarantee acceptance by Windows Update or prevent a restart. **Saved until** shows recorded dates. **OS reports** shows Windows `PausedQualityStatus` and `PausedFeatureStatus` values, which may lag. Use **Update settings** to confirm the actual state and expiry.

The app does not change update policies, services, scheduled tasks, or ACLs. Startup registration is a separate Run entry written only when the user enables it.

## Compatibility and validation

- Targets Windows 10 1903+ and Windows 11. No native ARM64 build is included.
- Settings dialog: 206×133 DLU. Pixel dimensions depend on DPI and font metrics.
- Uses Windows system DLLs; no separate .NET or Python installation is needed to run the app.
- Validation covers strict compilation of all six EXEs, language strings and placeholders, mocked startup registration, and update policy and date boundary checks. See `VERIFICATION.json` for the actual recorded checks.
- User-reported Win+B operation confirmed on 2026-09-16. Direct native Windows execution in the development environment, mixed-DPI behavior, launch after sign-in and organization-specific update behavior remain unverified.

## Source and build

Put MinGW-w64 i686/x86_64 gcc and windres on PATH.

```sh
sh build.sh
python3 src/test-policy-gate.py
python3 src/test-startup.py
python3 src/test-blackout.py
python3 src/test-win-b-hook.py
python3 src/test-state-test.py
cc -std=c11 -Wall -Wextra -Werror src/test-update-plan.c -o test-update-plan
./test-update-plan
```

Builds auto/ko/en × x86/x64 into `dist`. `AM_FORCE_LANGUAGE=0` selects automatic language, 1 Korean, and 2 English. Translations are in `src/language.c` and `src/translations.json`; update both when editing translations.

User settings: `HKCU\Software\AwakeMini`  
Startup: `AwakeMini` under `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`  
Renewal state: `HKLM\SOFTWARE\AwakeMini\UpdatePauseV1` (64-bit registry view)

## Official references

- [Windows startup registration](https://learn.microsoft.com/en-us/windows/win32/setupapi/run-and-runonce-registry-keys)
- [User display language](https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-getuserdefaultuilanguage)
- [User pause restriction policy](https://learn.microsoft.com/en-us/windows/client-management/mdm/policy-csp-update#setdisablepauseuxaccess)
- [Update pause behavior and status values](https://learn.microsoft.com/en-us/windows/deployment/update/waas-configure-wufb#pause-quality-updates)

License: [MIT](LICENSE.txt)

- [LowLevelKeyboardProc: callback, suppression and thread requirements](https://learn.microsoft.com/en-us/windows/win32/winmsg/lowlevelkeyboardproc)
- [SendInput: injected input and privilege limits](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendinput)

- [MarkFullscreenWindow](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-itaskbarlist2-markfullscreenwindow)
- [Desktop SMTC GetForWindow](https://learn.microsoft.com/en-us/windows/win32/api/systemmediatransportcontrolsinterop/nf-systemmediatransportcontrolsinterop-isystemmediatransportcontrolsinterop-getforwindow)
- [SystemMediaTransportControls](https://learn.microsoft.com/en-us/uwp/api/windows.media.systemmediatransportcontrols)
