// CollisionRects.h
#pragma once
#include <raylib.h>
#include <raygui.h>
#include <array>
#include <functional>

namespace Collision {

// Helper: Check if point P is inside triangle ABC
inline bool PointInTriangle(const Vector2 &P, const Vector2 &A, const Vector2 &B, const Vector2 &C) {
    float s = A.y * C.x - A.x * C.y + (C.y - A.y) * P.x + (A.x - C.x) * P.y;
    float t = A.x * B.y - A.y * B.x + (A.y - B.y) * P.x + (B.x - A.x) * P.y;

    if ((s < 0) != (t < 0)) return false;

    float area = -B.y * C.x + A.y * (C.x - B.x) + A.x * (B.y - C.y) + B.x * C.y;
    return area < 0 ? (s <= 0 && s + t >= area) : (s >= 0 && s + t <= area);
}

// Check collision between two rectangles represented by 4 corners each
inline bool CheckCollisionRectCorners(const std::array<Vector2, 4> &rect1, const std::array<Vector2, 4> &rect2) {
    // Split each rectangle into two triangles
    std::array<std::array<Vector2, 3>, 2> tris1 = {{{rect1[0], rect1[1], rect1[2]}, {rect1[0], rect1[2], rect1[3]}}};
    std::array<std::array<Vector2, 3>, 2> tris2 = {{{rect2[0], rect2[1], rect2[2]}, {rect2[0], rect2[2], rect2[3]}}};

    // Check all triangle pairs
    for (auto &t1 : tris1) {
        for (auto &t2 : tris2) {
            // Check if any vertex of t1 is inside t2
            for (const auto &v : t1) {
                if (PointInTriangle(v, t2[0], t2[1], t2[2])) return true;
            }
            // Check if any vertex of t2 is inside t1
            for (const auto &v : t2) {
                if (PointInTriangle(v, t1[0], t1[1], t1[2])) return true;
            }
        }
    }

    return false;
}

inline bool CheckCollisionRectCornersRec(const Rectangle &rect, const std::array<Vector2, 4> &rotated) {
    // Axis-aligned rectangle corners
    std::array<Vector2, 4> rectCorners = {{
        {rect.x, rect.y},
        {rect.x + rect.width, rect.y},
        {rect.x + rect.width, rect.y + rect.height},
        {rect.x, rect.y + rect.height}
    }};

    // Split both into triangles
    std::array<std::array<Vector2, 3>, 2> trisRect = {{{rectCorners[0], rectCorners[1], rectCorners[2]},
                                                       {rectCorners[0], rectCorners[2], rectCorners[3]}}};
    std::array<std::array<Vector2, 3>, 2> trisRot = {{{rotated[0], rotated[1], rotated[2]},
                                                      {rotated[0], rotated[2], rotated[3]}}};

    // Check all triangle pairs
    for (auto &t1 : trisRect) {
        for (auto &t2 : trisRot) {
            for (const auto &v : t1)
                if (PointInTriangle(v, t2[0], t2[1], t2[2])) return true;
            for (const auto &v : t2)
                if (PointInTriangle(v, t1[0], t1[1], t1[2])) return true;
        }
    }

    return false;
}

inline bool CheckCollisionPointRectCorners(const Vector2 &point, const std::array<Vector2, 4> &rectCorners) {
    // Split the rotated rectangle into two triangles
    std::array<std::array<Vector2, 3>, 2> tris = {{
        {rectCorners[0], rectCorners[1], rectCorners[2]},
        {rectCorners[0], rectCorners[2], rectCorners[3]}
    }};

    // If the point lies inside either triangle, it’s inside the rectangle
    return PointInTriangle(point, tris[0][0], tris[0][1], tris[0][2]) ||
           PointInTriangle(point, tris[1][0], tris[1][1], tris[1][2]);
}

} // namespace Collision

namespace Random {
float GetRandomFloat(float min, float max) {
    return min + (float)GetRandomValue(0, 10000) / 10000.0f * (max - min);
}
}

namespace Textures {
inline Texture2D LoadTextureFromMemory(const char *fileType, const unsigned char *fileData, int dataSize) {
    Image image = LoadImageFromMemory(fileType, fileData, dataSize);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}
}

namespace Sounds {
inline Sound LoadSoundFromMemory(const char *fileType, const unsigned char *fileData, int dataSize) {
    Wave wave = LoadWaveFromMemory(fileType, fileData, dataSize);
    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}
}

namespace Timers {
class Timer {
public:
    float duration;
    float elapsed = 0;
    bool isRunning = false;
    std::function<void()> callback;

    Timer(float duration, std::function<void()> callback) {
        this->duration = duration;
        this->callback = callback;
    }

    void start() {
        isRunning = true;
        elapsed = 0;
    }

    void update(float dt) {
        if (!isRunning) return;
        elapsed += dt;
        if (elapsed >= duration) {
            isRunning = false;
            callback();
        }
    }
};
};

namespace RayGuiTools {
inline void SetAllButtonBaseStyles(int value) {
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, value);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, value);
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, value);
}
inline void SetAllButtonBorderStyles(int value) {
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, value);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, value);
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, value);
    GuiSetStyle(BUTTON, BORDER_COLOR_DISABLED, value);
}
inline void SetAllButtonTextStyles(int value) {
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, value);
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, value);
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, value);
    GuiSetStyle(BUTTON, TEXT_COLOR_DISABLED, value);
}
inline void SetAllComboBoxBaseStyles(int value) {
    GuiSetStyle(COMBOBOX, BASE_COLOR_NORMAL, value);
    GuiSetStyle(COMBOBOX, BASE_COLOR_FOCUSED, value);
    GuiSetStyle(COMBOBOX, BASE_COLOR_PRESSED, value);
}
inline void SetAllComboBoxBorderStyles(int value) {
    GuiSetStyle(COMBOBOX, BORDER_COLOR_NORMAL, value);
    GuiSetStyle(COMBOBOX, BORDER_COLOR_FOCUSED, value);
    GuiSetStyle(COMBOBOX, BORDER_COLOR_PRESSED, value);
    GuiSetStyle(COMBOBOX, BORDER_COLOR_DISABLED, value);
}
inline void SetAllComboBoxTextStyles(int value) {
    GuiSetStyle(COMBOBOX, TEXT_COLOR_NORMAL, value);
    GuiSetStyle(COMBOBOX, TEXT_COLOR_FOCUSED, value);
    GuiSetStyle(COMBOBOX, TEXT_COLOR_PRESSED, value);
    GuiSetStyle(COMBOBOX, TEXT_COLOR_DISABLED, value);
}
}