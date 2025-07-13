#include <boss.hpp>
#include "pyzay.hpp"

#include <resource.hpp>

ZAY_DECLARE_VIEW_CLASS("PyZayView", PyZayData)

String gThisWidget;
String gNextWidget;
String gLastLocale;
sint32 gMinWindowWidth {1280};
sint32 gMinWindowHeight {720};

ZAY_VIEW_API OnCommand(CommandType type, id_share in, id_cloned_share* out)
{
    const uint64 CurMsec = Platform::Utility::CurrentTimeMsec();
    if(type == CT_Tick)
    {
        // 제이에디터에 틱전달
        if(m->mWidget->TickOnce())
            m->invalidate();
        if(CurMsec <= m->mUpdateMsec)
            m->invalidate(2);

        // 위젯변경 체크
        if(m->CheckWidget())
            m->invalidate();

        // GlueCall처리
        for(sint32 i = m->mReservedGlueCalls.Count() - 1; 0 <= i; --i)
        {
            auto& CurGlue = m->mReservedGlueCalls[i];
            if(CurGlue.mMsec <= CurMsec)
            {
                m->mWidget->GlueCall(CurGlue.mName, CurGlue.mParams);
                m->mReservedGlueCalls.SubtractionSection(i);
            }
        }
    }
    else if(type == CT_Size)
    {
        sint32s WH(in);
    }
    else if(type == CT_Activate && !boolo(in).ConstValue())
        m->clearCapture();
}

ZAY_VIEW_API OnNotify(NotifyType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == NT_Normal)
    {
        if(!String::Compare(topic, "UrlImageLoaded"))
        {
            const String Url(in);
            if(auto CurUrlImage = m->mUrlImages.Access(Url))
            {
                chars ExtNames[3] = {".png", ".jpg", ".bmp"};
                for(sint32 i = 0; i < 3; ++i)
                {
                    if(Asset::Exist(CurUrlImage->mFileTitle + ExtNames[i]))
                    {
                        CurUrlImage->mLoadedMsec = Platform::Utility::CurrentTimeMsec();
                        CurUrlImage->mImage.SetName(CurUrlImage->mFileTitle + ExtNames[i]).Load();
                        break;
                    }
                }
                if(0 < CurUrlImage->mLoadedMsec)
                {
                    const uint64 NewUpdateMsec = CurUrlImage->mLoadedMsec + CurUrlImage->mFadeMsec;
                    if(m->mUpdateMsec < NewUpdateMsec)
                        m->mUpdateMsec = NewUpdateMsec;
                    m->invalidate(2);
                }
            }
        }
    }
    else if(type == NT_ZayWidget)
    {
        if(!String::Compare(topic, "SetCursor"))
        {
            auto Cursor = (CursorRole) sint32o(in).Value();
            m->SetCursor(Cursor);
        }
    }
    else if(type == NT_WindowWeb)
    {
        if(!String::Compare(topic, "PythonLog"))
        {
            const String Text(in);
            m->mWidget->SendLog(Text);
        }
        else if(!String::Compare(topic, "PythonSet"))
        {
            const Strings Args(in);
            const String Key = Args[0];
            const String Value = Args[1];
            if(Key.Left(2) == "d.")
            {
                ZayWidgetDOM::SetValue(Key.Offset(2), Value);
                m->invalidate();
            }
        }
        else if(!String::Compare(topic, "PythonSetJson"))
        {
            const Strings Args(in);
            const String KeyHead = Args[0];
            const String JsonText = Args[1];
            if(KeyHead.Left(2) == "d.")
            {
                const Context Json(ST_Json, SO_OnlyReference, JsonText);
                ZayWidgetDOM::SetJson(Json, KeyHead.Offset(2));
                m->invalidate();
            }
        }
        else if(!String::Compare(topic, "PythonGet"))
        {
            const String Key(in);
            if(Key.Left(2) == "d.")
            {
                const String Value = ZayWidgetDOM::GetValue(Key.Offset(2)).ToText(true);
                if(out) *out = Value;
            }
        }
        else if(!String::Compare(topic, "PythonRem"))
        {
            const String KeyHead(in);
            if(KeyHead.Left(2) == "d.")
            {
                ZayWidgetDOM::RemoveVariables(KeyHead.Offset(2));
                m->invalidate();
            }
        }
        else if(!String::Compare(topic, "PythonGlueCall"))
        {
            const Strings Args(in);
            const String Name = Args[0];
            Strings Params;
            for(sint32 i = 1, iend = Args.Count(); i < iend; ++i)
                Params.AtAdding() = Args[i];
            if(m->mWidget->GlueCall(Name, Params))
                m->invalidate();
        }
        else if(!String::Compare(topic, "PythonJumpCall"))
        {
            const String Name(in);
            m->mWidget->JumpCall(Name);
            m->invalidate();
        }
    }
}

ZAY_VIEW_API OnGesture(GestureType type, sint32 x, sint32 y)
{
    static point64 OldCursorPos;
    static rect128 OldWindowRect;
    static uint64 ReleaseMsec = 0;

    if(type == GT_Moving)
        m->SetCursor(CR_Arrow);
    else if(type == GT_Pressed || type == GT_ExtendPressed || type == GT_WheelPressed)
    {
        Platform::Utility::GetCursorPos(OldCursorPos);
        OldWindowRect = Platform::GetWindowRect();
        m->clearCapture();
    }
    else if(type == GT_InDragging || type == GT_OutDragging || type == GT_ExtendDragging || type == GT_WheelDragging)
    {
        point64 CurCursorPos;
        Platform::Utility::GetCursorPos(CurCursorPos);
        if(!m->IsFullScreen())
        {
            rect128 CurWindowRect;
            CurWindowRect.l = OldWindowRect.l + CurCursorPos.x - OldCursorPos.x;
            CurWindowRect.t = OldWindowRect.t + CurCursorPos.y - OldCursorPos.y;
            CurWindowRect.r = OldWindowRect.r + CurCursorPos.x - OldCursorPos.x;
            CurWindowRect.b = OldWindowRect.b + CurCursorPos.y - OldCursorPos.y;
            Platform::SetWindowRect(CurWindowRect.l, CurWindowRect.t,
                CurWindowRect.r - CurWindowRect.l, CurWindowRect.b - CurWindowRect.t);
        }
        ReleaseMsec = 0;
        m->invalidate();
    }
    else if(type == GT_InReleased || type == GT_OutReleased || type == GT_CancelReleased || type == GT_ExtendRelease || type == GT_WheelRelease)
    {
        const uint64 CurReleaseMsec = Platform::Utility::CurrentTimeMsec();
        const bool HasDoubleClick = (CurReleaseMsec < ReleaseMsec + 300);
        if(HasDoubleClick)
        {
            if(m->IsFullScreen())
                m->SetNormalWindow();
            else m->SetFullScreen();
        }
        else ReleaseMsec = CurReleaseMsec;
    }
}

ZAY_VIEW_API OnRender(ZayPanel& panel)
{
    m->mWidget->Render(panel);
    if(!m->IsFullScreen())
        m->RenderWindowOutline(panel);
}

PyZayData::PyZayData()
{
    String DateText = __DATE__;
    String TimeText = __TIME__;
    DateText.Replace("Jan", "01"); DateText.Replace("Feb", "02"); DateText.Replace("Mar", "03");
    DateText.Replace("Apr", "04"); DateText.Replace("May", "05"); DateText.Replace("Jun", "06");
    DateText.Replace("Jul", "07"); DateText.Replace("Aug", "08"); DateText.Replace("Sep", "09");
    DateText.Replace("Oct", "10"); DateText.Replace("Nov", "11"); DateText.Replace("Dec", "12");
    const String Day = String::Format("%02d", Parser::GetInt(DateText.Middle(2, DateText.Length() - 6).Trim()));
    DateText = DateText.Right(4) + "/" + DateText.Left(2) + "/" + Day;
    ZayWidgetDOM::SetValue("program.buildtime", "'" + DateText + "_" + TimeText.Left(2) + "H'");
    ZayWidgetDOM::SetValue("program.locale", "'" + gLastLocale + "'");

    // 주기적인 Dom업데이트
    Platform::AddProcedure(PE_100MSEC,
        [](payload data)->void
        {
            auto Self = (PyZayData*) data;
            Self->ListingDomJsons();
            Self->ReloadDom();
        }, this);
}

PyZayData::~PyZayData()
{
    for(sint32 i = 0, iend = mUrlImages.Count(); i < iend; ++i)
        if(auto CurUrlImage = mUrlImages.AccessByOrder(i))
            Tasking::Release(CurUrlImage->mTasking, true);
}

bool PyZayData::IsFullScreen()
{
    return mIsFullScreen;
}

void PyZayData::SetFullScreen()
{
    if(!mIsFullScreen)
    {
        mIsFullScreen = true;
        mSavedNormalRect = Platform::GetWindowRect();

        point64 CursorPos;
        Platform::Utility::GetCursorPos(CursorPos);
        sint32 ScreenID = Platform::Utility::GetScreenID(CursorPos);
        rect128 FullScreenRect;
        Platform::Utility::GetScreenRect(FullScreenRect, ScreenID, false);
        Platform::SetWindowRect(FullScreenRect.l, FullScreenRect.t,
            FullScreenRect.r - FullScreenRect.l, FullScreenRect.b - FullScreenRect.t + 1);
        Platform::SetWindowRaise();
    }
}

void PyZayData::SetNormalWindow()
{
    if(mIsFullScreen)
    {
        mIsFullScreen = false;
        Platform::SetWindowRect(mSavedNormalRect.l, mSavedNormalRect.t,
            mSavedNormalRect.r - mSavedNormalRect.l, mSavedNormalRect.b - mSavedNormalRect.t);
        Platform::SetWindowRaise();
    }
}

void PyZayData::SetCursor(CursorRole role)
{
    if(mNowCursor != role)
    {
        mNowCursor = role;
        Platform::Utility::SetCursor(role);
        if(mNowCursor != CR_SizeVer && mNowCursor != CR_SizeHor && mNowCursor != CR_SizeBDiag && mNowCursor != CR_SizeFDiag && mNowCursor != CR_SizeAll)
        {
            mNcLeftBorder = false;
            mNcTopBorder = false;
            mNcRightBorder = false;
            mNcBottomBorder = false;
        }
    }
}

sint32 PyZayData::MoveNcLeft(const rect128& rect, sint32 addx)
{
    const sint32 NewLeft = rect.l + addx;
    return rect.r - Math::Max(gMinWindowWidth, rect.r - NewLeft);
}

sint32 PyZayData::MoveNcTop(const rect128& rect, sint32 addy)
{
    const sint32 NewTop = rect.t + addy;
    return rect.b - Math::Max(gMinWindowHeight, rect.b - NewTop);
}

sint32 PyZayData::MoveNcRight(const rect128& rect, sint32 addx)
{
    const sint32 NewRight = rect.r + addx;
    return rect.l + Math::Max(gMinWindowWidth, NewRight - rect.l);
}

sint32 PyZayData::MoveNcBottom(const rect128& rect, sint32 addy)
{
    const sint32 NewBottom = rect.b + addy;
    return rect.t + Math::Max(gMinWindowHeight, NewBottom - rect.t);
}

bool PyZayData::CheckWidget()
{
    if(gThisWidget != gNextWidget)
    {
        gThisWidget = gNextWidget;
        ClearDom();
        ListingDomJsons();
        ReloadDom();

        // 아틀라스
        const String AtlasInfoJson = String::FromAsset("atlasinfo.json");
        Context AtlasInfo(ST_Json, SO_OnlyReference, AtlasInfoJson, AtlasInfoJson.Length());
        R::SaveAtlas(AtlasInfo); // 최신정보의 병합
        R::ClearAll(false);
        R::AddAtlas("ui_atlaskey2.png", gThisWidget + ".png", AtlasInfo, 2);
        if(R::IsAtlasUpdated())
            R::RebuildAll();

        // 위젯
        delete mWidget;
        mWidget = new ZayWidget();
        InitWidget();
        mWidget->Reload("widget/" + gThisWidget + ".zay");
        mWidget->UpdateAtlas(R::PrintUpdatedAtlas(true));
        return true;
    }
    return false;
}

void PyZayData::InitWidget()
{
    auto& NewZayson = mWidget->Init("PyZayData", nullptr, [](chars name)->const Image* {return &((const Image&) R(name));});
    NewZayson.AddGlue("update", ZAY_DECLARE_GLUE(params, this) // 업데이트를 위한 글루함수를 추가
    {
        if(params.ParamCount() == 1)
        {
            auto Msec = sint32(params.Param(0).ToFloat() * 1000);
            mUpdateMsec = Platform::Utility::CurrentTimeMsec() + Msec;
        }
    })
    .AddGlue("reload", ZAY_DECLARE_GLUE(params, this)
    {
        ClearDom();
        ListingDomJsons();
        ReloadDom();
        invalidate();
    })
    .AddGlue("setlocale", ZAY_DECLARE_GLUE(params, this)
    {
        if(params.ParamCount() == 1)
        {
            gLastLocale = params.Param(0).ToText();
            ZayWidgetDOM::SetValue("program.locale", "'" + gLastLocale + "'");
        }
    })
    .AddGlue("setvalue", ZAY_DECLARE_GLUE(params, this)
    {
        if(params.ParamCount() == 2)
        {
            auto DomName = params.Param(0).ToText();
            auto Value = params.Param(1).ToText();
            if(DomName.Left(2) == "d.")
            {
                ZayWidgetDOM::SetValue(DomName.Offset(2), Value);
                invalidate();
            }
        }
    })
    .AddGlue("getcomment", ZAY_DECLARE_GLUE(params, this)
    {
        if(params.ParamCount() == 1)
        {
            auto DomName = params.Param(0).ToText();
            if(DomName.Left(2) == "d." && params.CanReturn())
            {
                auto Value = ZayWidgetDOM::GetComment(DomName.Offset(2));
                params.Return(Value);
            }
        }
    })
    .AddGlue("clearvalue", ZAY_DECLARE_GLUE(params, this)
    {
        if(params.ParamCount() == 1)
        {
            auto DomName = params.Param(0).ToText();
            if(DomName.Left(2) == "d.")
            {
                ZayWidgetDOM::RemoveVariables(DomName.Offset(2));
                invalidate();
            }
        }
    })
    .AddGlue("setdom", ZAY_DECLARE_GLUE(params, this)
    {
        if(2 <= params.ParamCount())
        {
            auto DomName = params.Param(0).ToText();
            auto FilePath = params.Param(1).ToText();
            if(DomName.Left(2) == "d.")
            {
                String DomHeader = DomName.Offset(2) + '.';
                ZayWidgetDOM::RemoveVariables(DomHeader);
                if(Platform::File::Exist(FilePath))
                {
                    const Context Json(ST_Json, SO_NeedCopy, String::FromFile(FilePath));
                    ZayWidgetDOM::SetJson(Json, DomHeader);
                }
                else if(2 < params.ParamCount())
                {
                    auto AssetPath = params.Param(2).ToText();
                    const Context Json(ST_Json, SO_NeedCopy, String::FromAsset(AssetPath));
                    ZayWidgetDOM::SetJson(Json, DomHeader);
                }
                invalidate();
            }
        }
    })
    .AddGlue("change", ZAY_DECLARE_GLUE(params, this)
    {
        if(params.ParamCount() == 1)
        {
            gNextWidget = params.Param(0).ToText();
            invalidate();
        }
    });

    ////////////////////////////////////////////////////////////////////////////////
    // user_content
    NewZayson.AddComponent(ZayExtend::ComponentType::ContentWithParameter, "user_content", ZAY_DECLARE_COMPONENT(panel, params, this)
    {
        if(params.ParamCount() < 1)
            return panel._push_pass();
        const String Type = params.Param(0).ToText();
        bool HasRender = false;

        branch;
        jump(!Type.Compare("clearbg"))
        {
            if(params.ParamCount() == 2)
            {
                const sint32 Round = params.Param(1).ToInteger();
                HasRender = RenderUC_ClearBG(panel, Round);
            }
        }
        jump(!Type.Compare("urlimage"))
        {
            if(params.ParamCount() == 3)
            {
                const double FadeSec = params.Param(1).ToFloat();
                const String Url = params.Param(2).ToText();
                HasRender = RenderUC_UrlImage(panel, FadeSec, Url);
            }
        }
        jump(!Type.Compare("dragcollector"))
        {
            if(params.ParamCount() == 4)
            {
                auto UIName = params.Param(1).ToText();
                auto DomName = params.Param(2).ToText();
                auto MinGap = params.Param(3).ToInteger();
                if(DomName.Left(2) == "d.")
                {
                    const String DomHeader = DomName.Offset(2) + '.';
                    HasRender = RenderUC_DragCollector(panel, UIName, DomHeader, MinGap);
                }
            }
        }

        // 그외 처리
        if(!HasRender)
        ZAY_INNER_SCISSOR(panel, 0)
        {
            ZAY_RGBA(panel, 255, 0, 0, 128)
            panel.fill();
            for(sint32 i = 0; i < 5; ++i)
            {
                ZAY_INNER(panel, 1 + i)
                ZAY_RGBA(panel, 255, 0, 0, 128 - 16 * i)
                panel.rect(1);
            }
            ZAY_FONT(panel, 1.2 * Math::MinF(Math::MinF(panel.w(), panel.h()) / 40, 1))
            ZAY_RGB(panel, 255, 0, 0)
            panel.text(Type, UIFA_CenterMiddle, UIFE_Right);
        }
        return panel._push_pass();
    });
}

void PyZayData::ListingDomJsons()
{
    uint64 DataModifyMsec = 0;
    if(Asset::Exist("widget/" + gThisWidget + ".json", nullptr, nullptr, nullptr, nullptr, &DataModifyMsec))
    if(!mDomJsons.Access(gThisWidget + ".json") || mDomJsons(gThisWidget + ".json").mModifiedMsec < DataModifyMsec)
    {
        hook(mDomJsons(gThisWidget + ".json"))
        {
            fish.mAssetName = gThisWidget + ".json";
            fish.mHeaderName = "data.";
            fish.mModifiedMsec = DataModifyMsec;
            fish.mUpdated = true;
        }
    }

    auto DomFinder = [](chars itemname, payload data)->void
    {
        auto Self = (PyZayData*) data;
        const String FileName = itemname;
        const sint32 HeadLength = gThisWidget.Length() + 1;
        if(!FileName.Left(HeadLength).CompareNoCase(gThisWidget + "_"))
        if(!FileName.Right(5).CompareNoCase(".json"))
        {
            uint64 DataModifyMsec = 0;
            if(Asset::Exist("widget/" + FileName, nullptr, nullptr, nullptr, nullptr, &DataModifyMsec))
            if(!Self->mDomJsons.Access(FileName) || Self->mDomJsons(FileName).mModifiedMsec < DataModifyMsec)
            {
                hook(Self->mDomJsons(FileName))
                {
                    fish.mAssetName = FileName;
                    fish.mHeaderName = FileName.Middle(HeadLength, FileName.Length() - (HeadLength + 5)) + ".";
                    fish.mModifiedMsec = DataModifyMsec;
                    fish.mUpdated = true;
                }
            }
        }
    };
    Platform::File::Search(Platform::File::RootForAssets() + "widget", DomFinder, this, false);
    Platform::File::Search(Platform::File::RootForAssetsRem() + "widget", DomFinder, this, false);
}

void PyZayData::ReloadDom()
{
    for(sint32 i = 0, iend = mDomJsons.Count(); i < iend; ++i)
    {
        if(auto CurDomJson = mDomJsons.AccessByOrder(i))
        {
            if(!CurDomJson->mUpdated) continue;
            const String DataJson = String::FromAsset("widget/" + CurDomJson->mAssetName);
            Context Data(ST_Json, SO_OnlyReference, DataJson, DataJson.Length());
            ZayWidgetDOM::RemoveVariables(CurDomJson->mHeaderName);
            ZayWidgetDOM::SetJson(Data, CurDomJson->mHeaderName);
            CurDomJson->mUpdated = false;

            if(CurDomJson->mHeaderName == "data.") // 폰트확보
            {
                Platform::Utility::RemoveSystemFontAll();
                ZayWidgetDOM::RemoveVariables("program.fonts.");
                Strings Collector;
                for(sint32 j = 0, jend = ZayWidgetDOM::GetValue("data.fonts.count").ToInteger(); j < jend; ++j)
                {
                    const String CurHeader = String::Format("data.fonts.%d.", j);
                    const String CurPath = ZayWidgetDOM::GetValue(CurHeader + "path").ToText();
                    if(0 < CurPath.Length())
                    {
                        buffer NewFontData = Asset::ToBuffer(CurPath);
                        auto FontFamilies = Platform::Utility::CreateSystemFont((bytes) NewFontData, Buffer::CountOf(NewFontData));
                        for(sint32 i = 0, iend = FontFamilies.Count(); i < iend; ++i)
                            Collector.AtAdding() = FontFamilies[i];
                        Buffer::Free(NewFontData);
                    }
                }
                for(sint32 j = 0, jend = Collector.Count(); j < jend; ++j)
                {
                    auto FontStyles = Platform::Utility::EnumSystemFontStyles(Collector[j]);
                    if(FontStyles.Count() == 0)
                        ZayWidgetDOM::SetValue(String::Format("program.fonts.%s", (chars) Collector[j]), "'<no style>'");
                    else for(sint32 k = 0, kend = FontStyles.Count(); k < kend; ++k)
                        ZayWidgetDOM::SetValue(String::Format("program.fonts.%s.%d", (chars) Collector[j], k), "'" + FontStyles[k] + "'");
                }
            }
        }
    }
}

void PyZayData::ClearDom()
{
    for(sint32 i = 0, iend = mDomJsons.Count(); i < iend; ++i)
        ZayWidgetDOM::RemoveVariables(mDomJsons[i].mHeaderName);
    mDomJsons.Reset();
}

void PyZayData::RenderWindowOutline(ZayPanel& panel)
{
    ZAY_INNER(panel, 1)
    ZAY_RGBA(panel, 0, 0, 0, 32)
        panel.rect(1);

    // 리사이징바
    ZAY_RGBA(panel, 0, 0, 0, 64 + 128 * Math::Abs(((frame() * 2) % 100) - 50) / 50)
    {
        if(mNcLeftBorder)
        {
            for(sint32 i = 0; i < 5; ++i)
            ZAY_RGBA(panel, 128, 128, 128, 128 - i * 25)
            ZAY_XYWH(panel, i, 0, 1, panel.h())
                panel.fill();
            invalidate(2);
        }
        if(mNcTopBorder)
        {
            for(sint32 i = 0; i < 5; ++i)
            ZAY_RGBA(panel, 128, 128, 128, 128 - i * 25)
            ZAY_XYWH(panel, 0, i, panel.w(), 1)
                panel.fill();
            invalidate(2);
        }
        if(mNcRightBorder)
        {
            for(sint32 i = 0; i < 5; ++i)
            ZAY_RGBA(panel, 128, 128, 128, 128 - i * 25)
            ZAY_XYWH(panel, panel.w() - 1 - i, 0, 1, panel.h())
                panel.fill();
            invalidate(2);
        }
        if(mNcBottomBorder)
        {
            for(sint32 i = 0; i < 5; ++i)
            ZAY_RGBA(panel, 128, 128, 128, 128 - i * 25)
            ZAY_XYWH(panel, 0, panel.h() - 1 - i, panel.w(), 1)
                panel.fill();
            invalidate(2);
        }
    }

    // 윈도우 리사이징 모듈
    #define RESIZING_MODULE(C, L, T, R, B, BORDER) do {\
        static point64 OldMousePos; \
        static rect128 OldWindowRect; \
        static bool DraggingRaised = false; \
        if(t == GT_Moving) \
        { \
            SetCursor(C); \
            mNcLeftBorder = false; \
            mNcTopBorder = false; \
            mNcRightBorder = false; \
            mNcBottomBorder = false; \
            BORDER = true; \
        } \
        else if(t == GT_MovingLosed) \
        { \
            SetCursor(CR_Arrow); \
        } \
        else if(t == GT_Pressed) \
        { \
            Platform::Utility::GetCursorPos(OldMousePos); \
            OldWindowRect = Platform::GetWindowRect(); \
            DraggingRaised = false; \
        } \
        else if(t == GT_InDragging || t == GT_OutDragging || t == GT_InReleased || t == GT_OutReleased || t == GT_CancelReleased) \
        { \
            point64 NewMousePos; \
            Platform::Utility::GetCursorPos(NewMousePos); \
            const rect128 NewWindowRect = {L, T, R, B}; \
            Platform::SetWindowRect(NewWindowRect.l, NewWindowRect.t, \
                NewWindowRect.r - NewWindowRect.l, NewWindowRect.b - NewWindowRect.t); \
            if(!DraggingRaised) \
            { \
                DraggingRaised = true; \
                Platform::SetWindowRaise(); \
            } \
        }} while(false);

    // 윈도우 리사이징 꼭지점
    const sint32 SizeBorder = 15;
    ZAY_LTRB_UI(panel, 0, 0, SizeBorder, SizeBorder, "NcLeftTop",
        ZAY_GESTURE_T(t, this)
        {
            RESIZING_MODULE(CR_SizeFDiag,
                MoveNcLeft(OldWindowRect, NewMousePos.x - OldMousePos.x),
                MoveNcTop(OldWindowRect, NewMousePos.y - OldMousePos.y),
                OldWindowRect.r,
                OldWindowRect.b,
                mNcLeftBorder = mNcTopBorder);
        });
    ZAY_LTRB_UI(panel, panel.w() - SizeBorder, 0, panel.w(), SizeBorder, "NcRightTop",
        ZAY_GESTURE_T(t, this)
        {
            RESIZING_MODULE(CR_SizeBDiag,
                OldWindowRect.l,
                MoveNcTop(OldWindowRect, NewMousePos.y - OldMousePos.y),
                MoveNcRight(OldWindowRect, NewMousePos.x - OldMousePos.x),
                OldWindowRect.b,
                mNcRightBorder = mNcTopBorder);
        });
    ZAY_LTRB_UI(panel, 0, panel.h() - SizeBorder, SizeBorder, panel.h(), "NcLeftBottom",
        ZAY_GESTURE_T(t, this)
        {
            RESIZING_MODULE(CR_SizeBDiag,
                MoveNcLeft(OldWindowRect, NewMousePos.x - OldMousePos.x),
                OldWindowRect.t,
                OldWindowRect.r,
                MoveNcBottom(OldWindowRect, NewMousePos.y - OldMousePos.y),
                mNcLeftBorder = mNcBottomBorder);
        });
    ZAY_LTRB_UI(panel, panel.w() - SizeBorder, panel.h() - SizeBorder, panel.w(), panel.h(), "NcRightBottom",
        ZAY_GESTURE_T(t, this)
        {
            RESIZING_MODULE(CR_SizeFDiag,
                OldWindowRect.l,
                OldWindowRect.t,
                MoveNcRight(OldWindowRect, NewMousePos.x - OldMousePos.x),
                MoveNcBottom(OldWindowRect, NewMousePos.y - OldMousePos.y),
                mNcRightBorder = mNcBottomBorder);
        });

    // 윈도우 리사이징 모서리
    ZAY_LTRB_UI(panel, 0, SizeBorder, SizeBorder, panel.h() - SizeBorder, "NcLeft",
        ZAY_GESTURE_T(t, this)
        {
            RESIZING_MODULE(CR_SizeHor,
                MoveNcLeft(OldWindowRect, NewMousePos.x - OldMousePos.x),
                OldWindowRect.t,
                OldWindowRect.r,
                OldWindowRect.b,
                mNcLeftBorder);
        });
    ZAY_LTRB_UI(panel, SizeBorder, 0, panel.w() - SizeBorder, SizeBorder, "NcTop",
        ZAY_GESTURE_T(t, this)
        {
            RESIZING_MODULE(CR_SizeVer,
                OldWindowRect.l,
                MoveNcTop(OldWindowRect, NewMousePos.y - OldMousePos.y),
                OldWindowRect.r,
                OldWindowRect.b,
                mNcTopBorder);
        });
    ZAY_LTRB_UI(panel, panel.w() - SizeBorder, SizeBorder, panel.w(), panel.h() - SizeBorder, "NcRight",
        ZAY_GESTURE_T(t, this)
        {
            RESIZING_MODULE(CR_SizeHor,
                OldWindowRect.l,
                OldWindowRect.t,
                MoveNcRight(OldWindowRect, NewMousePos.x - OldMousePos.x),
                OldWindowRect.b,
                mNcRightBorder);
        });
    ZAY_LTRB_UI(panel, SizeBorder, panel.h() - SizeBorder, panel.w() - SizeBorder, panel.h(), "NcBottom",
        ZAY_GESTURE_T(t, this)
        {
            RESIZING_MODULE(CR_SizeVer,
                OldWindowRect.l,
                OldWindowRect.t,
                OldWindowRect.r,
                MoveNcBottom(OldWindowRect, NewMousePos.y - OldMousePos.y),
                mNcBottomBorder);
        });
}

bool PyZayData::RenderUC_ClearBG(ZayPanel& panel, sint32 round)
{
    panel.erase(round);
    return true;
}

bool PyZayData::RenderUC_UrlImage(ZayPanel& panel, double fadesec, chars url)
{
    // 최초시 로딩처리
    auto& CurUrlImage = mUrlImages(url);
    if(CurUrlImage.mFileTitle.Length() == 0)
    {
        const String Url = url;
        CurUrlImage.mFileTitle = String::Format("urlimage/%s", AddOn::Ssl::ToSHA256((bytes)(chars) Url, Url.Length(), false));
        CurUrlImage.mFadeMsec = Math::Max(0, 1000 * fadesec);
        if(Asset::Exist(CurUrlImage.mFileTitle + ".png"))
            CurUrlImage.mImage.SetName(CurUrlImage.mFileTitle + ".png").Load();
        else if(Asset::Exist(CurUrlImage.mFileTitle + ".jpg"))
            CurUrlImage.mImage.SetName(CurUrlImage.mFileTitle + ".jpg").Load();
        else if(Asset::Exist(CurUrlImage.mFileTitle + ".bmp"))
            CurUrlImage.mImage.SetName(CurUrlImage.mFileTitle + ".bmp").Load();
        else CurUrlImage.mTasking = Tasking::Create(
            [](buffer& self, Queue<buffer>& query, Queue<buffer>& answer, id_common common)->sint32
            {
                const String Args = String::FromBuffer(self);
                self = nullptr;
                auto ColonPos = Args.Find(0, ":");
                String FileName = Args.Left(ColonPos);
                const String Url = Args.Offset(ColonPos + 1);
                if(id_curl NewCurl = AddOn::Curl::Create())
                {
                    sint32 FileSize = 0;
                    bytes FileData = AddOn::Curl::GetBytes(NewCurl, Url, &FileSize);
                    if(FileData[0] == 0x89 && FileData[1] == 0x50 && FileData[2] == 0x4E && FileData[3] == 0x47)
                        FileName += ".png";
                    else if(FileData[0] == 0xFF && FileData[1] == 0xD8 && FileData[2] == 0xFF)
                        FileName += ".jpg";
                    else if(FileData[0] == 0x42 && FileData[1] == 0x4D)
                        FileName += ".bmp";
                    if(auto NewAsset = Asset::OpenForWrite(FileName, true))
                    {
                        Asset::Write(NewAsset, FileData, FileSize);
                        Asset::Close(NewAsset);
                        Platform::BroadcastNotify("UrlImageLoaded", Url);
                    }
                    AddOn::Curl::Release(NewCurl);
                }
                return -1;
            }, (CurUrlImage.mFileTitle + ':' + Url).ToBuffer());
    }

    // 랜더링
    if(CurUrlImage.mImage.HasBitmap())
    {
        if(0 < CurUrlImage.mLoadedMsec)
        {
            const sint32 Opacity = (CurUrlImage.mFadeMsec == 0)? 255 :
                Math::Min(255 * (Platform::Utility::CurrentTimeMsec() - CurUrlImage.mLoadedMsec) / CurUrlImage.mFadeMsec, 255);
            ZAY_RGBA(panel, 128, 128, 128, Opacity)
                panel.stretch(CurUrlImage.mImage, Image::Build::Force);
            if(Opacity == 255)
                CurUrlImage.mLoadedMsec = 0;
        }
        else panel.stretch(CurUrlImage.mImage, Image::Build::Force);
    }
    return true;
}

bool PyZayData::RenderUC_DragCollector(ZayPanel& panel, chars uiname, chars domheader, sint32 mingap)
{
    const String DomHeader = domheader;
    ZAY_INNER_UI(panel, 0, uiname,
        ZAY_GESTURE_VNTXY(v, n, t, x, y, DomHeader, mingap)
        {
            static bool Draging = false;
            static sint32 LineCount = 0;
            if(t == GT_Pressed || t == GT_InDragging || t == GT_OutDragging)
            {
                static sint32 OldX, OldY;
                static sint32 DotCount;
                static uint64 DotMsec;
                if(t == GT_Pressed)
                {
                    Draging = true;
                    LineCount++;
                    OldX = x + mingap;
                    OldY = y;
                    DotCount = 0;
                    DotMsec = Platform::Utility::CurrentTimeMsec();
                    ZayWidgetDOM::SetValue(DomHeader + "line.count", String::FromInteger(LineCount));
                    ZayWidgetDOM::SetValue(DomHeader + "mode", "'draging'");
                }
                if(Draging && mingap <= Math::Distance(OldX, OldY, x, y))
                {
                    OldX = x;
                    OldY = y;
                    DotCount++;
                    const auto& CurRect = v->rect(n);
                    const sint32 CurDotMsec = Platform::Utility::CurrentTimeMsec() - DotMsec;
                    const String DotHeader = DomHeader + String::Format("line.%d.dot.", LineCount - 1);
                    ZayWidgetDOM::SetValue(DotHeader + "count", String::FromInteger(DotCount));
                    ZayWidgetDOM::SetValue(DotHeader + String::Format("%d.x", DotCount - 1), String::FromInteger(x - CurRect.l));
                    ZayWidgetDOM::SetValue(DotHeader + String::Format("%d.y", DotCount - 1), String::FromInteger(y - CurRect.t));
                    ZayWidgetDOM::SetValue(DotHeader + String::Format("%d.msec", DotCount - 1), String::FromInteger(CurDotMsec));
                    v->invalidate();
                }
            }
            else if(t == GT_InReleased || t == GT_OutReleased || t == GT_CancelReleased)
            {
                Draging = false;
                ZayWidgetDOM::SetValue(DomHeader + "mode", "'done'");
            }
            else if(t == GT_ExtendPressed)
            {
                Draging = false;
                LineCount = 0;
                ZayWidgetDOM::RemoveVariables(DomHeader);
                v->invalidate();
            }
        });
    return true;
}
