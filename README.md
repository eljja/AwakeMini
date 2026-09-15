# Awake Mini 1.3.1

작은 Windows 트레이 유틸리티입니다. 설치나 별도 런타임 없이 절전 방지, 화면 유지, 유휴 마우스 입력, 업데이트 일시중지 연장(시험), 로그인 시 자동 실행을 제공합니다.

[English README](README.en.md)

## 실행 파일과 언어

| 파일 | 언어 | 대상 |
|---|---|---|
| [AwakeMini-v1.3.1-x64-auto.exe](downloads/v1.3.1/AwakeMini-v1.3.1-x64-auto.exe?raw=true) | Windows 표시 언어에 따라 자동 선택 | 64비트 Windows |
| [AwakeMini-v1.3.1-x64-ko.exe](downloads/v1.3.1/AwakeMini-v1.3.1-x64-ko.exe?raw=true) | 한국어 고정 | 64비트 Windows |
| [AwakeMini-v1.3.1-x64-en.exe](downloads/v1.3.1/AwakeMini-v1.3.1-x64-en.exe?raw=true) | 영어 고정 | 64비트 Windows |
| [AwakeMini-v1.3.1-x86-auto.exe](downloads/v1.3.1/AwakeMini-v1.3.1-x86-auto.exe?raw=true) | Windows 표시 언어에 따라 자동 선택 | 32·64비트 Windows |
| [AwakeMini-v1.3.1-x86-ko.exe](downloads/v1.3.1/AwakeMini-v1.3.1-x86-ko.exe?raw=true) | 한국어 고정 | 32·64비트 Windows |
| [AwakeMini-v1.3.1-x86-en.exe](downloads/v1.3.1/AwakeMini-v1.3.1-x86-en.exe?raw=true) | 영어 고정 | 32·64비트 Windows |

자동 선택판은 현재 사용자의 Windows **표시 언어**가 한국어이면 한국어, 그 외에는 영어로 표시합니다. Windows 10/11 버전 번호나 키보드 입력 언어로 결정하지 않습니다. 언어 변경 후에는 프로그램을 다시 실행하세요. 언어 고정판은 별도 파일이며 명령줄 옵션이 필요 없습니다. 실행 파일 중 하나만 사용하세요.


**전체 다운로드:** [EXE 6개·소스·한영 설명서 ZIP](downloads/v1.3.1/AwakeMini-v1.3.1-source.zip?raw=true)

## 검은 화면 — Win + B

**Win + B**를 누른 뒤 두 키를 놓으면 모든 모니터를 검은 창으로 덮고 커서를 숨깁니다. **같은 조합을 다시 눌렀다 놓으면 해제**됩니다. 마우스 이동·클릭이나 Esc로는 해제되지 않습니다. 비상시 Alt+F4로 검은 창을 닫을 수 있습니다.

설정창의 **절전 방지 오른쪽에 있는 ‘검은화면’ 버튼**으로도 바로 시험할 수 있습니다. 버튼으로 실행하면 설정창을 검은 배경 위에 남기고 버튼 이름을 **‘검은화면 해제’**로 바꿉니다. 같은 버튼을 다시 누르면 해제됩니다. 이 시험 중 설정창을 최소화하거나 닫아도 검은 화면을 해제합니다. 별도로 적용 버튼을 누를 필요는 없습니다. 설정창 크기는 그대로입니다.

- 디스플레이 전원은 켜진 상태입니다. 화면 전원 끄기·화면보호기·잠금·로그아웃·절전을 요청하지 않습니다. LCD 백라이트는 계속 켜져 있습니다.
- 검은 화면 동안 절전 방지·화면 유지·유휴 마우스 입력을 임시 활성화합니다. 기존에 OFF 또는 전체 일시정지였어도 적용하며, 해제하면 원래 선택으로 돌아갑니다. 저장값은 바꾸지 않습니다.
- 마우스는 **기존 대기 시간**(기본 2분)을 그대로 사용합니다. 최근 입력, 누르고 있는 키·버튼, 잠금·보안 데스크톱, 모의 입력 제한은 기존처럼 확인합니다. 자동 마우스 이동으로 검은 화면이 해제되지 않습니다.
- 업데이트 연장은 기존 ON/OFF 및 전체 일시정지 설정을 따릅니다.
- 모니터 배치 변경 시 덮는 영역을 다시 맞춥니다. 잠금·세션 연결 해제·보안 데스크톱·절전 진입·프로그램 종료 시 검은 창을 정리하며, 로그인·복귀 후 자동으로 다시 표시하지 않습니다.
- Win+B는 예약된 단축키 등록 대신 백그라운드 Raw Input으로 감지합니다. 키 입력을 차단하거나 추가 키를 주입하지 않으므로 Windows 기본 트레이 초점 이동이 잠깐 나타날 수 있습니다. 왼쪽·오른쪽 Win 키를 지원하며, Ctrl/Alt/Shift를 함께 누른 조합은 처리하지 않습니다.
- 단축키 입력 등록 실패 시에도 설정창 버튼은 별도로 동작합니다. 단축키 실패 상태는 트레이 메뉴에 표시합니다. 포커스 요청 실패만으로 검은 창을 즉시 취소하지 않습니다.
- 검은 창은 보안 잠금 화면이 아닙니다. 시스템 화면이나 다른 최상위 창이 위에 나타날 수 있으며, 회사에서 강제하는 잠금·화면보호기 정책은 그대로 적용됩니다.
- 실제 Windows의 단축키, 검은 화면 렌더링, 혼합 DPI 다중 모니터 동작은 이 환경에서 검증하지 못했습니다.

## 기본 사용법

1. 기존 Awake Mini를 트레이 우클릭 → 종료로 끝냅니다.
2. 원하는 EXE를 계속 보관할 폴더에 둔 뒤 실행합니다.
3. 트레이 아이콘 더블클릭으로 설정창을 엽니다. 우클릭으로 메뉴를 엽니다.
4. 원하는 기능을 체크하고 **적용**을 누릅니다.
5. **최소화** 또는 창의 X를 누르면 창만 숨깁니다. 종료는 **프로그램 종료**를 사용합니다.

- 절전 방지: 무기한 전원 유지 요청.
- 화면 켜기 유지: 절전 방지와 함께 사용.
- 화면보호기 방지: 지정한 시간 동안 입력이 없으면 작은 마우스 이동 입력을 보낸 뒤 복귀. 기본 2분, 범위 1~1440분.
- 사용자가 키나 마우스 버튼을 누르고 있으면 이동을 생략합니다.
- 잠금/보안 데스크톱에서는 마우스 및 전원 유지 기능을 쉽니다.
- 모의 입력이 제한된 환경에서는 화면보호기 방지가 작동하지 않을 수 있습니다.
- 전체 일시정지는 현재 실행에만 적용됩니다. 다른 설정은 저장됩니다.

## Windows 시작 시 실행

설정창에서 **Windows 시작 시 실행 → 적용**을 선택합니다. 현재 사용자가 Windows에 로그인한 뒤 별도 설정창 없이 트레이에서 실행합니다. 로그온 전이나 다른 사용자의 세션에서는 실행하지 않습니다. Windows가 시작 프로그램 실행을 지연할 수 있습니다.

- 기본값은 OFF입니다. 체크 해제 → 적용으로 등록을 제거합니다.
- 현재 EXE의 전체 경로를 현재 사용자 Run 항목에 기록합니다. 다른 시작 프로그램을 변경하지 않습니다.
- EXE를 이동하거나 새 버전/다른 언어 파일로 교체한 경우, 해당 EXE에서 체크 → 적용을 다시 눌러 경로를 갱신하세요.
- 자동 실행은 **일반 권한**입니다. 관리자 권한을 자동으로 얻거나 로그인 때 UAC 창을 띄우지 않습니다.
- 업데이트 연장이 필요하면 자동 실행된 프로그램을 종료하고 해당 EXE를 관리자 권한으로 실행해야 합니다.
- Windows 시작 프로그램 설정에서 사용자가 비활성화한 항목이나 조직의 시작 프로그램 제한을 강제로 해제하지 않습니다. 앱의 체크는 Run 항목 등록 여부를 나타냅니다.

## 업데이트 일시중지 연장 — 시험 기능

관리자 권한으로 실행하고 **업데이트 일시중지 연장 (시험) → 적용**을 선택합니다. 날짜 기록 결과와 Windows의 중지 상태값이 설정창에 표시됩니다. 관리자 권한 부족 및 업데이트 관련 실패 상태는 별도 팝업으로 반복하지 않습니다. 설정 저장은 성공했지만 업데이트 연장이 미적용이면 버튼은 **설정 저장됨**으로 표시합니다.

- 시작 시 오늘+7일, 이후 마지막 갱신부터 24시간이 지나면 오늘+7일로 갱신합니다.
- 최초 중지일부터 최대 35일을 보수적인 상한으로 사용합니다. `SetMaxPauseDays`가 더 짧으면 해당 값을 따릅니다.
- 기존의 더 긴 일시중지 기간을 줄이지 않습니다.
- OFF / 전체 일시정지 / 종료 시 자동 갱신만 멈추고 마지막 중지 기한은 유지합니다. 즉시 재개는 Windows 설정에서 하세요.
- `SetDisablePauseUXAccess=1`이면 변경하지 않습니다. 관련 없는 관리 정책의 존재만으로 중단하지 않습니다.
- 정책 읽기 실패/알 수 없는 값/외부 날짜 변경/재부팅 대기 등은 상태 문구로 알립니다.
- Windows 설정 앱의 실제 일시중지 동작은 일부 재부팅 대기를 취소할 수 있습니다. 이 앱의 레지스트리 날짜 기록 방식은 그 동작까지 보장하지 않습니다.

이 기능은 WindowsUpdate `UX/Settings`의 날짜를 직접 기록하는 **비공식 시험 구현**입니다. Windows Update가 이를 수락하거나 실제 재부팅을 막는다는 보장은 없습니다. `기록 기한`은 저장된 날짜이며, `OS 상태값`은 Windows의 `PausedQualityStatus`와 `PausedFeatureStatus`를 읽은 결과입니다. 상태값은 지연될 수 있습니다. **업데이트 설정** 버튼을 눌러 실제 상태와 기한을 확인하세요.

정책/서비스/예약 작업/ACL을 변경하지 않습니다. 시작 시 실행은 사용자가 체크한 경우에만 별도 Run 항목을 기록합니다.

## 지원 및 검증 범위

- Windows 10 1903 이상 / Windows 11을 대상으로 빌드했습니다. ARM64 네이티브 빌드는 없습니다.
- 설정창은 206×133 DLU입니다. 픽셀 크기는 DPI와 글꼴에 따라 달라집니다.
- Windows 기본 DLL을 사용하며 별도 .NET/Python 설치가 필요 없습니다.
- 6개 EXE의 엄격한 컴파일, 언어 문자열/자리표시자, 시작 등록 모의 검사, 업데이트 정책 및 기간 경계 검사를 수행합니다. 실제 결과는 `VERIFICATION.json`을 참고하세요.
- 실제 Windows UI 표시, 로그인 자동 실행, 회사 PC의 업데이트 동작은 이 환경에서 검증하지 못했습니다.

## 소스 및 빌드

MinGW-w64의 i686/x86_64 gcc와 windres를 PATH에 둡니다.

```sh
sh build.sh
python3 src/test-policy-gate.py
python3 src/test-startup.py
python3 src/test-blackout.py
cc -std=c11 -Wall -Wextra -Werror src/test-blackout-keys.c -o test-blackout-keys
./test-blackout-keys
cc -std=c11 -Wall -Wextra -Werror src/test-update-plan.c -o test-update-plan
./test-update-plan
```

`dist`에 auto/ko/en × x86/x64 실행 파일이 생성됩니다. 자동 선택은 `AM_FORCE_LANGUAGE=0`, 한국어 고정은 1, 영어 고정은 2입니다. 번역은 `src/language.c` 및 `src/translations.json`에 있습니다. 언어 문자열 수정 시 두 파일을 함께 갱신하세요.

사용자 설정: `HKCU\Software\AwakeMini`  
시작 등록: `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`의 `AwakeMini`  
업데이트 갱신 상태: `HKLM\SOFTWARE\AwakeMini\UpdatePauseV1` (64비트 뷰)

## 공식 참고자료

- [Windows 시작 등록](https://learn.microsoft.com/en-us/windows/win32/setupapi/run-and-runonce-registry-keys)
- [사용자 표시 언어](https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-getuserdefaultuilanguage)
- [사용자 업데이트 일시중지 금지 정책](https://learn.microsoft.com/en-us/windows/client-management/mdm/policy-csp-update#setdisablepauseuxaccess)
- [업데이트 일시중지와 상태값](https://learn.microsoft.com/en-us/windows/deployment/update/waas-configure-wufb#pause-quality-updates)

라이선스: [MIT](LICENSE.txt)
