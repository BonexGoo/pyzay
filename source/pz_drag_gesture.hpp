#pragma once
#include <boss.hpp>

class PzDragGesture
{
public:
    PzDragGesture() {}
    ~PzDragGesture() {}
    BOSS_DECLARE_NONCOPYABLE_CLASS(PzDragGesture)

public:
    void LoadJson(chars json);
    String SaveJson() const;
    void UpdateDom(chars domheader) const;

public:
    struct Dot
    {
        sint32 mX;
        sint32 mY;
        double mSpeed;
    };
    typedef Array<Dot> Shape;
    Array<Shape> mShapes;
};
typedef Map<PzDragGesture> PzDragGestures;
