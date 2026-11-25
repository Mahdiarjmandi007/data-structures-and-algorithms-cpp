// Queue Visualizer - a raylib GUI built on top of the existing queue.h functions.
//
// The queue itself is untouched: enqueue(), dequeue(), peek(), length(),
// isempty(), is_full(), the global array and indices all come from queue.h.
// This file only handles drawing and input.

// Include the queue header before <algorithm> so its unqualified `count`
// resolves to the existing global variable, not std::count.
#include "queue.h"

#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

using namespace std;

// ---------------------------------------------------------------------------
// Layout and colors
// ---------------------------------------------------------------------------

static const int SCREEN_W = 1180;
static const int SCREEN_H = 780;
static const float TOP_BAR_H = 88.0f;
static const float BOTTOM_BAR_H = 64.0f;
static const int LANE_MAX = 10;
static const int GRID_COLS = 10;
static const int GRID_ROWS = 10;

static const Color BG           = { 15, 19, 28, 255 };
static const Color PANEL        = { 24, 30, 43, 255 };
static const Color PANEL_ALT    = { 33, 41, 58, 255 };
static const Color BORDER       = { 55, 68, 92, 255 };
static const Color TEXT         = { 231, 237, 245, 255 };
static const Color MUTED        = { 160, 173, 194, 255 };
static const Color ACCENT       = { 59, 130, 246, 255 };
static const Color ACCENT_DARK  = { 37, 99, 235, 255 };
static const Color CARD_COLOR   = { 37, 99, 235, 255 };
static const Color FRONT_COLOR  = { 74, 222, 128, 255 };
static const Color REAR_COLOR   = { 251, 146, 60, 255 };
static const Color SUCCESS      = { 74, 222, 128, 255 };
static const Color ERROR        = { 248, 113, 113, 255 };
static const Color INFO         = { 96, 165, 250, 255 };

// ---------------------------------------------------------------------------
// Animation and application state
// ---------------------------------------------------------------------------

enum class AnimType {
    None,
    Enqueue,
    Dequeue
};

struct Animation {
    AnimType type = AnimType::None;
    float t = 0.0f;
    int value = 0;
    int index = -1;
    Rectangle from = { 0.0f, 0.0f, 0.0f, 0.0f };
    Rectangle to = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct AppState {
    string input;
    string status = "Enter an integer and press Enqueue.";
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

static Rectangle lanePanel() {
    return Rectangle{ 32.0f, 108.0f, 800.0f, 180.0f };
}

static Rectangle gridPanel() {
    return Rectangle{ 32.0f, 304.0f, 800.0f, 392.0f };
}

static Rectangle controlsPanel() {
    return Rectangle{ 856.0f, 108.0f, 292.0f, 588.0f };
}

static Rectangle inputBox() {
    return Rectangle{ 880.0f, 174.0f, 244.0f, 54.0f };
}

static Rectangle enqueueButton() {
    return Rectangle{ 880.0f, 248.0f, 244.0f, 46.0f };
}

static Rectangle dequeueButton() {
    return Rectangle{ 880.0f, 304.0f, 244.0f, 46.0f };
}

static Rectangle peekButton() {
    return Rectangle{ 880.0f, 360.0f, 244.0f, 46.0f };
}

static Rectangle statsPanel() {
    return Rectangle{ 880.0f, 440.0f, 244.0f, 150.0f };
}

static Rectangle laneBoxRect(int position) {
    return Rectangle{ 56.0f + position * 70.0f, 166.0f, 58.0f, 58.0f };
}

static Rectangle gridCellRect(int index) {
    const float startX = 48.0f;
    const float startY = 356.0f;
    const float cellW = 66.0f;
    const float cellH = 26.0f;
    const float gapX = 10.0f;
    const float gapY = 6.0f;

    int column = index % GRID_COLS;
    int row = index / GRID_COLS;

    return Rectangle{
        startX + column * (cellW + gapX),
        startY + row * (cellH + gapY),
        cellW,
        cellH
    };
}

static bool isFilledIndex(int index) {
    if (::count == 0) {
        return false;
    }

    int distance = (index - front + MAX) % MAX;
    return distance < ::count;
}

static int logicalIndexAt(int position) {
    if (::count == 0) {
        return -1;
    }

    return (front + position) % MAX;
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

static void doEnqueue() {
    int value = 0;
    if (!parseValue(value)) {
        return;
    }

    if (!enqueue(value)) {
        setStatus("Queue is full (MAX = 100).", ERROR);
        return;
    }

    int newIndex = rear;
    Rectangle cell = gridCellRect(newIndex);
    Vector2 center = { cell.x + cell.width / 2.0f,
                       cell.y + cell.height / 2.0f };

    app.anim.type = AnimType::Enqueue;
    app.anim.t = 0.0f;
    app.anim.value = value;
    app.anim.index = newIndex;
    app.anim.to = Rectangle{ center.x - 27.0f, center.y - 14.0f, 54.0f, 28.0f };
    app.anim.from = Rectangle{ gridPanel().x + gridPanel().width - 60.0f,
                               app.anim.to.y, 54.0f, 28.0f };

    app.input.clear();
    setStatus("Enqueued " + to_string(value) + " at slot " +
              to_string(newIndex) + ".", SUCCESS);
}

static void doDequeue() {
    if (isempty()) {
        setStatus("Queue is empty.", ERROR);
        return;
    }

    int oldFront = front;
    int value = queue[front];
    Rectangle cell = gridCellRect(oldFront);
    Vector2 center = { cell.x + cell.width / 2.0f,
                       cell.y + cell.height / 2.0f };

    if (dequeue()) {
        app.anim.type = AnimType::Dequeue;
        app.anim.t = 0.0f;
        app.anim.value = value;
        app.anim.index = oldFront;
        app.anim.from = Rectangle{ center.x - 27.0f, center.y - 14.0f, 54.0f, 28.0f };
        app.anim.to = Rectangle{ gridPanel().x - 30.0f,
                                 app.anim.from.y, 54.0f, 28.0f };

        setStatus("Dequeued " + to_string(value) + " from slot " +
                  to_string(oldFront) + ".", INFO);
    }
}

static void doPeek() {
    if (isempty()) {
        setStatus("Queue is empty.", ERROR);
        return;
    }

    peek();
    app.pulse = 1.0f;
    setStatus("Front value: " + to_string(queue[front]), INFO);
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

static void update() {
    float dt = GetFrameTime();

    if (app.anim.type != AnimType::None) {
        app.anim.t += dt * 2.5f;
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
        doEnqueue();
    }
    if (IsKeyPressed(KEY_D)) {
        doDequeue();
    }
    if (IsKeyPressed(KEY_K)) {
        doPeek();
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        app.quit = true;
    }

    if (buttonClicked(enqueueButton())) {
        doEnqueue();
    } else if (buttonClicked(dequeueButton()) && !isempty()) {
        doDequeue();
    } else if (buttonClicked(peekButton()) && !isempty()) {
        doPeek();
    }

    bool overButton = buttonHovered(enqueueButton()) ||
                      (!isempty() && (buttonHovered(dequeueButton()) ||
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

static void drawMiniCard(Rectangle rect, int value, Color color, float alpha) {
    if (alpha <= 0.0f) {
        return;
    }

    DrawRectangleRounded(Rectangle{ rect.x + 3.0f, rect.y + 4.0f,
                                    rect.width, rect.height },
                         0.3f, 8, Fade(BLACK, 0.25f * alpha));
    DrawRectangleRounded(rect, 0.3f, 8, Fade(color, alpha));
    DrawRectangleRoundedLinesEx(rect, 0.3f, 8, 1.5f, Fade(WHITE, 0.25f * alpha));

    string text = to_string(value);
    float fontSize = 15.0f;
    drawTextCentered(text, rect.x + rect.width / 2.0f,
                     rect.y + (rect.height - fontSize) / 2.0f - 1.0f,
                     fontSize, Fade(TEXT, alpha), true);
}

static void drawTopBar() {
    DrawRectangle(0, 0, SCREEN_W, (int)TOP_BAR_H, PANEL);
    DrawLine(0, (int)TOP_BAR_H, SCREEN_W, (int)TOP_BAR_H, BORDER);

    drawText("Queue Visualizer", 24.0f, 14.0f, 30.0f, TEXT, true);
    drawText("Circular array queue  |  FIFO  |  front to rear",
             24.0f, 50.0f, 18.0f, MUTED);

    Rectangle badge = { SCREEN_W - 244.0f, 24.0f, 208.0f, 40.0f };
    DrawRectangleRounded(badge, 0.3f, 8, PANEL_ALT);
    DrawRectangleRoundedLinesEx(badge, 0.3f, 8, 1.0f, BORDER);
    drawTextCentered("MAX 100  |  circular", badge.x + badge.width / 2.0f,
                     badge.y + 10.0f, 15.0f, MUTED, true);
}

static void drawLanePanel() {
    Rectangle panel = lanePanel();
    DrawRectangleRounded(panel, 0.03f, 10, PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.03f, 10, 1.0f, BORDER);

    drawText("LOGICAL QUEUE", 56.0f, 120.0f, 18.0f, MUTED, true);

    int size = length();
    drawText("Dequeue removes the front; enqueue adds at the rear.",
             56.0f, 250.0f, 15.0f, MUTED);

    if (size == 0) {
        drawTextCentered("Queue is empty", panel.x + panel.width / 2.0f,
                         panel.y + 78.0f, 18.0f, MUTED);
        return;
    }

    if (size == 1) {
        drawTextCentered("front / rear", laneBoxRect(0).x + 29.0f, 146.0f,
                         14.0f, FRONT_COLOR, true);
    } else {
        drawText("front", laneBoxRect(0).x + 4.0f, 146.0f, 14.0f,
                 FRONT_COLOR, true);

        if (size <= LANE_MAX) {
            int rearPos = size - 1;
            drawTextCentered("rear", laneBoxRect(rearPos).x + 29.0f, 146.0f,
                             14.0f, REAR_COLOR, true);
        } else {
            drawText("rear: slot " + to_string(rear), 600.0f, 146.0f,
                     14.0f, REAR_COLOR, true);
        }
    }

    int shown = min(size, LANE_MAX);
    int hidden = 0;

    if (size > LANE_MAX) {
        shown = LANE_MAX - 1;
        hidden = size - shown;
    }

    for (int position = 0; position < shown; position++) {
        int index = logicalIndexAt(position);
        Rectangle box = laneBoxRect(position);
        bool isFront = position == 0;
        bool isRear = position == size - 1;

        Color fill = isFront ? Color{ 22, 101, 52, 255 }
                             : (isRear ? Color{ 124, 45, 18, 255 } : CARD_COLOR);
        Color border = isFront ? FRONT_COLOR : (isRear ? REAR_COLOR : BORDER);

        DrawRectangleRounded(box, 0.22f, 8, fill);
        DrawRectangleRoundedLinesEx(box, 0.22f, 8, 2.0f, border);

        drawText(to_string(index), box.x + 6.0f, box.y + 3.0f, 12.0f,
                 Fade(TEXT, 0.75f), true);

        string value = to_string(queue[index]);
        float fontSize = 17.0f;
        float textWidth = measureText(value, fontSize, true);
        if (textWidth > box.width - 10.0f) {
            fontSize *= (box.width - 10.0f) / textWidth;
        }
        drawTextCentered(value, box.x + box.width / 2.0f,
                         box.y + 24.0f, fontSize, TEXT, true);

        if (position + 1 < shown) {
            float arrowY = box.y + box.height / 2.0f;
            DrawTriangle(Vector2{ box.x + box.width + 3.0f, arrowY - 5.0f },
                         Vector2{ box.x + box.width + 3.0f, arrowY + 5.0f },
                         Vector2{ box.x + box.width + 10.0f, arrowY },
                         Fade(MUTED, 0.7f));
        }
    }

    if (hidden > 0) {
        Rectangle chip = laneBoxRect(shown);
        DrawRectangleRounded(chip, 0.22f, 8, PANEL_ALT);
        DrawRectangleRoundedLinesEx(chip, 0.22f, 8, 1.0f, BORDER);
        drawTextCentered("+" + to_string(hidden), chip.x + chip.width / 2.0f,
                         chip.y + 16.0f, 18.0f, MUTED, true);
        drawText("more", chip.x + 13.0f, chip.y + 42.0f, 12.0f, MUTED);
    }
}

static void drawGridPanel() {
    Rectangle panel = gridPanel();
    DrawRectangleRounded(panel, 0.03f, 10, PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.03f, 10, 1.0f, BORDER);

    drawText("CIRCULAR ARRAY", 56.0f, 318.0f, 18.0f, MUTED, true);
    drawText("100 slots  |  wrapped view", 56.0f, 337.0f, 15.0f, MUTED);

    for (int index = 0; index < MAX; index++) {
        Rectangle cell = gridCellRect(index);
        bool filled = isFilledIndex(index);
        bool isFront = ::count > 0 && index == front;
        bool isRear = ::count > 0 && index == rear;

        Color fill = filled ? CARD_COLOR : PANEL_ALT;
        Color border = BORDER;
        float borderWidth = 1.0f;

        if (isFront) {
            border = FRONT_COLOR;
            borderWidth = 2.0f + app.pulse * 2.0f;
        } else if (isRear) {
            border = REAR_COLOR;
            borderWidth = 2.0f;
        }

        if (filled) {
            fill = isFront ? Color{ 22, 101, 52, 255 }
                           : (isRear ? Color{ 124, 45, 18, 255 } : CARD_COLOR);
        }

        DrawRectangleRounded(cell, 0.25f, 8, fill);
        DrawRectangleRoundedLinesEx(cell, 0.25f, 8, borderWidth, border);

        drawText(to_string(index), cell.x + 4.0f, cell.y + 1.0f, 12.0f,
                 Fade(TEXT, filled ? 0.85f : 0.7f));

        if (filled) {
            string value = to_string(queue[index]);
            drawTextCentered(value, cell.x + cell.width / 2.0f,
                             cell.y + 8.0f, 15.0f, TEXT, true);
        }

        if (isFront && isRear) {
            drawText("FR", cell.x + cell.width - 18.0f, cell.y + 2.0f,
                     11.0f, FRONT_COLOR, true);
        } else if (isFront) {
            drawText("F", cell.x + cell.width - 12.0f, cell.y + 1.0f,
                     12.0f, FRONT_COLOR, true);
        } else if (isRear) {
            drawText("R", cell.x + cell.width - 12.0f, cell.y + 1.0f,
                     12.0f, REAR_COLOR, true);
        }

        if (app.anim.type == AnimType::Enqueue && app.anim.index == index) {
            float glow = 0.5f + 0.5f * app.anim.t;
            DrawRectangleRoundedLinesEx(
                Rectangle{ cell.x - 3.0f, cell.y - 3.0f,
                           cell.width + 6.0f, cell.height + 6.0f },
                0.25f, 8, 2.0f, Fade(ACCENT, glow));
        }

        if (app.anim.type == AnimType::Dequeue && app.anim.index == index) {
            DrawRectangleRoundedLinesEx(
                Rectangle{ cell.x - 3.0f, cell.y - 3.0f,
                           cell.width + 6.0f, cell.height + 6.0f },
                0.25f, 8, 2.0f, Fade(ERROR, 1.0f - app.anim.t));
        }
    }

    if (app.anim.type != AnimType::None) {
        bool enqueuing = app.anim.type == AnimType::Enqueue;
        float t = enqueuing ? easeOutCubic(app.anim.t) : easeInCubic(app.anim.t);
        float x = app.anim.from.x + (app.anim.to.x - app.anim.from.x) * t;
        Rectangle ghost = { x, app.anim.from.y,
                            app.anim.from.width, app.anim.from.height };
        float alpha = enqueuing ? clampf(app.anim.t * 3.0f, 0.0f, 1.0f)
                                : 1.0f - app.anim.t;
        drawMiniCard(ghost, app.anim.value,
                     enqueuing ? ACCENT : REAR_COLOR, alpha);
    }
}

static void drawControlsPanel() {
    Rectangle panel = controlsPanel();
    DrawRectangleRounded(panel, 0.03f, 10, PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.03f, 10, 1.0f, BORDER);

    drawText("OPERATIONS", 880.0f, 120.0f, 18.0f, MUTED, true);
    drawText("VALUE", 880.0f, 150.0f, 15.0f, MUTED, true);

    Rectangle input = inputBox();
    DrawRectangleRounded(input, 0.18f, 10, PANEL_ALT);
    DrawRectangleRoundedLinesEx(input, 0.18f, 10, 2.0f, ACCENT);

    string shown = app.input.empty() ? "enter an integer" : app.input;
    Color shownColor = app.input.empty() ? MUTED : TEXT;
    drawText(shown, input.x + 16.0f, input.y + 14.0f, 22.0f, shownColor, true);

    if (fmod(GetTime(), 1.0) < 0.5) {
        float textWidth = measureText(shown, 22.0f, true);
        DrawRectangle((int)(input.x + 16.0f + textWidth + 4.0f),
                      (int)(input.y + 18.0f), 2, 24, ACCENT);
    }

    drawButton(enqueueButton(), "Enqueue", true, true, 20.0f);
    drawButton(dequeueButton(), "Dequeue", false, !isempty(), 20.0f);
    drawButton(peekButton(), "Peek", false, !isempty(), 20.0f);

    drawText("Enter: enqueue    D: dequeue    K: peek",
             880.0f, 413.0f, 15.0f, MUTED);

    Rectangle stats = statsPanel();
    DrawRectangleRounded(stats, 0.05f, 10, PANEL_ALT);
    DrawRectangleRoundedLinesEx(stats, 0.05f, 10, 1.0f, BORDER);

    drawText("STATISTICS", stats.x + 16.0f, stats.y + 12.0f, 16.0f, MUTED, true);

    int size = length();
    string state = isempty() ? "Empty" : (is_full() ? "Full" : "Ready");
    Color stateColor = isempty() ? MUTED : (is_full() ? ERROR : SUCCESS);

    drawText("Size", stats.x + 16.0f, stats.y + 40.0f, 16.0f, MUTED);
    drawText(to_string(size) + " / " + to_string(MAX),
             stats.x + 108.0f, stats.y + 40.0f, 16.0f, TEXT, true);

    drawText("Front", stats.x + 16.0f, stats.y + 64.0f, 16.0f, MUTED);
    drawText(isempty() ? "-" : to_string(front),
             stats.x + 108.0f, stats.y + 64.0f, 16.0f, TEXT, true);

    drawText("Rear", stats.x + 16.0f, stats.y + 88.0f, 16.0f, MUTED);
    drawText(isempty() ? "-" : to_string(rear),
             stats.x + 108.0f, stats.y + 88.0f, 16.0f, TEXT, true);

    drawText("State", stats.x + 16.0f, stats.y + 112.0f, 16.0f, MUTED);
    drawText(state, stats.x + 108.0f, stats.y + 112.0f, 16.0f, stateColor, true);

    float ratio = (float)size / (float)MAX;
    DrawRectangleRounded(Rectangle{ stats.x + 16.0f, stats.y + 132.0f,
                                    stats.width - 32.0f, 6.0f },
                         0.5f, 8, Color{ 15, 19, 28, 255 });
    if (ratio > 0.0f) {
        DrawRectangleRounded(Rectangle{ stats.x + 16.0f, stats.y + 132.0f,
                                        (stats.width - 32.0f) * ratio, 6.0f },
                             0.5f, 8, ACCENT);
    }

    drawText("FIFO", 880.0f, 614.0f, 18.0f, TEXT, true);
    drawText("First in, first out.", 880.0f, 636.0f, 15.0f, MUTED);

    DrawCircle(888, 668, 5.0f, FRONT_COLOR);
    drawText("front", 902.0f, 660.0f, 15.0f, MUTED);

    DrawCircle(968, 668, 5.0f, REAR_COLOR);
    drawText("rear", 982.0f, 660.0f, 15.0f, MUTED);

    DrawCircle(1040, 668, 5.0f, CARD_COLOR);
    drawText("occupied", 1054.0f, 660.0f, 15.0f, MUTED);
}

static void drawBottomBar() {
    int bottomY = SCREEN_H - (int)BOTTOM_BAR_H;
    DrawRectangle(0, bottomY, SCREEN_W, (int)BOTTOM_BAR_H, PANEL);
    DrawLine(0, bottomY, SCREEN_W, bottomY, BORDER);

    DrawCircle(34, bottomY + 23, 5.0f, app.statusColor);
    drawText(app.status, 48.0f, (float)bottomY + 13.0f, 18.0f, TEXT);

    drawText("Click a button or use the keyboard  |  Esc to quit",
             24.0f, (float)bottomY + 37.0f, 16.0f, MUTED);

    const string complexity = "enqueue / dequeue / peek: O(1)";
    float complexityWidth = measureText(complexity, 15.0f, true);
    drawText(complexity, SCREEN_W - complexityWidth - 24.0f,
             (float)bottomY + 21.0f, 15.0f, ACCENT, true);
}

static void draw() {
    ClearBackground(BG);
    drawDotGrid(TOP_BAR_H, SCREEN_H - BOTTOM_BAR_H);

    drawTopBar();
    drawLanePanel();
    drawGridPanel();
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
    InitWindow(SCREEN_W, SCREEN_H, "Queue Visualizer");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    loadFonts();

    if (app.smokeTest || app.demo) {
        enqueue(10);
        enqueue(20);
        enqueue(30);
        enqueue(40);
        enqueue(50);
        setStatus(app.smokeTest
                      ? "Smoke test mode: queue drawn with 5 values."
                      : "Demo queue: 10, 20, 30, 40, 50.",
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
