// Stack Visualizer - a raylib GUI built on top of the existing stack.h functions.
//
// The stack itself is untouched: push(), pop(), peek(), display(), the global
// array and top all come from stack.h. This file only handles drawing and input.

#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

#include "stack.h"

using namespace std;

// ---------------------------------------------------------------------------
// Layout and colors
// ---------------------------------------------------------------------------

static const int SCREEN_W = 1120;
static const int SCREEN_H = 760;
static const float TOP_BAR_H = 88.0f;
static const float BOTTOM_BAR_H = 64.0f;
static const int VISIBLE_SLOTS = 12;

static const Color BG           = { 15, 19, 28, 255 };
static const Color PANEL        = { 24, 30, 43, 255 };
static const Color PANEL_ALT    = { 33, 41, 58, 255 };
static const Color BORDER       = { 55, 68, 92, 255 };
static const Color TEXT         = { 231, 237, 245, 255 };
static const Color MUTED        = { 160, 173, 194, 255 };
static const Color ACCENT       = { 59, 130, 246, 255 };
static const Color ACCENT_DARK  = { 37, 99, 235, 255 };
static const Color CARD_COLOR   = { 37, 99, 235, 255 };
static const Color TOP_COLOR    = { 250, 204, 21, 255 };
static const Color SUCCESS      = { 74, 222, 128, 255 };
static const Color ERROR        = { 248, 113, 113, 255 };
static const Color INFO         = { 96, 165, 250, 255 };

// ---------------------------------------------------------------------------
// Animation and application state
// ---------------------------------------------------------------------------

enum class AnimType {
    None,
    Push,
    Pop
};

struct Animation {
    AnimType type = AnimType::None;
    float t = 0.0f;
    int value = 0;
    Rectangle from = { 0.0f, 0.0f, 0.0f, 0.0f };
    Rectangle to = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct AppState {
    string input;
    string status = "Enter an integer and press Push.";
    Color statusColor = INFO;

    Animation anim;
    float pulse = 0.0f;

    bool quit = false;
    bool smokeTest = false;
    bool demo = false;
    int frame = 0;
};

static AppState app;

static Font fontRegular;
static Font fontBold;
static bool customRegular = false;
static bool customBold = false;

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------

static float clampf(float value, float low, float high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

static float easeOutCubic(float t) {
    float u = 1.0f - t;
    return 1.0f - u * u * u;
}

static float easeInCubic(float t) {
    return t * t * t;
}

static void setStatus(const string& message, Color color) {
    app.status = message;
    app.statusColor = color;
}

static float measureText(const string& text, float size, bool bold = false) {
    Font font = bold ? fontBold : fontRegular;
    return MeasureTextEx(font, text.c_str(), size, 0.0f).x;
}

static void drawText(const string& text, float x, float y, float size, Color color, bool bold = false) {
    Font font = bold ? fontBold : fontRegular;
    DrawTextEx(font, text.c_str(), Vector2{ x, y }, size, 0.0f, color);
}

static void drawTextCentered(const string& text, float centerX, float y, float size, Color color, bool bold = false) {
    drawText(text, centerX - measureText(text, size, bold) / 2.0f, y, size, color, bold);
}

static bool pointInRectangle(Vector2 point, Rectangle rectangle) {
    return CheckCollisionPointRec(point, rectangle);
}

static bool buttonClicked(Rectangle rectangle) {
    return pointInRectangle(GetMousePosition(), rectangle) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static bool buttonHovered(Rectangle rectangle) {
    return pointInRectangle(GetMousePosition(), rectangle);
}

static void drawButton(Rectangle rectangle, const string& label, bool primary,
                       bool enabled = true, float fontSize = 19.0f) {
    bool hovered = enabled && buttonHovered(rectangle);

    Color fill = primary ? ACCENT : PANEL_ALT;
    if (!enabled) {
        fill = PANEL;
    } else if (hovered) {
        fill = primary ? ACCENT_DARK : Color{ 45, 57, 80, 255 };
    }

    DrawRectangleRounded(rectangle, 0.28f, 8, fill);
    DrawRectangleRoundedLinesEx(rectangle, 0.28f, 8, 1.0f,
                                primary ? Color{ 96, 165, 250, 255 } : BORDER);
    drawTextCentered(label, rectangle.x + rectangle.width / 2.0f,
                     rectangle.y + (rectangle.height - fontSize) / 2.0f - 1.0f,
                     fontSize, enabled ? TEXT : MUTED, true);
}

static void loadFonts() {
    fontRegular = GetFontDefault();
    fontBold = GetFontDefault();

    const char* regularCandidates[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };

    for (const char* path : regularCandidates) {
        if (FileExists(path)) {
            Font font = LoadFontEx(path, 64, nullptr, 0);
            if (IsFontValid(font)) {
                fontRegular = font;
                customRegular = true;
                break;
            }
        }
    }

    const char* boldCandidates[] = {
        "C:/Windows/Fonts/segoeuib.ttf",
        "C:/Windows/Fonts/arialbd.ttf"
    };

    for (const char* path : boldCandidates) {
        if (FileExists(path)) {
            Font font = LoadFontEx(path, 64, nullptr, 0);
            if (IsFontValid(font)) {
                fontBold = font;
                customBold = true;
                break;
            }
        }
    }

    if (!customBold) {
        fontBold = fontRegular;
    }

    if (customRegular) {
        SetTextureFilter(fontRegular.texture, TEXTURE_FILTER_BILINEAR);
    }
    if (customBold) {
        SetTextureFilter(fontBold.texture, TEXTURE_FILTER_BILINEAR);
    }
}

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------

static Rectangle stackPanel() {
    return Rectangle{ 32.0f, 108.0f, 700.0f, 580.0f };
}

static Rectangle controlsPanel() {
    return Rectangle{ 756.0f, 108.0f, 332.0f, 580.0f };
}

static Rectangle stackContainer() {
    return Rectangle{ 72.0f, 176.0f, 620.0f, 490.0f };
}

static Rectangle inputBox() {
    return Rectangle{ 780.0f, 174.0f, 284.0f, 54.0f };
}

static Rectangle pushButton() {
    return Rectangle{ 780.0f, 248.0f, 136.0f, 46.0f };
}

static Rectangle popButton() {
    return Rectangle{ 928.0f, 248.0f, 136.0f, 46.0f };
}

static Rectangle peekButton() {
    return Rectangle{ 780.0f, 306.0f, 284.0f, 46.0f };
}

static Rectangle statsPanel() {
    return Rectangle{ 780.0f, 380.0f, 284.0f, 150.0f };
}

static Rectangle slotRect(int visualIndex) {
    Rectangle container = stackContainer();
    const float slotHeight = 34.0f;
    const float slotGap = 6.0f;

    float y = container.y + container.height -
              (visualIndex + 1) * (slotHeight + slotGap);

    return Rectangle{ 150.0f, y, 340.0f, slotHeight };
}

// ---------------------------------------------------------------------------
// Operations
// ---------------------------------------------------------------------------

static bool parseValue(int& value) {
    if (app.input.empty() || app.input == "-") {
        setStatus("Enter an integer first.", ERROR);
        return false;
    }

    value = atoi(app.input.c_str());
    return true;
}

static void doPush() {
    int value = 0;
    if (!parseValue(value)) {
        return;
    }

    if (push(value)) {
        int visual = min(top, VISIBLE_SLOTS - 1);
        Rectangle end = slotRect(visual);

        app.anim.type = AnimType::Push;
        app.anim.t = 0.0f;
        app.anim.value = value;
        app.anim.from = Rectangle{ end.x + 480.0f, end.y, end.width, end.height };
        app.anim.to = end;
        app.pulse = 1.0f;

        app.input.clear();
        setStatus("Pushed " + to_string(value) + " onto the stack.", SUCCESS);
    } else {
        setStatus("Stack is full (MAX = 100).", ERROR);
    }
}

static void doPop() {
    if (top == -1) {
        setStatus("Stack is empty.", ERROR);
        return;
    }

    int value = stack[top];
    int visual = min(top, VISIBLE_SLOTS - 1);
    Rectangle start = slotRect(visual);

    if (pop()) {
        app.anim.type = AnimType::Pop;
        app.anim.t = 0.0f;
        app.anim.value = value;
        app.anim.from = start;
        app.anim.to = Rectangle{ start.x + 480.0f, start.y, start.width, start.height };

        setStatus("Popped " + to_string(value) + " from the stack.", INFO);
    }
}

static void doPeek() {
    if (top == -1) {
        setStatus("Stack is empty.", ERROR);
        return;
    }

    peek();
    app.pulse = 1.0f;
    setStatus("Top value: " + to_string(stack[top]), INFO);
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

static void update() {
    float dt = GetFrameTime();

    if (app.anim.type != AnimType::None) {
        app.anim.t += dt * 2.6f;
        if (app.anim.t >= 1.0f) {
            app.anim.type = AnimType::None;
            app.anim.t = 0.0f;
        }
    }

    if (app.pulse > 0.0f) {
        app.pulse = clampf(app.pulse - dt * 1.8f, 0.0f, 1.0f);
    }

    int character = 0;
    while ((character = GetCharPressed()) > 0) {
        bool digit = character >= '0' && character <= '9';
        bool minus = character == '-' && app.input.empty();

        if ((digit || minus) && app.input.size() < 7) {
            app.input.push_back((char)character);
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !app.input.empty()) {
        app.input.pop_back();
    }

    if (IsKeyPressed(KEY_ENTER)) {
        doPush();
    }
    if (IsKeyPressed(KEY_P)) {
        doPop();
    }
    if (IsKeyPressed(KEY_K)) {
        doPeek();
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        app.quit = true;
    }

    if (buttonClicked(pushButton())) {
        doPush();
    } else if (buttonClicked(popButton()) && top >= 0) {
        doPop();
    } else if (buttonClicked(peekButton()) && top >= 0) {
        doPeek();
    }

    bool overButton = buttonHovered(pushButton()) ||
                      (top >= 0 && (buttonHovered(popButton()) ||
                                    buttonHovered(peekButton())));

    if (overButton) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else if (pointInRectangle(GetMousePosition(), inputBox())) {
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
}

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------

static void drawDotGrid(float top, float bottom) {
    for (int x = 40; x < SCREEN_W; x += 40) {
        for (int y = (int)top + 40; y < (int)bottom; y += 40) {
            DrawCircle(x, y, 1.2f, Color{ 255, 255, 255, 10 });
        }
    }
}

static void drawCard(Rectangle rect, int value, bool isTop, float alpha) {
    if (alpha <= 0.0f) {
        return;
    }

    DrawRectangleRounded(Rectangle{ rect.x + 4.0f, rect.y + 6.0f,
                                    rect.width, rect.height },
                         0.25f, 8, Fade(BLACK, 0.25f * alpha));

    Color fill = isTop ? TOP_COLOR : CARD_COLOR;
    Color border = isTop ? Color{ 253, 224, 71, 255 } : Color{ 96, 165, 250, 255 };

    DrawRectangleRounded(rect, 0.25f, 8, Fade(fill, alpha));
    DrawRectangleRoundedLinesEx(rect, 0.25f, 8, 2.0f, Fade(border, alpha));

    string text = to_string(value);
    float fontSize = 20.0f;
    Color textColor = isTop ? Color{ 20, 24, 33, 255 } : TEXT;

    drawTextCentered(text, rect.x + rect.width / 2.0f,
                     rect.y + (rect.height - fontSize) / 2.0f - 1.0f,
                     fontSize, Fade(textColor, alpha), true);
}

static void drawTopBar() {
    DrawRectangle(0, 0, SCREEN_W, (int)TOP_BAR_H, PANEL);
    DrawLine(0, (int)TOP_BAR_H, SCREEN_W, (int)TOP_BAR_H, BORDER);

    drawText("Stack Visualizer", 24.0f, 14.0f, 30.0f, TEXT, true);
    drawText("Array-based stack  |  LIFO  |  top at the top",
             24.0f, 50.0f, 18.0f, MUTED);

    Rectangle badge = { SCREEN_W - 234.0f, 24.0f, 198.0f, 40.0f };
    DrawRectangleRounded(badge, 0.3f, 8, PANEL_ALT);
    DrawRectangleRoundedLinesEx(badge, 0.3f, 8, 1.0f, BORDER);
    drawTextCentered("MAX 100  |  int", badge.x + badge.width / 2.0f,
                     badge.y + 10.0f, 15.0f, MUTED, true);
}

static void drawStackPanel() {
    Rectangle panel = stackPanel();
    DrawRectangleRounded(panel, 0.03f, 10, PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.03f, 10, 1.0f, BORDER);

    drawText("STACK", 56.0f, 120.0f, 24.0f, TEXT, true);
    drawText("The top of the stack is drawn at the top.",
             56.0f, 150.0f, 16.0f, MUTED);

    Rectangle container = stackContainer();
    DrawRectangleRounded(container, 0.03f, 10, BG);
    DrawRectangleRoundedLinesEx(container, 0.03f, 10, 1.0f, BORDER);

    for (int slot = 0; slot < VISIBLE_SLOTS; slot++) {
        Rectangle rect = slotRect(slot);
        DrawRectangleRounded(rect, 0.25f, 8, PANEL_ALT);
        DrawRectangleRoundedLinesEx(rect, 0.25f, 8, 1.0f, Color{ 43, 54, 75, 255 });
    }

    int visibleCount = min(top + 1, VISIBLE_SLOTS);
    int visibleStart = max(0, top - (VISIBLE_SLOTS - 1));

    for (int slot = 0; slot < visibleCount; slot++) {
        int index = visibleStart + slot;
        Rectangle rect = slotRect(slot);

        bool isTop = index == top;
        bool pushTarget = app.anim.type == AnimType::Push && isTop;

        drawText(to_string(index), 100.0f, rect.y + 8.0f, 15.0f,
                 isTop ? ACCENT : MUTED, isTop);

        if (!pushTarget) {
            drawCard(rect, stack[index], isTop, 1.0f);
        }
    }

    if (top >= 0) {
        int topSlot = min(top, VISIBLE_SLOTS - 1);
        Rectangle topRect = slotRect(topSlot);
        float glow = 0.35f + 0.25f * (float)sin(GetTime() * 3.0) + app.pulse * 0.4f;
        DrawRectangleRoundedLinesEx(
            Rectangle{ topRect.x - 5.0f, topRect.y - 5.0f,
                       topRect.width + 10.0f, topRect.height + 10.0f },
            0.25f, 8, 2.0f, Fade(ACCENT, clampf(glow, 0.0f, 0.9f)));

        Vector2 tip = { topRect.x - 18.0f, topRect.y + topRect.height / 2.0f };
        DrawTriangle(Vector2{ tip.x, tip.y - 7.0f },
                     Vector2{ tip.x, tip.y + 7.0f },
                     Vector2{ tip.x + 11.0f, tip.y }, TOP_COLOR);
        drawText("TOP", topRect.x + topRect.width + 18.0f,
                 topRect.y + topRect.height / 2.0f - 9.0f, 16.0f, ACCENT, true);
    } else {
        drawTextCentered("Stack is empty", container.x + container.width / 2.0f,
                         container.y + container.height / 2.0f - 10.0f,
                         18.0f, MUTED);
    }

    if (visibleStart > 0) {
        drawText("+ " + to_string(visibleStart) + " more below",
                 100.0f, container.y + container.height + 2.0f, 15.0f, MUTED);
    }

    if (app.anim.type != AnimType::None) {
        bool pushing = app.anim.type == AnimType::Push;
        float t = pushing ? easeOutCubic(app.anim.t) : easeInCubic(app.anim.t);
        float x = app.anim.from.x + (app.anim.to.x - app.anim.from.x) * t;
        Rectangle ghost = { x, app.anim.from.y,
                            app.anim.from.width, app.anim.from.height };
        float alpha = pushing ? clampf(app.anim.t * 3.0f, 0.0f, 1.0f)
                              : 1.0f - app.anim.t;
        drawCard(ghost, app.anim.value, pushing, alpha);
    }
}

static void drawControlsPanel() {
    Rectangle panel = controlsPanel();
    DrawRectangleRounded(panel, 0.03f, 10, PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.03f, 10, 1.0f, BORDER);

    drawText("OPERATIONS", 780.0f, 120.0f, 18.0f, MUTED, true);
    drawText("VALUE", 780.0f, 149.0f, 16.0f, MUTED, true);

    Rectangle input = inputBox();
    DrawRectangleRounded(input, 0.18f, 10, PANEL_ALT);
    DrawRectangleRoundedLinesEx(input, 0.18f, 10, 2.0f, ACCENT);

    string shown = app.input.empty() ? "enter an integer" : app.input;
    Color shownColor = app.input.empty() ? MUTED : TEXT;
    drawText(shown, input.x + 18.0f, input.y + 14.0f, 24.0f, shownColor, true);

    if (fmod(GetTime(), 1.0) < 0.5) {
        float textWidth = measureText(shown, 24.0f, true);
        DrawRectangle((int)(input.x + 18.0f + textWidth + 4.0f),
                      (int)(input.y + 18.0f), 2, 26, ACCENT);
    }

    drawButton(pushButton(), "Push", true, true, 20.0f);
    drawButton(popButton(), "Pop", false, top >= 0, 20.0f);
    drawButton(peekButton(), "Peek", false, top >= 0, 20.0f);

    drawText("Enter: push    P: pop    K: peek",
             780.0f, 360.0f, 16.0f, MUTED);

    Rectangle stats = statsPanel();
    DrawRectangleRounded(stats, 0.05f, 10, PANEL_ALT);
    DrawRectangleRoundedLinesEx(stats, 0.05f, 10, 1.0f, BORDER);

    drawText("STATISTICS", stats.x + 16.0f, stats.y + 12.0f, 16.0f, MUTED, true);

    int size = top + 1;
    string state = (top == -1) ? "Empty" : (size >= MAX ? "Full" : "Ready");
    Color stateColor = (top == -1) ? MUTED : (size >= MAX ? ERROR : SUCCESS);

    drawText("Size", stats.x + 16.0f, stats.y + 40.0f, 16.0f, MUTED);
    drawText(to_string(size) + " / " + to_string(MAX),
             stats.x + 130.0f, stats.y + 40.0f, 16.0f, TEXT, true);

    drawText("Top index", stats.x + 16.0f, stats.y + 66.0f, 16.0f, MUTED);
    drawText(top >= 0 ? to_string(top) : "-",
             stats.x + 130.0f, stats.y + 66.0f, 16.0f, TEXT, true);

    drawText("State", stats.x + 16.0f, stats.y + 92.0f, 16.0f, MUTED);
    drawText(state, stats.x + 130.0f, stats.y + 92.0f, 16.0f, stateColor, true);

    float ratio = (float)size / (float)MAX;
    DrawRectangleRounded(Rectangle{ stats.x + 16.0f, stats.y + 122.0f,
                                    stats.width - 32.0f, 8.0f },
                         0.5f, 8, Color{ 15, 19, 28, 255 });
    if (ratio > 0.0f) {
        DrawRectangleRounded(Rectangle{ stats.x + 16.0f, stats.y + 122.0f,
                                        (stats.width - 32.0f) * ratio, 8.0f },
                             0.5f, 8, ACCENT);
    }

    drawText("LIFO", 780.0f, 556.0f, 18.0f, TEXT, true);
    drawText("Last in, first out.", 780.0f, 578.0f, 16.0f, MUTED);

    DrawCircle(788, 618, 5.0f, TOP_COLOR);
    drawText("top element", 802.0f, 609.0f, 16.0f, MUTED);

    DrawCircle(788, 642, 5.0f, CARD_COLOR);
    drawText("stack value", 802.0f, 633.0f, 16.0f, MUTED);
}

static void drawBottomBar() {
    int bottomY = SCREEN_H - (int)BOTTOM_BAR_H;
    DrawRectangle(0, bottomY, SCREEN_W, (int)BOTTOM_BAR_H, PANEL);
    DrawLine(0, bottomY, SCREEN_W, bottomY, BORDER);

    DrawCircle(34, bottomY + 23, 5.0f, app.statusColor);
    drawText(app.status, 48.0f, (float)bottomY + 13.0f, 18.0f, TEXT);

    drawText("Click a button or use the keyboard  |  Esc to quit",
             24.0f, (float)bottomY + 37.0f, 16.0f, MUTED);
}

static void draw() {
    ClearBackground(BG);
    drawDotGrid(TOP_BAR_H, SCREEN_H - BOTTOM_BAR_H);

    drawTopBar();
    drawStackPanel();
    drawControlsPanel();
    drawBottomBar();
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--smoke-test") {
            app.smokeTest = true;
        }
        if (string(argv[i]) == "--demo") {
            app.demo = true;
        }
    }

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_W, SCREEN_H, "Stack Visualizer");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    loadFonts();

    if (app.smokeTest || app.demo) {
        push(12);
        push(7);
        push(23);
        push(5);
        push(18);
        setStatus(app.smokeTest
                      ? "Smoke test mode: stack drawn with 5 values."
                      : "Demo stack: 12, 7, 23, 5, 18.",
                  INFO);
    }

    while (!WindowShouldClose() && !app.quit) {
        update();

        BeginDrawing();
        draw();
        EndDrawing();

        if (app.smokeTest && ++app.frame > 30) {
            break;
        }
    }

    if (customRegular) {
        UnloadFont(fontRegular);
    }
    if (customBold) {
        UnloadFont(fontBold);
    }

    CloseWindow();
    return 0;
}
