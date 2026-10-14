# Awake Mini 1.4.0-state4

설치 없이 실행하는 작은 Windows 트레이 프로그램입니다. 절전 방지, 화면 유지, 유휴 마우스 입력, Win+B 검은화면, 로그인 시 자동 실행을 제공합니다.

**최신 버전: 1.4.0-state4** · [English](README.en.md) · [이전 버전: legacy-main](https://github.com/eljja/AwakeMini/tree/legacy-main)

## 다운로드

[GitHub Releases](https://github.com/eljja/AwakeMini/releases)에서 버전별 EXE와 ZIP을 받을 수 있습니다. `state` 버전은 시험판으로 표시합니다.

일반적인 64비트 Windows에서는 **x64 자동 언어판**을 선택하세요. 별도의 .NET·Python 설치가 필요 없습니다. 실행 파일 크기는 약 75~79KiB입니다.

| 대상 | 자동 언어 선택 | 한국어 고정 | 영어 고정 |
|---|---|---|---|
| 64비트 Windows | [EXE](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state4/AwakeMini-v1.4.0-state4-x64-auto.exe) | [EXE](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state4/AwakeMini-v1.4.0-state4-x64-ko.exe) | [EXE](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state4/AwakeMini-v1.4.0-state4-x64-en.exe) |
| 32·64비트 Windows | [EXE](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state4/AwakeMini-v1.4.0-state4-x86-auto.exe) | [EXE](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state4/AwakeMini-v1.4.0-state4-x86-ko.exe) | [EXE](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state4/AwakeMini-v1.4.0-state4-x86-en.exe) |

[전체 ZIP — EXE 6개·소스·한글/영문 설명서](https://github.com/eljja/AwakeMini/releases/download/v1.4.0-state4/AwakeMini-v1.4.0-state4-source.zip)

자동 언어판은 Windows **표시 언어**가 한국어이면 한국어, 그 외에는 영어를 사용합니다. Windows 버전 번호나 키보드 입력 언어로 선택하지 않습니다. 언어 변경 후에는 프로그램을 다시 실행하세요. ARM64 네이티브 파일은 제공하지 않습니다.

## 빠른 시작

1. 기존 Awake Mini를 트레이 우클릭 → **종료**로 끝냅니다.
2. 원하는 EXE를 계속 보관할 폴더에 놓고 실행합니다. 여러 언어/아키텍처 파일 중 하나만 실행하세요.
3. 트레이 아이콘을 **더블클릭**하면 설정창, **우클릭**하면 메뉴가 열립니다.
4. 일반 설정은 체크하고 **적용**을 누릅니다. 성공하면 버튼이 **적용 완료**로 바뀝니다.
5. **최소화** 또는 X는 설정창만 숨깁니다. 프로그램을 끝내려면 **프로그램 종료**를 누릅니다.

전체화면 신호와 미디어 재생 상태는 **체크 즉시 적용**되며 적용 버튼을 누르지 않아도 됩니다.

## 최신 변경 사항

- 설정창의 **절전방지** 문구를 짧게 변경했습니다.
- **검은화면** 버튼 왼쪽에 **해제표시** 체크박스를 추가했습니다.
- 체크하면 검은 화면의 작은 **해제** 버튼을 표시하고, 해제하면 숨깁니다. 처음 실행 시 기본값은 ON입니다.
- 체크 즉시 현재 실행에 반영됩니다. **적용**을 누르면 다음 실행에도 선택을 유지합니다.
- 버튼이 숨겨져 있어도 **Win+B**로 검은화면을 해제할 수 있습니다. 절전·화면 유지·기존 간격의 마우스 입력은 계속 동작합니다.
- 설정창 크기는 **250×145 DLU**로 유지했습니다.

## 기능

| 기능 | 동작 |
|---|---|
| 절전 방지 | 실행 중 무기한 전원 유지 요청 |
| 화면 켜기 유지 | 절전 방지와 함께 모니터 유지 요청 |
| 화면보호기 방지 | 지정한 유휴 시간 후 마우스를 조금 움직이고 즉시 원위치로 복귀 |
| 전체화면 신호 | 작은 기본 설정창에 `MarkFullscreenWindow(TRUE)` 표시만 설정. 크기 변경이나 화면 덮기 없음 |
| 미디어 재생 상태 | 앱의 SMTC에 Video 유형과 Playing 상태 등록. 실제 영상·소리 재생 없음 |
| QUNS | `SHQueryUserNotificationState`의 현재 알림 상태 조회 |
| Windows 시작 시 실행 | 현재 사용자 로그인 후 설정창 없이 트레이 실행 |
| 업데이트 일시중지 연장 (시험) | 24시간마다 오늘+7일 기한 기록. 실제 상태는 Windows 설정에서 확인 |

마우스 대기 시간은 기본 **2분**, 범위 **1~1440분**입니다. 최근 사용자 입력이 있거나 키·마우스 버튼을 누르고 있으면 이동을 생략합니다. 모의 입력을 제한하는 설정에서는 화면보호기 방지가 작동하지 않을 수 있습니다.

## Win+B 검은화면

**Windows 키 + B**로 모든 모니터를 검게 덮고, 같은 키를 다시 누르면 해제합니다. 설정창/트레이의 **검은화면**도 사용할 수 있습니다. **해제표시**가 ON이면 검은 화면 오른쪽 아래에 작은 **해제** 버튼이 나타납니다. B를 길게 누르면 한 번만 전환됩니다.

- 검은화면 동안 절전 방지·화면 유지·기존 대기 시간의 마우스 입력을 활성화합니다. 해제하면 원래 기능 선택으로 복귀합니다.
- 실제 모니터 전원을 끄거나 잠금·로그아웃을 요청하지 않습니다. LCD 백라이트는 켜져 있습니다.
- 검은 배경에서는 포인터를 숨깁니다. 마우스 이동이나 배경 클릭으로 해제되지 않습니다. 비상시 Alt+F4로 해제할 수 있습니다.
- 실행 중에는 Windows 기본 Win+B 동작을 검은화면 기능에 사용합니다. 종료하면 기본 동작으로 돌아갑니다.
- 반응이 없으면 키를 모두 놓고 트레이의 **Win+B 훅 다시 연결**을 선택하세요. 메뉴에서 훅 상태·감지 횟수·보조 입력 실패를 확인할 수 있습니다.

DLL 주입이나 드라이버 없이 전용 스레드의 `WH_KEYBOARD_LL` 훅을 사용합니다. 잠금·보안 데스크톱에서는 동작하지 않으며, 회사의 강제 잠금 정책은 그대로 적용됩니다.

## 전체화면 신호·미디어 재생 상태

두 기능은 함께 켤 수 있습니다. 전체화면 신호를 트레이에서 켜면 기본 설정창이 열립니다. X·최소화로 숨겨도 표시 설정은 유지됩니다. Windows 문서상 셸 효과는 대상 창이 활성일 때 적용되므로 다른 창으로 전환한 경우는 따로 확인해야 합니다.

트레이 상태 줄은 신호의 활성/비활성 상태, 미디어 상태와 오류를 표시합니다. 실패한 항목은 체크가 꺼집니다. API 성공은 QUNS 변경이나 SSO 유지 성공을 의미하지 않습니다. **미디어 상태는 실제 영상 재생을 대신하지 않습니다.**

- 전체 일시정지·잠금·세션 연결 해제·보안 데스크톱 감지·절전·종료 시 두 상태를 해제하며 자동 재활성화하지 않습니다.
- 검은화면을 켜면 전체화면 신호를 끄고, 신호를 켜면 기존 검은화면을 해제합니다. 미디어 상태는 독립적으로 유지됩니다.
- 두 상태 항목 자체는 전원 요청이나 마우스 입력을 추가하지 않습니다.
- 표시 해제가 실패하면 표시가 남지 않도록 설정창을 닫습니다. 트레이 더블클릭으로 다시 열 수 있으며, 아직 적용하지 않은 일반 설정은 다시 입력해야 할 수 있습니다.

SSO 프로그램·통신·세션 토큰·보안 정책에는 접근하지 않습니다. 실제 Windows 상태 노출과 SSO 유지 효과는 미검증입니다.

## Windows 시작 시 실행

**Windows 시작 시 실행 → 적용**을 선택합니다. 기본값은 OFF이며 체크 해제 → 적용으로 등록을 제거합니다. 현재 EXE 경로를 현재 사용자 Run 항목에 저장합니다.

EXE를 이동하거나 교체하면 새 EXE에서 체크 → 적용으로 경로를 갱신하세요. 자동 실행은 **일반 권한**입니다. 업데이트 연장이 필요하면 자동 실행된 프로그램을 종료하고 관리자 권한으로 실행해야 합니다. 다른 사용자의 로그인이나 로그인 전에는 실행하지 않으며, Windows가 시작을 지연할 수 있습니다.

## 업데이트 일시중지 연장 — 시험 기능

관리자 권한으로 실행하여 **업데이트 일시중지 연장 (시험) → 적용**을 선택합니다. 기록 날짜와 OS 중지 상태값을 창에 표시하며, 관련 실패를 별도 팝업으로 반복하지 않습니다.

- 시작 시 오늘+7일, 마지막 갱신부터 24시간이 지나면 오늘+7일로 갱신합니다.
- 최초 중지일부터 최대 35일을 상한으로 사용합니다. 더 짧은 `SetMaxPauseDays` 정책은 해당 상한을 따릅니다.
- 기존의 더 긴 기간을 줄이지 않습니다. OFF·전체 일시정지·종료는 자동 갱신만 멈추고 기록된 기한을 유지합니다.
- `SetDisablePauseUXAccess=1`, 정책 읽기 실패, 예상하지 못한 값, 외부 날짜 변경 또는 재부팅 대기는 상태 문구로 알립니다.

WindowsUpdate `UX/Settings`의 날짜를 직접 기록하는 **비공식 시험 기능**으로, Windows Update가 수락하거나 재부팅을 막는다고 보장할 수 없습니다. **업데이트 설정** 버튼에서 실제 상태와 기한을 확인하세요. 정책·서비스·예약 작업·ACL을 변경하지 않습니다.

## 지원 및 검증

Windows 10 1903 이상 / Windows 11을 대상으로 빌드했습니다. x86/x64 auto/ko/en 6개 빌드, PE 리소스와 배치, 번역, 모의 Win32/WinRT 상태 생명주기, 해제 버튼 표시/숨김 및 기존 Win+B·검은화면 검사를 통과했습니다. 상세 결과는 [VERIFICATION.json](VERIFICATION.json), 실행 파일 해시는 [SHA256SUMS.txt](SHA256SUMS.txt)에 있습니다.

사용자가 이전 **1.3.3-winb1**에서 Win+B 동작을 확인했습니다. 이 확인은 이번 버전의 새 상태 기능을 검증한 결과가 아닙니다. 개발 환경에서 실제 Windows 실행, 혼합 DPI, 로그인 자동 실행 및 회사별 업데이트/SSO 동작은 검증하지 못했습니다.

## 브랜치와 릴리스

현재 기본 브랜치는 `experiment/presentation-media-state`입니다. 이전 기본 브랜치는 `legacy-main`으로 보존했습니다.

`release.yml`은 현재 기본 브랜치에 새 버전 패키지를 게시하면 체크섬과 ZIP 내용을 확인한 뒤 Releases에 등록합니다. 별도 비밀번호나 개인 토큰이 필요하지 않습니다. 이미 공개한 같은 버전은 덮어쓰지 않으므로 변경된 EXE는 새 버전 번호로 게시하세요. 중단된 초안은 Actions에서 재실행할 수 있습니다.

## 소스 및 빌드

MinGW-w64 i686/x86_64 gcc와 windres를 PATH에 두고 실행합니다.

```sh
sh build.sh
python3 src/test-state-test.py
python3 src/test-blackout.py
python3 src/test-win-b-hook.py
```

`dist`에 auto/ko/en × x86/x64 파일이 생성됩니다. 번역 수정 시 `src/language.c`와 `src/translations.json`을 함께 갱신하세요.

- 사용자 설정: `HKCU\Software\AwakeMini`
- 시작 등록: 현재 사용자 `Run`의 `AwakeMini`
- 업데이트 갱신 기록: `HKLM\SOFTWARE\AwakeMini\UpdatePauseV1` (64비트 뷰)

## 공식 문서

- [전원 유지 요청](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setthreadexecutionstate)
- [전체화면 표시](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-itaskbarlist2-markfullscreenwindow)
- [알림 상태 조회](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shqueryusernotificationstate)
- [미디어 상태](https://learn.microsoft.com/en-us/windows/win32/api/systemmediatransportcontrolsinterop/nf-systemmediatransportcontrolsinterop-isystemmediatransportcontrolsinterop-getforwindow)
- [시작 등록](https://learn.microsoft.com/en-us/windows/win32/setupapi/run-and-runonce-registry-keys)

라이선스: [MIT](LICENSE.txt)
