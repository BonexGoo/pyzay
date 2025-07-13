#pragma once
#include <service/boss_zaywidget.hpp>

class DomJson
{
public:
    String mAssetName;
    String mHeaderName;
    uint64 mModifiedMsec {0};
    bool mUpdated {false};
};
typedef Map<DomJson> DomJsons;

class GlueCall
{
public:
    GlueCall() {}
    ~GlueCall() {}
    BOSS_DECLARE_NONCOPYABLE_CLASS(GlueCall)

public:
    String mName;
    Strings mParams;
    uint64 mMsec {0};
    GlueCall& operator=(GlueCall&& rhs)
    {
        mName = ToReference(rhs.mName);
        mParams = ToReference(rhs.mParams);
        mMsec = rhs.mMsec; rhs.mMsec = 0;
        return *this;
    }
};
typedef Array<GlueCall> GlueCalls;

class UrlImage
{
public:
    UrlImage() {}
    ~UrlImage() {}
    BOSS_DECLARE_NONCOPYABLE_CLASS(UrlImage)

public:
    String mFileTitle;
    uint64 mFadeMsec {0};
    uint64 mLoadedMsec {0};
    Image mImage;
    id_tasking mTasking {nullptr};
};
typedef Map<UrlImage> UrlImages;

class PyZayData : public ZayObject
{
public:
    PyZayData();
    ~PyZayData();

public:
    bool IsFullScreen();
    void SetFullScreen();
    void SetNormalWindow();
    void SetCursor(CursorRole role);
    sint32 MoveNcLeft(const rect128& rect, sint32 addx);
    sint32 MoveNcTop(const rect128& rect, sint32 addy);
    sint32 MoveNcRight(const rect128& rect, sint32 addx);
    sint32 MoveNcBottom(const rect128& rect, sint32 addy);
    bool CheckWidget();
    void InitWidget();
    void ListingDomJsons();
    void ReloadDom();
    void ClearDom();
    void RenderWindowOutline(ZayPanel& panel);
    bool RenderUC_ClearBG(ZayPanel& panel, sint32 round);
    bool RenderUC_UrlImage(ZayPanel& panel, double fadesec, chars url);
    bool RenderUC_DragCollector(ZayPanel& panel, chars uiname, chars domheader, sint32 mingap);

public:
    ZayWidget* mWidget {new ZayWidget()};
    uint64 mUpdateMsec {0};
    bool mNcLeftBorder {false};
    bool mNcTopBorder {false};
    bool mNcRightBorder {false};
    bool mNcBottomBorder {false};
    bool mIsFullScreen {false};
    rect128 mSavedNormalRect {0, 0, 0, 0};
    CursorRole mNowCursor {CR_Arrow};
    DomJsons mDomJsons; // [filename]
    UrlImages mUrlImages; // [url]
    GlueCalls mReservedGlueCalls;
};
