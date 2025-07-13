#include <boss.hpp>

#include <resource.hpp>
#include <platform/boss_platform.hpp>
#include <service/boss_zay.hpp>

extern String gThisWidget;
extern String gNextWidget;
extern String gLastLocale;
extern sint32 gMinWindowWidth;
extern sint32 gMinWindowHeight;

bool PlatformInit()
{
    #if BOSS_WASM
        Platform::InitForMDI(true, false, nullptr);
    #else
        Platform::InitForMDI(true, false, nullptr,
            Platform::File::RootForAssets() + "webpython/pyodide.html");
    #endif

    Platform::SetViewCreator(ZayView::Creator);
    Platform::SetWindowName("pyzay");

    // 윈도우 위치설정
    String WindowInfoString = String::FromAsset("windowinfo.json");
    Context WindowInfo(ST_Json, SO_OnlyReference, WindowInfoString, WindowInfoString.Length());
    gNextWidget = WindowInfo("widget").GetText("sign");
    gLastLocale = WindowInfo("locale").GetText(Platform::Utility::GetLocaleBCP47());
    const sint32 ScreenID = WindowInfo("screen").GetInt(0);
    rect128 ScreenRect = {};
    Platform::Utility::GetScreenRect(ScreenRect, ScreenID);
    const sint32 ScreenWidth = ScreenRect.r - ScreenRect.l;
    const sint32 ScreenHeight = ScreenRect.b - ScreenRect.t;
    const sint32 WindowWidth = WindowInfo("w").GetInt(gMinWindowWidth);
    const sint32 WindowHeight = WindowInfo("h").GetInt(gMinWindowHeight);
    const sint32 WindowX = Math::Clamp(WindowInfo("x").GetInt((ScreenWidth - WindowWidth) / 2), 0, ScreenWidth - WindowWidth);
    const sint32 WindowY = Math::Clamp(WindowInfo("y").GetInt((ScreenHeight - WindowHeight) / 2), 0, ScreenHeight - WindowHeight);
    Platform::SetWindowRect(ScreenRect.l + WindowX, ScreenRect.t + WindowY, WindowWidth, WindowHeight);

    // 아틀라스 동적로딩
    R::SetAtlasDir("image");
    Platform::AddProcedure(PE_100MSEC,
        [](payload data)->void
        {
            if(R::IsAtlasUpdated())
            {
                R::RebuildAll();
                Platform::UpdateAllViews();
            }
        });

    Platform::SetWindowView("PyZayView");
    return true;
}

void PlatformQuit()
{
    // 윈도우
    const rect128 WindowRect = Platform::GetWindowRect(true);
    const sint32 ScreenID = Platform::Utility::GetScreenID(
        {(WindowRect.l + WindowRect.r) / 2, (WindowRect.t + WindowRect.b) / 2});
    rect128 ScreenRect = {};
    Platform::Utility::GetScreenRect(ScreenRect, ScreenID);
    Context WindowInfo;
    WindowInfo.At("widget").Set(gThisWidget);
    WindowInfo.At("locale").Set(gLastLocale);
    WindowInfo.At("screen").Set(String::FromInteger(ScreenID));
    WindowInfo.At("x").Set(String::FromInteger(WindowRect.l - ScreenRect.l));
    WindowInfo.At("y").Set(String::FromInteger(WindowRect.t - ScreenRect.t));
    WindowInfo.At("w").Set(String::FromInteger(WindowRect.r - WindowRect.l));
    WindowInfo.At("h").Set(String::FromInteger(WindowRect.b - WindowRect.t));
    WindowInfo.SaveJson().ToAsset("windowinfo.json", true);

    // 아틀라스
    Context AtlasInfo;
    R::SaveAtlas(AtlasInfo);
    AtlasInfo.SaveJson().ToAsset("atlasinfo.json", true);
}

void PlatformFree()
{
}
