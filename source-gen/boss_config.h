﻿#pragma once

////////////////////////////////////////
// Platform (select one or not)
////////////////////////////////////////
#define BOSS_PLATFORM_QT6

////////////////////////////////////////
// AddOn
////////////////////////////////////////
#define BOSS_NEED_ADDON_AAC        0
#define BOSS_NEED_ADDON_ABC        0
#define BOSS_NEED_ADDON_ALPR       0
#define BOSS_NEED_ADDON_CURL       1
#define BOSS_NEED_ADDON_DLIB       0
#define BOSS_NEED_ADDON_FREETYPE   0
#define BOSS_NEED_ADDON_GIT        0
#define BOSS_NEED_ADDON_H264       0
#define BOSS_NEED_ADDON_JPG        1
#define BOSS_NEED_ADDON_OGG        0
#define BOSS_NEED_ADDON_OPENCV     0
#define BOSS_NEED_ADDON_PSD        0
#define BOSS_NEED_ADDON_SSL        1
#define BOSS_NEED_ADDON_TENSORFLOW 0
#define BOSS_NEED_ADDON_TESSERACT  0
#define BOSS_NEED_ADDON_TIF        0
#define BOSS_NEED_ADDON_WEBP       0
#define BOSS_NEED_ADDON_WEBRTC     0
#define BOSS_NEED_ADDON_WEBSOCKET  0
#define BOSS_NEED_ADDON_ZIP        0
#define BOSS_NEED_ADDON_ZIPA       0

////////////////////////////////////////
// Detail
////////////////////////////////////////
#define BOSS_NEED_FORCED_FAKEWIN   0
#define BOSS_NEED_FULLSCREEN       (BOSS_IPHONE | BOSS_ANDROID)
#define BOSS_NEED_MAIN             1
#define BOSS_NEED_PROFILE          0
#define BOSS_NEED_RELEASE_TRACE    1
#define BOSS_NEED_NATIVE_OPENGL    0
#define BOSS_NEED_CEF_WEBVIEW      1
#define BOSS_NEED_EMBEDDED_ASSET   (BOSS_WASM)
#define BOSS_NEED_EMBEDDED_CACHE   (BOSS_WASM)
#define BOSS_NEED_BUILD_RESIZING   0
#define BOSS_NEED_BUILD_COLORING   0
#define BOSS_ASSET_IGNORES         {".json.pipe", ".json.old", ".json.f5", "etc"}
#define BOSS_SUMMIT_DOMAIN         "summit.zaylab.com:8109"
