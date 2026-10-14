# Awake Mini 1.4.0-state3

A small portable Windows tray utility for keeping the PC awake, keeping the display on, sending idle mouse input, toggling a black screen with Win+B, and starting at sign-in.

**Latest version: 1.4.0-state3** · [한국어](README.ko.md) · [Previous version: legacy-main](https://github.com/eljja/AwakeMini/tree/legacy-main)

## Downloads

[GitHub Releases](https://github.com/eljja/AwakeMini/releases) provides versioned EXEs and ZIPs. `state` versions are marked as prereleases.

For a typical 64-bit Windows PC, choose **x64 automatic language**. No separate .NET or Python installation is required. Executables are approximately 75–79 KiB.

| Target | Automatic language | Korean | English |
|---|---|---|---|
| 64-bit Windows | [EXE](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state3/AwakeMini-v1.4.0-state3-x64-auto.exe) | [EXE](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state3/AwakeMini-v1.4.0-state3-x64-ko.exe) | [EXE](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state3/AwakeMini-v1.4.0-state3-x64-en.exe) |
| 32-bit or 64-bit Windows | [EXE](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state3/AwakeMini-v1.4.0-state3-x86-auto.exe) | [EXE](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state3/AwakeMini-v1.4.0-state3-x86-ko.exe) | [EXE](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state3/AwakeMini-v1.4.0-state3-x86-en.exe) |

[Complete ZIP — six EXEs, source, Korean/English documentation](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state3/AwakeMini-v1.4.0-state3-source.zip)

The automatic build uses Korean when the Windows **display language** is Korean; otherwise it uses English. It does not use the Windows version number or keyboard input language. Restart after changing the display language. No native ARM64 build is included.

## Quick start

1. Exit any older instance using the tray menu.
2. Place the selected EXE in a permanent folder and run it. Use only one language/architecture build at a time.
3. **Double-click** the tray icon for settings; **right-click** for the menu.
4. Select regular settings and click **Apply**. Successful application changes the button to **Applied**.
5. **Minimize** or X hides settings. Use **Exit program** to stop the app.

Fullscreen flag and Media playing state take effect **immediately**, without Apply.

## What's new

- **Fullscreen flag** and **Media playing state** are beside **Keep display on**.
- The actual Windows **QUNS state and numeric value** appear directly below.
- The separate state-test dialog and actual-fullscreen test were removed. Existing Win+B black screen remains available.
- Settings measure **250×145 DLU**; pixel dimensions vary with DPI and font metrics.
- Both state features start **OFF** on every launch. Other feature choices and idle delay are saved. Pause all applies only to the current run.

## Features

| Feature | Action |
|---|---|
| Keep PC awake | Requests an indefinite system awake state while running |
| Keep display on | Requests display retention together with keep-awake |
| Prevent screen saver | Sends a small mouse movement and immediate return after the selected idle delay |
| Fullscreen flag | Marks the small settings window with `MarkFullscreenWindow(TRUE)`, without resizing or covering the screen |
| Media playing state | Registers Video and Playing in the app's SMTC session; no video/audio playback |
| QUNS | Reads the current notification state through `SHQueryUserNotificationState` |
| Run at sign-in | Starts in the current user's tray after sign-in |
| Pause updates (test) | Records now + 7 days every 24 hours; confirm acceptance in Windows Settings |

Mouse idle delay defaults to **2 minutes**, with a range of **1–1440 minutes**. Movement is skipped during recent input or while a key/mouse button is held. Screen saver prevention may fail where synthetic input is restricted.

## Win+B black screen

Press **Windows + B** to cover all monitors in black. Press the same shortcut again to restore. Use the settings/tray **Black screen** command or the small **Restore** button at the bottom-right of the primary display. Holding B toggles only once.

- Black screen temporarily enables keep-awake, display-on and mouse input using the **existing idle delay**. Exiting restores the original selections.
- It does not power off the monitor or request lock/sign-out. LCD backlights remain on.
- The pointer is hidden over the background. Mouse movement and background clicks do not dismiss it. Alt+F4 is an emergency exit.
- While running, the app repurposes Windows' default Win+B action. Exiting restores the default behavior.
- If the shortcut stops responding, release all keys and choose **Reconnect Win+B hook** in the tray. Hook state, detections and masking-input failures are shown there.

A dedicated-thread `WH_KEYBOARD_LL` hook is used without DLL injection or a driver. It does not operate on locked/secure desktops. Organization-enforced lock policies still apply.

## Fullscreen flag and media state

Both controls can be enabled together. Enabling the flag from the tray opens the main settings window. X/Minimize retains the mark. The documented shell effect applies while the marked window is active; background behavior requires separate checking.

The tray status row shows active/inactive flag state, media state and errors. Failed features are unchecked. API success does not establish a QUNS change or SSO retention. **Media state does not substitute for real video playback.**

- Pause all, lock, disconnect, secure desktop detection, suspend and exit clear both states without automatic reactivation.
- Entering black screen clears the flag; enabling the flag exits black screen. Media state remains independent.
- Neither state feature adds power requests or mouse input.
- If unmarking fails, settings are destroyed to remove the marked window's lifetime. Reopen using the tray; unapplied regular settings may need to be entered again.

The app does not access SSO processes, traffic, tokens or security policies. Native state visibility and SSO retention remain unverified.

## Run at sign-in

Select **Run at sign-in → Apply**. Default is OFF; clear and Apply to remove registration. The current EXE path is recorded in the current user's Run entry.

After moving/replacing the EXE, enable and Apply from the new file to refresh its path. Startup uses **standard user privileges**. For update renewal, exit the automatically launched instance and restart as administrator. It does not run before sign-in or for other users; Windows may delay startup.

## Update pause renewal — experimental

Run as administrator and select **Pause updates (test) → Apply**. Recorded dates and OS pause status appear inline; related failures are not repeated in popups.

- Initially records now + 7 days; renews after 24 hours since the last renewal.
- Uses a 35-day maximum from the original pause start. A shorter `SetMaxPauseDays` policy takes precedence.
- Does not shorten an existing longer pause. OFF, Pause all and Exit stop renewal but keep the recorded expiry.
- `SetDisablePauseUXAccess=1`, policy read failures, unexpected values, external date changes and pending restarts are reported inline.

This is an **unsupported experimental feature** that writes WindowsUpdate `UX/Settings` dates directly. Acceptance by Windows Update and prevention of restarts cannot be guaranteed. Use **Update settings** to confirm actual status and expiry. Policies, services, scheduled tasks and ACLs are not modified.

## Compatibility and validation

Targets Windows 10 1903+ / Windows 11. All six x86/x64 auto/ko/en builds, PE resources/layout, translations, mocked Win32/WinRT state lifecycle, and existing Win+B/black-screen checks passed. See [VERIFICATION.json](VERIFICATION.json) and [SHA256SUMS.txt](SHA256SUMS.txt).

The user confirmed Win+B on the earlier **1.3.3-winb1**. That report does not validate these new state features. Native Windows execution, mixed DPI, startup, and organization-specific update/SSO behavior have not been tested in the development environment.

## Branches and releases

The default branch is `experiment/presentation-media-state`. The former default is preserved as `legacy-main`.

`release.yml` verifies checksums and ZIP contents, then publishes a new packaged version pushed to the current default branch. No separate password or personal token is required. Existing published versions are not overwritten; use a new version number for changed executables. Interrupted drafts can be resumed by rerunning the Actions workflow.

## Source and build

Put MinGW-w64 i686/x86_64 gcc and windres on PATH, then run:

```sh
sh build.sh
python3 src/test-state-test.py
python3 src/test-blackout.py
python3 src/test-win-b-hook.py
```

Six auto/ko/en × x86/x64 files are written to `dist`. Update both `src/language.c` and `src/translations.json` when changing translations.

- User settings: `HKCU\Software\AwakeMini`
- Startup: `AwakeMini` in the current user's `Run` entry
- Renewal records: `HKLM\SOFTWARE\AwakeMini\UpdatePauseV1` (64-bit view)

## Official documentation

- [Power requests](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadexecutionstate)
- [Fullscreen marking](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-itaskbarlist2-markfullscreenwindow)
- [Notification-state query](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shqueryusernotificationstate)
- [Media state](https://learn.microsoft.com/en-us/windows/win32/api/systemmediatransportcontrolsinterop/nf-systemmediatransportcontrolsinterop-isystemmediatransportcontrolsinterop-getforwindow)
- [Startup registration](https://learn.microsoft.com/en-us/windows/win32/setupapi/run-and-runonce-registry-keys)

License: [MIT](LICENSE.txt)
