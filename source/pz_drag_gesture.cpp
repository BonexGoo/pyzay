#include <boss.hpp>
#include "pz_drag_gesture.hpp"

#include <service/boss_zaywidget.hpp>

void PzDragGesture::LoadJson(chars json)
{
    const Context Json(ST_Json, SO_OnlyReference, json);
    mShapes.Clear();
    for(sint32 i = 0, iend = Json("line").LengthOfIndexable(); i < iend; ++i)
    {
        auto& NewLine = mShapes.AtAdding();
        for(sint32 j = 0, jend = Json("line")[i]("dot").LengthOfIndexable(); j < jend; ++j)
        {
            auto& NewDot = NewLine.AtAdding();
            NewDot.mX = Json("line")[i]("dot")[j]("x").GetInt();
            NewDot.mY = Json("line")[i]("dot")[j]("y").GetInt();
            NewDot.mSpeed = Json("line")[i]("dot")[j]("speed").GetFloat();
        }
    }
}

String PzDragGesture::SaveJson() const
{
    Context Json;
    for(sint32 i = 0, iend = mShapes.Count(); i < iend; ++i)
    {
        auto& NewDotGroup = Json.At("line").AtAdding().At("dot");
        for(sint32 j = 0, jend = mShapes[i].Count(); j < jend; ++j)
        {
            const auto& CurDot = mShapes[i][j];
            auto& NewDot = NewDotGroup.AtAdding();
            NewDot.At("x").Set(String::FromInteger(CurDot.mX));
            NewDot.At("y").Set(String::FromInteger(CurDot.mY));
            NewDot.At("speed").Set(String::FromFloat(CurDot.mSpeed));
        }
    }
    return Json.SaveJson();
}

void PzDragGesture::UpdateDom(chars domheader) const
{
    const sint32 ShapeCount = mShapes.Count();
    for(sint32 i = 0; i < ShapeCount; ++i)
    {
        const String DotHeader = String::Format("%sline.%d.dot.", domheader, i);
        const sint32 DotCount = mShapes[i].Count();
        for(sint32 j = 0; j < DotCount; ++j)
        {
            const auto& CurDot = mShapes[i][j];
            ZayWidgetDOM::SetValue(DotHeader + String::Format("%d.x", j), String::FromInteger(CurDot.mX));
            ZayWidgetDOM::SetValue(DotHeader + String::Format("%d.y", j), String::FromInteger(CurDot.mY));
            ZayWidgetDOM::SetValue(DotHeader + String::Format("%d.speed", j), String::FromFloat(CurDot.mSpeed));
        }
        ZayWidgetDOM::SetValue(DotHeader + "count", String::FromInteger(DotCount));
    }
    ZayWidgetDOM::SetValue(String::Format("%sline.count", domheader), String::FromInteger(ShapeCount));
}
