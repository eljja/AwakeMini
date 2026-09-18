#define UNICODE
#define _UNICODE
#include <windows.h>
#include "language.h"
#ifndef AM_FORCE_LANGUAGE
#define AM_FORCE_LANGUAGE 0
#endif
static BOOL korean;
static const struct { const WCHAR *ko; const WCHAR *en; } strings[] = {
    {L"1분", L"1 min"},
    {L"2분 (기본)", L"2 min (default)"},
    {L"5분", L"5 min"},
    {L"10분", L"10 min"},
    {L"대기 중", L"Waiting"},
    {L"일시정지", L"Paused"},
    {L"잠금 / 세션 대기", L"Locked / session idle"},
    {L"실행 중", L"Running"},
    {L"모두 꺼짐", L"All off"},
    {L"Awake Mini — %s\n절전 %s / 화면 %s / 마우스 %s (%lu분)\n더블클릭: 설정", L"Awake Mini — %s\nAwake %s / Screen %s / Mouse %s (%lu min)\nDouble-click: settings"},
    {L"모의 입력이 화면보호기 설정에 의해 제한됨", L"Synthetic input is restricted"},
    {L"유휴 시간 확인 실패", L"Cannot read idle time"},
    {L"최근 이동 입력 전송 성공", L"Mouse input sent"},
    {L"이동 입력 전송 실패 / 일부 전송", L"Mouse input failed / incomplete"},
    {L"Awake Mini · 일시정지", L"Awake Mini · Paused"},
    {L"Awake Mini · 트레이 모드", L"Awake Mini · Tray mode"},
    {L"설정 열기 (더블클릭)", L"Open settings (double-click)"},
    {L"절전 방지 · 무기한 유지", L"Keep PC awake indefinitely"},
    {L"화면 켜기 유지", L"Keep display on"},
    {L"화면보호기 방지 · 마우스 미세 이동", L"Prevent screen saver (mouse)"},
    {L"마우스 이동 전 유휴 시간", L"Mouse idle delay"},
    {L"전체 일시정지 / 다시 시작", L"Pause all / resume"},
    {L"현재 세션에서는 동작 대기 중", L"Waiting for an active session"},
    {L"전원 유지 요청 실패", L"Power request failed"},
    {L"마우스: %s", L"Mouse: %s"},
    {L"모의 입력 제한 설정 감지", L"Synthetic input restriction detected"},
    {L"업데이트 설정", L"Update settings"},
    {L"사용 안내 / 정보", L"Help / About"},
    {L"종료", L"Exit"},
    {L"적용", L"Apply"},
    {L"대기 시간은 1~1440분의 정수를 입력하세요.", L"Enter an idle delay from 1 to 1440 minutes."},
    {L"설정 저장됨", L"Settings saved"},
    {L"적용 완료", L"Applied"},
    {L"시작 시 실행 설정을 저장하지 못했습니다. (오류 %lu)", L"Could not save startup registration. (Error %lu)"},
    {L"전원 유지 요청을 적용하지 못했습니다.", L"Could not apply the power request."},
    {L"현재 실행에는 반영했지만 설정을 저장하지 못했습니다.", L"Changes are active for this session, but settings could not be saved."},
    {L"트레이 아이콘을 등록하지 못했습니다. 잠시 후 다시 실행해 주세요.", L"Could not add the tray icon. Please try again shortly."},
    {L"OFF · 남은 일시중지 기간은 유지", L"OFF · Existing pause expiry is kept"},
    {L"%s 중지 정책 읽기 실패 (%lu)", L"Cannot read %s pause policy (%lu)"},
    {L"%s 중지 정책 값 확인 필요\nSetDisablePauseUXAccess", L"Invalid %s pause policy\nSetDisablePauseUXAccess"},
    {L"%s 사용자 일시중지 금지\nSetDisablePauseUXAccess = 1", L"%s policy prohibits user pause\nSetDisablePauseUXAccess = 1"},
    {L"%s 최대 중지 기간 확인 필요\nSetMaxPauseDays", L"Invalid %s maximum pause period\nSetMaxPauseDays"},
    {L"OS 중지 상태 미확인 · 업데이트 설정 확인", L"OS pause unconfirmed · Check Settings"},
    {L"OS 상태값: 품질·기능 업데이트 중지", L"OS reports: quality + feature paused"},
    {L"OS 상태값: 품질 중지 · 기능 확인 필요", L"OS: quality paused · Check feature"},
    {L"OS 상태값: 기능 중지 · 품질 확인 필요", L"OS: feature paused · Check quality"},
    {L"기록 기한 %04u-%02u-%02u %02u:%02u\n%s", L"Saved until %04u-%02u-%02u %02u:%02u\n%s"},
    {L"Windows 10 1903 이상 / Windows 11 필요", L"Requires Windows 10 1903+ or Windows 11"},
    {L"미적용 · 종료 후 관리자 권한으로 실행", L"Not applied · Restart as administrator"},
    {L"재부팅 대기 · 설정 앱에서 중지 필요", L"Restart pending · Pause in Settings"},
    {L"외부 날짜 변경 감지 · 자동 갱신 중단", L"External date change · Renewal stopped"},
    {L"%u일 상한 · 업데이트 설정 확인 필요", L"%u-day limit · Check update settings"},
    {L"미적용 · 기존 날짜 확인 필요 (Windows 설정)", L"Not applied · Check existing pause dates"},
    {L"미적용 · 설정 기록 실패 (오류 %lu)", L"Not applied · Date write failed (%lu)"},
    {L"Awake Mini 설정", L"Awake Mini settings"},
    {L"절전 방지 · 무기한 활성 상태 유지", L"Keep PC awake indefinitely"},
    {L"마우스 이동 전 대기 시간", L"Mouse idle delay"},
    {L"분", L"min"},
    {L"업데이트 일시중지 연장 (시험)", L"Pause updates (test)"},
    {L"전체 일시정지", L"Pause all"},
    {L"Windows 시작 시 실행", L"Run at sign-in"},
    {L"프로그램 종료", L"Exit program"},
    {L"최소화", L"Minimize"},
    {L"Awake Mini 1.4.0-state1\n\n트레이 더블클릭: 설정 / 우클릭: 메뉴\n절전 방지 · 화면 유지 · 마우스 유휴 입력\n검은화면 버튼 / 트레이 메뉴: 실행 · 화면의 해제 버튼: 복귀\n검은 화면에서는 절전·화면 유지·기존 간격 마우스 입력 활성화\n\nWindows 시작 시 실행: 로그인 후 일반 권한으로 실행합니다.\n업데이트 연장에는 관리자 권한이 필요합니다.\nEXE 이동 후 자동 실행을 체크하고 적용하여 경로를 갱신하세요.\n\n업데이트 연장은 시험 기능입니다.\n24시간마다 오늘+7일, 최초 중지일부터 최대 35일을 적용합니다.\n더 짧은 기간 정책이 있으면 해당 상한을 따릅니다.\nOFF / 전체 일시정지 / 종료 시 남은 중지 기간을 유지합니다.\n실제 중지 여부는 Windows 설정에서 확인하세요.\n\n언어: Windows 표시 언어 자동 선택 (한국어 / 영어).", L"Awake Mini 1.4.0-state1\n\nDouble-click tray: settings / Right-click: menu\nKeep awake · Keep display on · Idle mouse input\nBlack screen button / tray menu: start · Restore button: return\nBlack screen enables keep-awake, display-on and timed mouse input\n\nRun at sign-in uses standard user privileges.\nUpdate pause renewal requires administrator privileges.\nAfter moving the EXE, enable startup and Apply to refresh its path.\n\nUpdate pause renewal is experimental.\nEvery 24 hours: now + 7 days, capped at 35 days from pause start.\nA shorter configured pause limit takes precedence.\nOFF / Pause all / Exit leaves the existing expiry unchanged.\nCheck Windows Settings to confirm the actual pause state.\n\nLanguage follows the Windows display language (Korean / English)."},
    {L"검은 화면", L"Black screen"},
    {L"검은 화면 사용 불가 · 창 생성 실패", L"Black screen unavailable · Window creation failed"},
    {L"검은화면", L"Black screen"},
    {L"검은화면 해제", L"Show screen"},
    {L"해제", L"Restore"},
    {L"Win+B: 훅 오류 %lu · 버튼 사용 가능", L"Win+B: hook error %lu · buttons available"},
    {L"Win+B: 훅 설치됨 · 감지 %lu · 보조 입력 실패 %lu", L"Win+B: hook installed · detected %lu · mask failures %lu"},
    {L"Win+B: 훅 미설치 · 버튼 사용 가능", L"Win+B: hook not installed · buttons available"},
    {L"Win+B 훅 다시 연결", L"Reconnect Win+B hook"},
    {L"상태 시험 설정", L"State test settings"},
    {L"전체화면 알림 (시험)", L"Fullscreen notification (test)"},
    {L"미디어 재생 상태 (시험)", L"Media playing status (test)"},
    {L"상태 시험", L"State tests"},
    {L"Awake Mini 상태 시험", L"Awake Mini state tests"},
    {L"상태만 시험 · SSO 유지 여부는 별도 확인", L"Status test only; check SSO separately"},
    {L"Awake Mini · 미디어 상태 시험 (영상·소리 없음)", L"Awake Mini - Media status test (no video/audio)"},
    {L"전체화면 오류: 0x%08lX", L"Fullscreen error: 0x%08lX"},
    {L"전체화면 알림 ON", L"Fullscreen notification ON"},
    {L"미디어 오류: 0x%08lX", L"Media error: 0x%08lX"},
    {L"전체화면 시험 · Esc 또는 해제로 종료", L"Fullscreen test - Esc or Restore to exit"},
};
void am_language_init(void)
{
    korean = AM_FORCE_LANGUAGE == 1 ||
        (AM_FORCE_LANGUAGE == 0 && PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_KOREAN);
}
const WCHAR *am_text(const WCHAR *source)
{
    unsigned i;
    if (korean) return source;
    for (i = 0; i < sizeof(strings)/sizeof(strings[0]); ++i)
        if (!lstrcmpW(source, strings[i].ko)) return strings[i].en;
    return source;
}
static BOOL CALLBACK translate_child(HWND child, LPARAM param)
{
    WCHAR caption[256];
    (void)param;
    if (GetWindowTextW(child, caption, 256)) SetWindowTextW(child, am_text(caption));
    return TRUE;
}
void am_translate_window(HWND window)
{
    translate_child(window, 0);
    EnumChildWindows(window, translate_child, 0);
}
