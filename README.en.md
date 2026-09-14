# Awake Mini 1.2.0

A small Windows tray utility providing keep-awake requests, display keep-on, idle mouse input, experimental update pause renewal, and launch at sign-in. No installer or additional runtime is required.

[한국어 README](README.ko.md)

## Executables and language

| File | Language | Target |
|---|---|---|
| [AwakeMini-v1.2.0-x64-auto.exe](downloads/v1.2.0/AwakeMini-v1.2.0-x64-auto.exe?raw=true) | Automatic | 64-bit Windows |
| [AwakeMini-v1.2.0-x64-ko.exe](downloads/v1.2.0/AwakeMini-v1.2.0-x64-ko.exe?raw=true) | Korean | 64-bit Windows |
| [AwakeMini-v1.2.0-x64-en.exe](downloads/v1.2.0/AwakeMini-v1.2.0-x64-en.exe?raw=true) | English | 64-bit Windows |
| [AwakeMini-v1.2.0-x86-auto.exe](downloads/v1.2.0/AwakeMini-v1.2.0-x86-auto.exe?raw=true) | Automatic | 32-bit or 64-bit Windows |
| [AwakeMini-v1.2.0-x86-ko.exe](downloads/v1.2.0/AwakeMini-v1.2.0-x86-ko.exe?raw=true) | Korean | 32-bit or 64-bit Windows |
| [AwakeMini-v1.2.0-x86-en.exe](downloads/v1.2.0/AwakeMini-v1.2.0-x86-en.exe?raw=true) | English | 32-bit or 64-bit Windows |

The automatic build uses Korean when the current user's Windows **display language** is Korean; otherwise it uses English. It does not use the Windows version number or keyboard input language. Restart the app after changing the display language. Fixed-language builds are separate executables and need no command-line options. Run only one executable at a time.


**Complete download:** [Six executables, source, and both READMEs](downloads/v1.2.0/AwakeMini-v1.2.0-source.zip?raw=true)

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
- Native Windows UI, automatic launch after sign-in, and organization-specific update behavior have not been tested in this environment.

## Source and build

Put MinGW-w64 i686/x86_64 gcc and windres on PATH.

```sh
sh build.sh
python3 src/test-policy-gate.py
python3 src/test-startup.py
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
