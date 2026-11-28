// Circular Linked List Visualizer - a raylib GUI built on top of the
// existing circular linked list functions.
//
// All list mutations go through the functions copied from
// circular-linked-list.cpp into circular-linked-list.h.
// The UI only reads the global head for drawing and animation snapshots.

#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "circular-linked-list.h"

using namespace std;

// ---------------------------------------------------------------------------
// Layout and colors
// ---------------------------------------------------------------------------

static const int SCREEN_W = 1180;
static const int SCREEN_H = 760;
static const float TOP_BAR_H = 88.0f;
static const float CONTROLS_H = 112.0f;
static const float BOTTOM_BAR_H = 64.0f;
static const float ANIM_DURATION = 0.55f;
static const int MAX_NODES = 10;

static const float NODE_Y = 340.0f;
static const float NODE_H = 84.0f;
static const float NODE_GAP = 26.0f;
static const float LIST_LEFT = 40.0f;
static const float LIST_RIGHT = 1140.0f;
static const float HEAD_CARD_Y = 548.0f;
static const float BOTTOM_ROUTE_Y = 510.0f;

static const Color BG          = { 15, 19, 28, 255 };
static const Color PANEL       = { 24, 30, 43, 255 };
static const Color PANEL_ALT   = { 33, 41, 58, 255 };
static const Color BORDER      = { 55, 68, 92, 255 };
static const Color TEXT        = { 231, 237, 245, 255 };
static const Color MUTED       = { 160, 173, 194, 255 };
static const Color ACCENT      = { 59, 130, 246, 255 };
static const Color ACCENT_DARK = { 37, 99, 235, 255 };
static const Color NODE_COLOR  = { 56, 128, 235, 255 };
static const Color NODE_HOVER  = { 96, 165, 250, 255 };
static const Color HEAD_COLOR  = { 250, 204, 21, 255 };
static const Color EDGE_COLOR  = { 130, 145, 168, 255 };
static const Color SUCCESS     = { 74, 222, 128, 255 };
static const Color ERROR       = { 248, 113, 113, 255 };
static const Color INFO        = { 96, 165, 250, 255 };
static const Color NO_HIGHLIGHT = { 0, 0, 0, 0 };

// ---------------------------------------------------------------------------
// Application state
// ---------------------------------------------------------------------------

enum class Field {
    None,
    Value,
    Position
};

enum class ButtonStyle {
    Default,
    Primary,
    Danger
};

enum class OpType {
    None,
    Insert,
    Delete,
    Reverse,
    Clear,
    Search
};

struct NodeSnapshot {
    node* ptr;
    node* next;
    int value;
};

struct AnimNode {
    node* ptr;
    node* next;
    int value;
    int oldIndex;
    int newIndex;
    bool hadOld;
    bool hasNew;
};

struct AppState {
    // Current list snapshot (head first).
    vector<NodeSnapshot> list;
    int length = 0;
    bool hasHead = false;
    int headValue = 0;

    // Inputs
    string valueInput = "10";
    string positionInput = "0";
    Field activeField = Field::Value;
    bool valueFresh = true;
    bool positionFresh = true;

    // Status
    string status = "Enter a value, then choose an operation.";
    Color statusColor = INFO;

    // Node transition
    vector<AnimNode> animNodes;
    int oldCount = 0;
    int newCount = 0;
    float animT = 1.0f;
    bool animating = false;

    // Teaching steps
    vector<string> steps;
    int stepIndex = -1;
    float stepTimer = 0.0f;
    bool stepAnimating = false;
    OpType opType = OpType::None;

    string finalStatus;
    Color finalStatusColor = INFO;

    node* opNewNode = nullptr;
    node* opPrevNode = nullptr;
    node* opTargetNode = nullptr;
    node* opNextNode = nullptr;
    int opNewIndex = -1;

    node* highlightFrom = nullptr;
    node* highlightTo = nullptr;
    bool highlightHeadArrow = false;
    bool highlightAllEdges = false;

    // Search traversal
    int searchFoundIndex = -1;
    int searchCheckCount = 0;
    int searchValue = 0;

    // Feedback
    int highlightIndex = -1;
    float highlightTimer = 0.0f;
    int hoveredNode = -1;

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

static float lerpFloat(float a, float b, float t) {
    return a + (b - a) * t;
}

static Vector2 lerpVector(Vector2 a, Vector2 b, float t) {
    return Vector2{ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
}

static float easeOutCubic(float t) {
    float u = 1.0f - t;
    return 1.0f - u * u * u;
}

static float easeInCubic(float t) {
    return t * t * t;
}

static float easeInOutCubic(float t) {
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    }
    float u = -2.0f * t + 2.0f;
    return 1.0f - (u * u * u) / 2.0f;
}

static Vector2 normalizeVector(Vector2 v) {
    float length = sqrtf(v.x * v.x + v.y * v.y);
    if (length < 0.0001f) {
        return Vector2{ 0.0f, -1.0f };
    }
    return Vector2{ v.x / length, v.y / length };
}

static void setStatus(const string& message, Color color) {
    app.status = message;
    app.statusColor = color;
}

static float measureText(const string& text, float size, bool bold = false) {
    Font font = bold ? fontBold : fontRegular;
    return MeasureTextEx(font, text.c_str(), size, 0.0f).x;
}

static void drawText(const string& text, float x, float y, float size,
                     Color color, bool bold = false) {
    Font font = bold ? fontBold : fontRegular;
    DrawTextEx(font, text.c_str(), Vector2{ x, y }, size, 0.0f, color);
}

static void drawTextCentered(const string& text, float centerX, float y,
                             float size, Color color, bool bold = false) {
    drawText(text, centerX - measureText(text, size, bold) / 2.0f, y,
             size, color, bold);
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

static void drawButton(Rectangle rectangle, const string& label,
                       ButtonStyle style, bool enabled = true,
                       float fontSize = 16.0f) {
    bool hovered = enabled && buttonHovered(rectangle);

    Color fill = PANEL_ALT;
    Color hoverFill = { 45, 57, 80, 255 };
    Color border = BORDER;

    if (style == ButtonStyle::Primary) {
        fill = ACCENT;
        hoverFill = ACCENT_DARK;
        border = { 96, 165, 250, 255 };
    } else if (style == ButtonStyle::Danger) {
        fill = { 185, 28, 28, 255 };
        hoverFill = { 153, 27, 27, 255 };
        border = { 248, 113, 113, 255 };
    }

    if (!enabled) {
        fill = PANEL;
        border = BORDER;
    } else if (hovered) {
        fill = hoverFill;
    }

    DrawRectangleRounded(rectangle, 0.24f, 8, fill);
    DrawRectangleRoundedLinesEx(rectangle, 0.24f, 8, 1.0f, border);
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

struct ChainLayout {
    float startX;
    float nodeWidth;
    float gap;
};

static ChainLayout chainLayout(int count) {
    ChainLayout layout;
    layout.gap = NODE_GAP;

    if (count <= 0) {
        layout.nodeWidth = 150.0f;
        layout.startX = SCREEN_W / 2.0f - layout.nodeWidth / 2.0f;
        return layout;
    }

    float available = LIST_RIGHT - LIST_LEFT;
    float width = (available - layout.gap * (float)(count - 1)) / (float)count;
    layout.nodeWidth = clampf(width, 72.0f, 160.0f);

    float rowWidth = (float)count * layout.nodeWidth +
                     (float)(count - 1) * layout.gap;
    layout.startX = (SCREEN_W - rowWidth) / 2.0f;
    return layout;
}

static Rectangle nodeRect(int index, int count, const ChainLayout& layout) {
    (void)count;
    return Rectangle{
        layout.startX + (float)index * (layout.nodeWidth + layout.gap),
        NODE_Y,
        layout.nodeWidth,
        NODE_H
    };
}

static Rectangle headCard(int count) {
    ChainLayout layout = chainLayout(count);
    float centerX = count > 0
        ? layout.startX + layout.nodeWidth / 2.0f
        : SCREEN_W / 2.0f;

    return Rectangle{ centerX - 50.0f, HEAD_CARD_Y, 100.0f, 44.0f };
}

static Rectangle valueInputBox() {
    return Rectangle{ 24.0f, 108.0f, 120.0f, 42.0f };
}

static Rectangle positionInputBox() {
    return Rectangle{ 156.0f, 108.0f, 80.0f, 42.0f };
}

static Rectangle insertFrontButton() {
    return Rectangle{ 248.0f, 108.0f, 105.0f, 42.0f };
}

static Rectangle insertEndButton() {
    return Rectangle{ 361.0f, 108.0f, 100.0f, 42.0f };
}

static Rectangle insertAtButton() {
    return Rectangle{ 469.0f, 108.0f, 130.0f, 42.0f };
}

static Rectangle deleteValueButton() {
    return Rectangle{ 607.0f, 108.0f, 115.0f, 42.0f };
}

static Rectangle deletePositionButton() {
    return Rectangle{ 730.0f, 108.0f, 105.0f, 42.0f };
}

static Rectangle searchButton() {
    return Rectangle{ 843.0f, 108.0f, 85.0f, 42.0f };
}

static Rectangle reverseButton() {
    return Rectangle{ 936.0f, 108.0f, 95.0f, 42.0f };
}

static Rectangle clearButton() {
    return Rectangle{ 1039.0f, 108.0f, 100.0f, 42.0f };
}

static Rectangle lengthBadge() {
    return Rectangle{ SCREEN_W - 360.0f, 24.0f, 150.0f, 40.0f };
}

static Rectangle headBadge() {
    return Rectangle{ SCREEN_W - 190.0f, 24.0f, 166.0f, 40.0f };
}

static Rectangle stepBanner() {
    return Rectangle{ 24.0f, 158.0f, 1132.0f, 30.0f };
}

// ---------------------------------------------------------------------------
// Address display
// ---------------------------------------------------------------------------

static string hexAddress(node* ptr) {
    if (ptr == nullptr) {
        return "NULL";
    }

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "0x%04X",
             (unsigned int)(reinterpret_cast<uintptr_t>(ptr) & 0xFFFF));
    return string(buffer);
}

// ---------------------------------------------------------------------------
// List snapshots and transition
// ---------------------------------------------------------------------------

// The CLI has no function that returns the list contents. The UI performs a
// read-only traversal of head; all mutations still go through the functions.
static vector<NodeSnapshot> snapshotList() {
    vector<NodeSnapshot> result;
    if (head == NULL) {
        return result;
    }

    node* current = head;
    do {
        result.push_back(NodeSnapshot{ current, current->next, current->data });
        current = current->next;
    } while (current != head);

    return result;
}

static void refreshList() {
    app.list = snapshotList();
    app.length = (int)app.list.size();
    app.hasHead = (head != NULL);
    app.headValue = app.hasHead ? head->data : 0;
}

static void clearHighlight() {
    app.highlightIndex = -1;
    app.highlightTimer = 0.0f;
}

static void prepareTransition(const vector<NodeSnapshot>& oldList,
                              const vector<NodeSnapshot>& newList) {
    app.animNodes.clear();
    app.oldCount = (int)oldList.size();
    app.newCount = (int)newList.size();

    for (int i = 0; i < (int)oldList.size(); i++) {
        int newIndex = -1;
        for (int j = 0; j < (int)newList.size(); j++) {
            if (newList[j].ptr == oldList[i].ptr) {
                newIndex = j;
                break;
            }
        }

        node* displayNext = newIndex >= 0
            ? newList[newIndex].next
            : oldList[i].next;
        int displayValue = newIndex >= 0
            ? newList[newIndex].value
            : oldList[i].value;

        app.animNodes.push_back(AnimNode{
            oldList[i].ptr,
            displayNext,
            displayValue,
            i,
            newIndex,
            true,
            newIndex >= 0
        });
    }

    for (int j = 0; j < (int)newList.size(); j++) {
        bool found = false;
        for (int i = 0; i < (int)oldList.size(); i++) {
            if (oldList[i].ptr == newList[j].ptr) {
                found = true;
                break;
            }
        }

        if (!found) {
            app.animNodes.push_back(AnimNode{
                newList[j].ptr,
                newList[j].next,
                newList[j].value,
                -1,
                j,
                false,
                true
            });
        }
    }

    app.animT = 0.0f;
    app.animating = !app.animNodes.empty();
}

static void updateTransition(float dt) {
    if (!app.animating) {
        return;
    }

    app.animT += dt / ANIM_DURATION;
    if (app.animT >= 1.0f) {
        app.animT = 1.0f;
        app.animating = false;
    }
}

static bool busy() {
    return app.animating || app.stepAnimating;
}

// ---------------------------------------------------------------------------
// Teaching steps
// ---------------------------------------------------------------------------

static void updateStepHighlights();

static void startSteps(OpType type, const vector<string>& steps,
                       const string& finalMessage, Color finalColor) {
    app.steps = steps;
    app.stepIndex = steps.empty() ? -1 : 0;
    app.stepTimer = 0.0f;
    app.stepAnimating = !steps.empty();
    app.opType = type;
    app.finalStatus = finalMessage;
    app.finalStatusColor = finalColor;

    app.highlightFrom = nullptr;
    app.highlightTo = nullptr;
    app.highlightHeadArrow = false;
    app.highlightAllEdges = false;

    updateStepHighlights();

    if (app.stepAnimating) {
        setStatus("Step 1/" + to_string(app.steps.size()) + ": " +
                  app.steps[0], INFO);
    } else {
        setStatus(finalMessage, finalColor);
    }
}

static void updateStepHighlights() {
    app.highlightFrom = nullptr;
    app.highlightTo = nullptr;
    app.highlightHeadArrow = false;
    app.highlightAllEdges = false;

    if (!app.stepAnimating) {
        return;
    }

    int step = app.stepIndex;

    if (app.opType == OpType::Insert) {
        if (step == 1) {
            app.highlightFrom = app.opNewNode;
            app.highlightTo = app.opTargetNode;
        } else if (step == 2) {
            if (app.opPrevNode != nullptr) {
                app.highlightFrom = app.opPrevNode;
                app.highlightTo = app.opNewNode;
            } else {
                app.highlightHeadArrow = true;
            }
        }
    } else if (app.opType == OpType::Delete) {
        if (step == 1 || step == 2) {
            if (app.opPrevNode != nullptr) {
                app.highlightFrom = app.opPrevNode;
                app.highlightTo = app.opNextNode;
            } else {
                app.highlightHeadArrow = true;
            }
        }
    } else if (app.opType == OpType::Reverse) {
        if (step == 1) {
            app.highlightAllEdges = true;
        } else if (step == 2) {
            app.highlightHeadArrow = true;
        }
    }
}

static void finishSteps() {
    OpType finishedType = app.opType;
    int foundIndex = app.searchFoundIndex;

    app.stepAnimating = false;
    app.stepIndex = -1;
    app.stepTimer = 0.0f;
    app.steps.clear();
    app.opType = OpType::None;

    app.highlightFrom = nullptr;
    app.highlightTo = nullptr;
    app.highlightHeadArrow = false;
    app.highlightAllEdges = false;

    if (finishedType == OpType::Search) {
        if (foundIndex >= 0) {
            app.highlightIndex = foundIndex;
            app.highlightTimer = 1.8f;
        } else {
            clearHighlight();
        }
    }

    setStatus(app.finalStatus, app.finalStatusColor);

    app.searchFoundIndex = -1;
    app.searchCheckCount = 0;

    if (!app.animating) {
        app.animNodes.clear();
    }
}

static void updateSteps(float dt) {
    if (!app.stepAnimating) {
        return;
    }

    float duration = (app.opType == OpType::Search) ? 0.55f : 0.85f;
    app.stepTimer += dt;
    if (app.stepTimer < duration) {
        return;
    }

    app.stepTimer = 0.0f;

    if (app.stepIndex + 1 < (int)app.steps.size()) {
        app.stepIndex++;
        updateStepHighlights();
        setStatus("Step " + to_string(app.stepIndex + 1) + "/" +
                  to_string(app.steps.size()) + ": " +
                  app.steps[app.stepIndex], INFO);
    } else {
        finishSteps();
    }
}

static int searchCurrentIndex() {
    if (app.opType != OpType::Search || !app.stepAnimating) {
        return -1;
    }

    if (app.stepIndex == 0) {
        return app.length > 0 ? 0 : -1;
    }

    if (app.stepIndex >= 1 && app.stepIndex <= app.searchCheckCount) {
        return app.stepIndex - 1;
    }

    if (app.stepIndex == app.searchCheckCount + 1 &&
        app.searchFoundIndex >= 0) {
        return app.searchFoundIndex;
    }

    return -1;
}

static bool nodeHighlight(node* ptr, Color& color) {
    if (!app.stepAnimating) {
        return false;
    }

    if (app.opType == OpType::Insert) {
        if (ptr == app.opNewNode) {
            color = SUCCESS;
            return true;
        }
        if (app.stepIndex == 1 && ptr == app.opTargetNode) {
            color = INFO;
            return true;
        }
        if (app.stepIndex == 2 && ptr == app.opPrevNode) {
            color = ACCENT;
            return true;
        }
    } else if (app.opType == OpType::Delete) {
        if (ptr == app.opTargetNode) {
            color = ERROR;
            return true;
        }
        if (app.stepIndex == 1 &&
            (ptr == app.opPrevNode || ptr == app.opNextNode)) {
            color = ACCENT;
            return true;
        }
    } else if (app.opType == OpType::Reverse) {
        if (app.stepIndex == 1) {
            color = ACCENT;
            return true;
        }
        if (app.stepIndex == 2 && ptr == app.list[0].ptr) {
            color = HEAD_COLOR;
            return true;
        }
    } else if (app.opType == OpType::Clear) {
        color = ERROR;
        return true;
    } else if (app.opType == OpType::Search) {
        int index = searchCurrentIndex();
        if (index >= 0 && index < (int)app.list.size() &&
            ptr == app.list[index].ptr) {
            color = SUCCESS;
            return true;
        }
    }

    return false;
}

static bool edgeHighlighted(node* from, node* to) {
    return app.highlightFrom != nullptr && app.highlightTo != nullptr &&
           app.highlightFrom == from && app.highlightTo == to;
}

// ---------------------------------------------------------------------------
// Operation step builders
// ---------------------------------------------------------------------------

static void beginInsertSteps(int value, const string& finalMessage) {
    app.opNewNode = nullptr;
    app.opPrevNode = nullptr;
    app.opTargetNode = nullptr;
    app.opNewIndex = -1;

    for (const AnimNode& animated : app.animNodes) {
        if (!animated.hadOld && animated.hasNew) {
            app.opNewNode = animated.ptr;
            app.opNewIndex = animated.newIndex;
            break;
        }
    }

    if (app.newCount > 0 && app.opNewNode != nullptr) {
        app.opTargetNode = app.list[(app.opNewIndex + 1) % app.newCount].ptr;
        app.opPrevNode = app.opNewIndex > 0
            ? app.list[app.opNewIndex - 1].ptr
            : nullptr;
    }

    vector<string> steps;
    steps.push_back("Create a new node with data = " + to_string(value) + ".");

    if (app.opTargetNode == app.opNewNode) {
        steps.push_back("newNode->next = newNode (single-node self-loop).");
    } else {
        steps.push_back("newNode->next = " + hexAddress(app.opTargetNode) + ".");
    }

    if (app.opPrevNode != nullptr) {
        steps.push_back("prev->next = newNode.");
    } else {
        steps.push_back("HEAD = newNode.");
    }

    startSteps(OpType::Insert, steps, finalMessage, SUCCESS);
}

static void beginDeleteSteps(const vector<NodeSnapshot>& oldList,
                             int targetIndex, const string& finalMessage) {
    app.opNewNode = nullptr;
    app.opPrevNode = nullptr;
    app.opTargetNode = nullptr;
    app.opNextNode = nullptr;

    if (targetIndex < 0 || targetIndex >= (int)oldList.size()) {
        startSteps(OpType::None, {}, finalMessage, SUCCESS);
        return;
    }

    app.opTargetNode = oldList[targetIndex].ptr;
    app.opNextNode = oldList[targetIndex].next;
    app.opPrevNode = targetIndex > 0
        ? oldList[targetIndex - 1].ptr
        : nullptr;

    vector<string> steps;
    steps.push_back("Find target node " + to_string(targetIndex) +
                    " (data = " + to_string(oldList[targetIndex].value) + ").");

    if (oldList.size() == 1) {
        steps.push_back("HEAD = NULL.");
    } else if (app.opPrevNode != nullptr) {
        steps.push_back("prev->next = target->next (" +
                        hexAddress(app.opNextNode) + ").");
    } else {
        steps.push_back("HEAD = target->next (" +
                        hexAddress(app.opNextNode) + ").");
    }

    steps.push_back("Delete the target node.");

    startSteps(OpType::Delete, steps, finalMessage, SUCCESS);
}

static void beginReverseSteps(const string& finalMessage) {
    vector<string> steps;
    steps.push_back("prev = NULL; curr = head.");
    steps.push_back("Reverse each next pointer.");
    steps.push_back("HEAD = old last node (" +
                    hexAddress(app.hasHead ? app.list[0].ptr : nullptr) + ").");

    startSteps(OpType::Reverse, steps, finalMessage, SUCCESS);
}

static void beginClearSteps(const string& finalMessage) {
    vector<string> steps;
    steps.push_back("Delete every node.");
    steps.push_back("HEAD = NULL.");

    startSteps(OpType::Clear, steps, finalMessage, INFO);
}

static void beginSearchSteps(int value, int foundIndex, int checkCount,
                             const string& finalMessage, Color finalColor) {
    app.searchFoundIndex = foundIndex;
    app.searchCheckCount = checkCount;
    app.searchValue = value;

    vector<string> steps;
    steps.push_back("curr = head.");

    for (int i = 0; i < checkCount; i++) {
        steps.push_back("Check node " + to_string(i) + " (data = " +
                        to_string(app.list[i].value) + ").");
    }

    steps.push_back(finalMessage);

    startSteps(OpType::Search, steps, finalMessage, finalColor);
}

// ---------------------------------------------------------------------------
// Operations
// ---------------------------------------------------------------------------

static bool parseValue(int& value) {
    if (app.valueInput.empty() || app.valueInput == "-") {
        setStatus("Enter a value first.", ERROR);
        app.activeField = Field::Value;
        return false;
    }

    value = atoi(app.valueInput.c_str());
    return true;
}

static bool parsePosition(int& position) {
    if (app.positionInput.empty()) {
        setStatus("Enter a position first.", ERROR);
        app.activeField = Field::Position;
        return false;
    }

    position = atoi(app.positionInput.c_str());
    return true;
}

static void doInsertFront() {
    int value = 0;
    if (!parseValue(value)) return;

    if (app.length >= MAX_NODES) {
        setStatus("Visual limit is 10 nodes.", ERROR);
        return;
    }

    vector<NodeSnapshot> oldList = app.list;
    insert_fotl(value);
    refreshList();
    prepareTransition(oldList, app.list);
    clearHighlight();

    app.valueInput.clear();
    app.valueFresh = false;
    app.activeField = Field::Value;

    string finalMessage = "Inserted " + to_string(value) +
                          " at the front. Length: " +
                          to_string(app.length) + ".";
    beginInsertSteps(value, finalMessage);
}

static void doInsertEnd() {
    int value = 0;
    if (!parseValue(value)) return;

    if (app.length >= MAX_NODES) {
        setStatus("Visual limit is 10 nodes.", ERROR);
        return;
    }

    vector<NodeSnapshot> oldList = app.list;
    insert_eotl(value);
    refreshList();
    prepareTransition(oldList, app.list);
    clearHighlight();

    app.valueInput.clear();
    app.valueFresh = false;
    app.activeField = Field::Value;

    string finalMessage = "Inserted " + to_string(value) +
                          " at the end. Length: " +
                          to_string(app.length) + ".";
    beginInsertSteps(value, finalMessage);
}

static void doInsertAtPosition() {
    int value = 0;
    int position = 0;
    if (!parseValue(value) || !parsePosition(position)) return;

    if (app.length >= MAX_NODES) {
        setStatus("Visual limit is 10 nodes.", ERROR);
        return;
    }

    int length = get_length();
    if (position < 0 || position > length) {
        setStatus("Insert position must be between 0 and " +
                  to_string(length) + ".", ERROR);
        return;
    }

    vector<NodeSnapshot> oldList = app.list;
    insert_at_position(position, value);
    refreshList();
    prepareTransition(oldList, app.list);
    clearHighlight();

    app.valueInput.clear();
    app.valueFresh = false;
    app.activeField = Field::Value;

    string finalMessage = "Inserted " + to_string(value) +
                          " at position " + to_string(position) +
                          ". Length: " + to_string(app.length) + ".";
    beginInsertSteps(value, finalMessage);
}

static void doDeleteValue() {
    int value = 0;
    if (!parseValue(value)) return;

    if (app.length == 0) {
        setStatus("List is empty.", ERROR);
        return;
    }

    int index = search(value);
    if (index == -1) {
        setStatus("Value " + to_string(value) + " was not found.", ERROR);
        clearHighlight();
        return;
    }

    vector<NodeSnapshot> oldList = app.list;
    Delete(value);
    refreshList();
    prepareTransition(oldList, app.list);
    clearHighlight();

    string finalMessage = "Deleted the first occurrence of " +
                          to_string(value) + " at index " +
                          to_string(index) + ". Length: " +
                          to_string(app.length) + ".";
    beginDeleteSteps(oldList, index, finalMessage);
}

static void doDeleteAtPosition() {
    int position = 0;
    if (!parsePosition(position)) return;

    if (app.length == 0) {
        setStatus("List is empty.", ERROR);
        return;
    }

    int length = get_length();
    if (position < 0 || position >= length) {
        setStatus("Delete position must be between 0 and " +
                  to_string(length - 1) + ".", ERROR);
        return;
    }

    vector<NodeSnapshot> oldList = app.list;
    delete_at_position(position);
    refreshList();
    prepareTransition(oldList, app.list);
    clearHighlight();

    string finalMessage = "Deleted the node at position " +
                          to_string(position) + ". Length: " +
                          to_string(app.length) + ".";
    beginDeleteSteps(oldList, position, finalMessage);
}

static void doSearch() {
    int value = 0;
    if (!parseValue(value)) return;

    app.highlightIndex = -1;
    app.highlightTimer = 0.0f;

    if (app.length == 0) {
        beginSearchSteps(value, -1, 0, "List is empty.", ERROR);
        return;
    }

    int index = search(value);
    int checkCount = index >= 0 ? index + 1 : app.length;

    if (index == -1) {
        beginSearchSteps(value, -1, checkCount,
                         "Reached head again; value " + to_string(value) +
                         " was not found.", ERROR);
    } else {
        beginSearchSteps(value, index, checkCount,
                         "Found " + to_string(value) + " at index " +
                         to_string(index) + ".", SUCCESS);
    }
}

static void doReverse() {
    if (app.length == 0) {
        setStatus("List is empty.", ERROR);
        return;
    }

    if (app.length == 1) {
        setStatus("A single node is already its own reverse.", INFO);
        return;
    }

    vector<NodeSnapshot> oldList = app.list;
    revers();
    refreshList();
    prepareTransition(oldList, app.list);
    clearHighlight();

    string finalMessage = "Reversed the list. New head: " +
                          to_string(app.headValue) + ".";
    beginReverseSteps(finalMessage);
}

static void doClear() {
    if (app.length == 0) {
        setStatus("List is already empty.", INFO);
        return;
    }

    vector<NodeSnapshot> oldList = app.list;
    while (get_length() > 0) {
        delete_at_position(0);
    }
    refreshList();
    prepareTransition(oldList, app.list);
    clearHighlight();

    beginClearSteps("Cleared the list.");
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

static void updateTextInput() {
    int character = 0;
    while ((character = GetCharPressed()) > 0) {
        if (app.activeField == Field::Value) {
            bool digit = character >= '0' && character <= '9';
            bool minus = character == '-' &&
                         (app.valueFresh || app.valueInput.empty());

            if (digit || minus) {
                if (app.valueFresh) {
                    app.valueInput.clear();
                    app.valueFresh = false;
                }
                if (app.valueInput.size() < 6) {
                    app.valueInput.push_back((char)character);
                }
            }
        } else if (app.activeField == Field::Position) {
            if (character >= '0' && character <= '9') {
                if (app.positionFresh) {
                    app.positionInput.clear();
                    app.positionFresh = false;
                }
                if (app.positionInput.size() < 2) {
                    app.positionInput.push_back((char)character);
                }
            }
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (app.activeField == Field::Value) {
            if (app.valueFresh) {
                app.valueInput.clear();
                app.valueFresh = false;
            } else if (!app.valueInput.empty()) {
                app.valueInput.pop_back();
            }
        } else if (app.activeField == Field::Position) {
            if (app.positionFresh) {
                app.positionInput.clear();
                app.positionFresh = false;
            } else if (!app.positionInput.empty()) {
                app.positionInput.pop_back();
            }
        }
    }
}

static void updateKeyboardShortcuts() {
    if (IsKeyPressed(KEY_TAB)) {
        if (app.activeField == Field::Value) {
            app.activeField = Field::Position;
            app.positionFresh = true;
        } else {
            app.activeField = Field::Value;
            app.valueFresh = true;
        }
    }

    if (IsKeyPressed(KEY_ENTER) && !busy()) {
        if (app.activeField == Field::Value) {
            doInsertEnd();
        } else if (app.activeField == Field::Position) {
            doInsertAtPosition();
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (app.activeField != Field::None) {
            app.activeField = Field::None;
        } else {
            app.quit = true;
        }
    }
}

static void updateHoveredNode() {
    app.hoveredNode = -1;

    if (busy() || app.length == 0) {
        return;
    }

    Vector2 mouse = GetMousePosition();
    ChainLayout layout = chainLayout(app.length);

    for (int i = 0; i < app.length; i++) {
        Rectangle rect = nodeRect(i, app.length, layout);
        if (CheckCollisionPointRec(mouse, rect)) {
            app.hoveredNode = i;
            break;
        }
    }
}

static void handleButtons() {
    if (busy()) {
        return;
    }

    if (buttonClicked(insertFrontButton()) && app.length < MAX_NODES) {
        doInsertFront();
        return;
    }
    if (buttonClicked(insertEndButton()) && app.length < MAX_NODES) {
        doInsertEnd();
        return;
    }
    if (buttonClicked(insertAtButton()) && app.length < MAX_NODES) {
        doInsertAtPosition();
        return;
    }
    if (buttonClicked(deleteValueButton()) && app.length > 0) {
        doDeleteValue();
        return;
    }
    if (buttonClicked(deletePositionButton()) && app.length > 0) {
        doDeleteAtPosition();
        return;
    }
    if (buttonClicked(searchButton()) && app.length > 0) {
        doSearch();
        return;
    }
    if (buttonClicked(reverseButton()) && app.length > 1) {
        doReverse();
        return;
    }
    if (buttonClicked(clearButton()) && app.length > 0) {
        doClear();
        return;
    }
}

static void handleFieldClicks() {
    if (buttonClicked(valueInputBox())) {
        app.activeField = Field::Value;
        app.valueFresh = true;
        return;
    }

    if (buttonClicked(positionInputBox())) {
        app.activeField = Field::Position;
        app.positionFresh = true;
        return;
    }
}

static void handleNodeClick() {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || app.hoveredNode < 0) {
        return;
    }

    int index = app.hoveredNode;
    if (index < 0 || index >= (int)app.list.size()) {
        return;
    }

    app.valueInput = to_string(app.list[index].value);
    app.activeField = Field::Value;
    app.valueFresh = false;
    setStatus("Selected node " + to_string(index) + " (value " +
              to_string(app.list[index].value) + ").", INFO);
}

static void updateCursor() {
    Vector2 mouse = GetMousePosition();

    if (pointInRectangle(mouse, valueInputBox()) ||
        pointInRectangle(mouse, positionInputBox())) {
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
        return;
    }

    bool canOperate = !busy();
    bool overEnabledButton =
        (canOperate && app.length < MAX_NODES &&
         (buttonHovered(insertFrontButton()) ||
          buttonHovered(insertEndButton()) ||
          buttonHovered(insertAtButton()))) ||
        (canOperate && app.length > 0 &&
         (buttonHovered(deleteValueButton()) ||
          buttonHovered(deletePositionButton()) ||
          buttonHovered(searchButton()) ||
          buttonHovered(clearButton()))) ||
        (canOperate && app.length > 1 && buttonHovered(reverseButton()));

    if (overEnabledButton || app.hoveredNode >= 0) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
}

static void update() {
    float dt = GetFrameTime();

    updateTransition(dt);
    updateSteps(dt);

    if (app.highlightTimer > 0.0f) {
        app.highlightTimer -= dt;
        if (app.highlightTimer <= 0.0f) {
            app.highlightTimer = 0.0f;
            app.highlightIndex = -1;
        }
    }

    if (!app.animating && !app.stepAnimating) {
        app.animNodes.clear();
    }

    updateTextInput();
    updateKeyboardShortcuts();
    updateHoveredNode();
    handleButtons();
    handleFieldClicks();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        !buttonHovered(insertFrontButton()) &&
        !buttonHovered(insertEndButton()) &&
        !buttonHovered(insertAtButton()) &&
        !buttonHovered(deleteValueButton()) &&
        !buttonHovered(deletePositionButton()) &&
        !buttonHovered(searchButton()) &&
        !buttonHovered(reverseButton()) &&
        !buttonHovered(clearButton()) &&
        !pointInRectangle(GetMousePosition(), valueInputBox()) &&
        !pointInRectangle(GetMousePosition(), positionInputBox())) {
        if (app.hoveredNode >= 0) {
            handleNodeClick();
        } else {
            app.activeField = Field::None;
        }
    }

    updateCursor();
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

static void drawStatBadge(Rectangle rectangle, const string& label,
                          const string& value, Color valueColor) {
    DrawRectangleRounded(rectangle, 0.28f, 8, PANEL_ALT);
    DrawRectangleRoundedLinesEx(rectangle, 0.28f, 8, 1.0f, BORDER);
    drawText(label, rectangle.x + 12.0f, rectangle.y + 6.0f, 11.0f, MUTED, true);
    drawText(value, rectangle.x + 12.0f, rectangle.y + 20.0f, 17.0f,
             valueColor, true);
}

static void drawTopBar() {
    DrawRectangle(0, 0, SCREEN_W, (int)TOP_BAR_H, PANEL);
    DrawLine(0, (int)TOP_BAR_H, SCREEN_W, (int)TOP_BAR_H, BORDER);

    drawText("Circular Linked List", 24.0f, 14.0f, 30.0f, TEXT, true);
    drawText("Singly linked  |  data + next pointer  |  last node points to head",
             24.0f, 50.0f, 16.0f, MUTED);

    drawStatBadge(lengthBadge(), "LENGTH", to_string(app.length), TEXT);
    drawStatBadge(headBadge(), "HEAD",
                  app.hasHead ? hexAddress(app.list[0].ptr) : "NULL",
                  app.hasHead ? HEAD_COLOR : MUTED);
}

static void drawInputField(Rectangle box, const string& label,
                           const string& text, bool active,
                           const string& placeholder) {
    drawText(label, box.x, box.y - 18.0f, 12.0f, MUTED, true);

    DrawRectangleRounded(box, 0.18f, 10, PANEL_ALT);
    DrawRectangleRoundedLinesEx(box, 0.18f, 10, 2.0f,
                                active ? ACCENT : BORDER);

    string shown = text.empty() ? placeholder : text;
    Color shownColor = text.empty() ? MUTED : TEXT;
    drawText(shown, box.x + 14.0f, box.y + 8.0f, 22.0f, shownColor, true);

    if (active && fmod(GetTime(), 1.0) < 0.5) {
        float textWidth = measureText(text, 22.0f, true);
        DrawRectangle((int)(box.x + 14.0f + textWidth + 2.0f),
                      (int)(box.y + 10.0f), 2, 20, ACCENT);
    }
}

static void drawStepBanner() {
    Rectangle banner = stepBanner();
    bool active = app.stepAnimating;

    DrawRectangleRounded(banner, 0.3f, 8, PANEL_ALT);
    DrawRectangleRoundedLinesEx(banner, 0.3f, 8, 1.0f,
                                active ? ACCENT : BORDER);

    if (active && app.stepIndex >= 0 &&
        app.stepIndex < (int)app.steps.size()) {
        string stepLabel = "STEP " + to_string(app.stepIndex + 1) + "/" +
                           to_string(app.steps.size());
        drawText(stepLabel, banner.x + 12.0f, banner.y + 8.0f, 12.0f,
                 ACCENT, true);

        float labelWidth = measureText(stepLabel, 12.0f, true);
        drawText(app.steps[app.stepIndex], banner.x + 12.0f + labelWidth + 18.0f,
                 banner.y + 7.0f, 14.0f, TEXT, true);

        float dotX = banner.x + banner.width - 22.0f;
        for (int i = (int)app.steps.size() - 1; i >= 0; i--) {
            Color dotColor = i <= app.stepIndex ? ACCENT : BORDER;
            DrawCircle((int)dotX, (int)(banner.y + banner.height / 2.0f),
                       3.5f, dotColor);
            dotX -= 14.0f;
        }
    } else {
        drawText("Ready. Choose an operation to see its pointer steps.",
                 banner.x + 12.0f, banner.y + 7.0f, 14.0f, MUTED);
    }
}

static void drawControls() {
    DrawRectangle(0, (int)TOP_BAR_H, SCREEN_W, (int)CONTROLS_H, PANEL);
    DrawLine(0, (int)(TOP_BAR_H + CONTROLS_H),
             SCREEN_W, (int)(TOP_BAR_H + CONTROLS_H), BORDER);

    drawInputField(valueInputBox(), "VALUE", app.valueInput,
                   app.activeField == Field::Value, "value");
    drawInputField(positionInputBox(), "POSITION", app.positionInput,
                   app.activeField == Field::Position, "pos");

    bool canOperate = !busy();
    bool canInsert = canOperate && app.length < MAX_NODES;
    bool hasNodes = canOperate && app.length > 0;

    drawButton(insertFrontButton(), "Insert Front",
               ButtonStyle::Primary, canInsert, 14.0f);
    drawButton(insertEndButton(), "Insert End",
               ButtonStyle::Primary, canInsert, 14.0f);
    drawButton(insertAtButton(), "Insert at Pos",
               ButtonStyle::Primary, canInsert, 14.0f);
    drawButton(deleteValueButton(), "Delete Value",
               ButtonStyle::Danger, hasNodes, 14.0f);
    drawButton(deletePositionButton(), "Delete Pos",
               ButtonStyle::Danger, hasNodes, 14.0f);
    drawButton(searchButton(), "Search",
               ButtonStyle::Default, hasNodes, 14.0f);
    drawButton(reverseButton(), "Reverse",
               ButtonStyle::Default, canOperate && app.length > 1, 14.0f);
    drawButton(clearButton(), "Clear",
               ButtonStyle::Danger, hasNodes, 14.0f);

    drawStepBanner();
}

static void drawArrowHead(Vector2 tip, Vector2 direction, float size, Color color) {
    Vector2 dir = normalizeVector(direction);
    Vector2 normal = Vector2{ -dir.y, dir.x };
    Vector2 base = Vector2{ tip.x - dir.x * size, tip.y - dir.y * size };

    DrawTriangle(
        tip,
        Vector2{ base.x + normal.x * size * 0.55f,
                 base.y + normal.y * size * 0.55f },
        Vector2{ base.x - normal.x * size * 0.55f,
                 base.y - normal.y * size * 0.55f },
        color);
}

static void drawStraightArrow(Vector2 from, Vector2 to, Color color,
                              float thickness, float arrowSize) {
    DrawLineEx(from, to, thickness, color);
    drawArrowHead(to, Vector2{ to.x - from.x, to.y - from.y },
                  arrowSize, color);
}

static void drawDashedLine(Vector2 from, Vector2 to, Color color,
                           float thickness, float dash, float gap) {
    Vector2 direction = normalizeVector(Vector2{ to.x - from.x, to.y - from.y });
    float length = sqrtf((to.x - from.x) * (to.x - from.x) +
                         (to.y - from.y) * (to.y - from.y));
    float position = 0.0f;

    while (position < length) {
        float end = min(position + dash, length);
        Vector2 a = Vector2{ from.x + direction.x * position,
                             from.y + direction.y * position };
        Vector2 b = Vector2{ from.x + direction.x * end,
                             from.y + direction.y * end };
        DrawLineEx(a, b, thickness, color);
        position += dash + gap;
    }
}

static void drawNodeCard(Rectangle rect, int value, node* next, int index,
                         bool isHead, bool isNew, bool isRemoved,
                         Color highlight, float alpha, bool showIndex) {
    if (rect.width < 2.0f || alpha <= 0.01f) {
        return;
    }

    Color fill = PANEL_ALT;
    Color border = BORDER;

    if (isHead) {
        fill = Color{ 45, 40, 18, 255 };
        border = HEAD_COLOR;
    }
    if (isRemoved) {
        fill = Color{ 60, 22, 22, 255 };
        border = ERROR;
    }
    if (isNew) {
        border = SUCCESS;
    }
    if (highlight.a > 0) {
        border = highlight;
    }

    DrawRectangleRounded(rect, 0.14f, 10, Fade(fill, alpha));
    DrawRectangleRoundedLinesEx(rect, 0.14f, 10, isHead || highlight.a > 0 ? 2.5f : 1.5f,
                                Fade(border, alpha));

    if (isHead || highlight.a > 0 || isNew) {
        DrawRectangleRounded(Rectangle{ rect.x - 3.0f, rect.y - 3.0f,
                                        rect.width + 6.0f, rect.height + 6.0f },
                             0.14f, 10, Fade(border, 0.16f * alpha));
    }

    float dataWidth = rect.width * 0.58f;
    DrawLineEx(Vector2{ rect.x + dataWidth, rect.y + 8.0f },
               Vector2{ rect.x + dataWidth, rect.y + rect.height - 8.0f },
               1.0f, Fade(BORDER, alpha));

    drawText("data", rect.x + 8.0f, rect.y + 7.0f, 9.5f,
             Fade(MUTED, alpha), true);
    drawText("next", rect.x + dataWidth + 6.0f, rect.y + 7.0f, 9.5f,
             Fade(MUTED, alpha), true);

    string valueText = to_string(value);
    float valueSize = 20.0f;
    float valueWidth = measureText(valueText, valueSize, true);
    float maxValueWidth = dataWidth - 12.0f;
    if (valueWidth > maxValueWidth) {
        valueSize *= maxValueWidth / valueWidth;
    }

    Color valueColor = isHead ? HEAD_COLOR : TEXT;
    if (isRemoved) {
        valueColor = ERROR;
    }

    drawTextCentered(valueText, rect.x + dataWidth / 2.0f,
                     rect.y + 34.0f, valueSize, Fade(valueColor, alpha), true);

    string addressText = hexAddress(next);
    float addressSize = 11.5f;
    float addressWidth = measureText(addressText, addressSize, true);
    float maxAddressWidth = rect.width - dataWidth - 10.0f;
    if (addressWidth > maxAddressWidth) {
        addressSize *= maxAddressWidth / addressWidth;
    }

    drawTextCentered(addressText, rect.x + dataWidth +
                     (rect.width - dataWidth) / 2.0f,
                     rect.y + 42.0f, addressSize, Fade(MUTED, alpha), true);

    if (showIndex) {
        drawTextCentered(to_string(index), rect.x + rect.width / 2.0f,
                         rect.y - 18.0f, 13.0f, Fade(MUTED, alpha), true);
    }
}

static void drawHeadCard() {
    Rectangle card = headCard(app.length);
    bool hasNode = app.hasHead;
    Color borderColor = app.highlightHeadArrow
        ? ACCENT
        : (hasNode ? HEAD_COLOR : BORDER);

    DrawRectangleRounded(card, 0.25f, 10, PANEL_ALT);
    DrawRectangleRoundedLinesEx(card, 0.25f, 10,
                                app.highlightHeadArrow ? 2.5f : 1.5f,
                                borderColor);

    drawTextCentered("HEAD", card.x + card.width / 2.0f,
                     card.y + 7.0f, 12.0f, borderColor, true);
    drawTextCentered(hasNode ? hexAddress(app.list[0].ptr) : "NULL",
                     card.x + card.width / 2.0f, card.y + 23.0f,
                     10.0f, hasNode ? TEXT : MUTED, true);

    if (hasNode) {
        ChainLayout layout = chainLayout(app.length);
        Rectangle first = nodeRect(0, app.length, layout);
        Vector2 from = Vector2{ card.x + card.width / 2.0f, card.y };
        Vector2 to = Vector2{ first.x + first.width / 2.0f,
                              first.y + first.height + 10.0f };
        drawStraightArrow(from, to,
                          app.highlightHeadArrow ? ACCENT : HEAD_COLOR,
                          app.highlightHeadArrow ? 3.5f : 2.0f, 10.0f);
    }
}

static void drawListArrows(float alpha) {
    int count = (int)app.list.size();
    if (count == 0 || alpha <= 0.01f) {
        return;
    }

    ChainLayout layout = chainLayout(count);

    for (int i = 0; i + 1 < count; i++) {
        Rectangle fromRect = nodeRect(i, count, layout);
        Rectangle toRect = nodeRect(i + 1, count, layout);
        Vector2 from = Vector2{ fromRect.x + fromRect.width + 2.0f,
                                fromRect.y + fromRect.height / 2.0f };
        Vector2 to = Vector2{ toRect.x - 9.0f,
                              toRect.y + toRect.height / 2.0f };

        bool highlighted = app.highlightAllEdges ||
            edgeHighlighted(app.list[i].ptr, app.list[i + 1].ptr);
        Color color = highlighted ? SUCCESS : EDGE_COLOR;

        drawStraightArrow(from, to, Fade(color, alpha),
                          highlighted ? 3.5f : 2.2f, 9.0f);
    }
}

// Drawn after the HEAD card so the arrowhead stays visible.
static void drawWrapArrow(float alpha) {
    int count = (int)app.list.size();
    if (count == 0 || alpha <= 0.01f) {
        return;
    }

    ChainLayout layout = chainLayout(count);
    Rectangle lastRect = nodeRect(count - 1, count, layout);
    Rectangle card = headCard(count);

    Vector2 start = Vector2{ lastRect.x + lastRect.width / 2.0f,
                             lastRect.y + lastRect.height };
    Vector2 corner1 = Vector2{ start.x, BOTTOM_ROUTE_Y };
    Vector2 corner2 = Vector2{ card.x + card.width, BOTTOM_ROUTE_Y };
    Vector2 end = Vector2{ card.x + card.width,
                           card.y + card.height / 2.0f };

    bool loopHighlighted = app.highlightAllEdges ||
        edgeHighlighted(app.list[count - 1].ptr, app.list[0].ptr);

    Color loopColor = Fade(loopHighlighted ? SUCCESS : EDGE_COLOR, alpha);

    drawDashedLine(start, corner1, loopColor,
                   loopHighlighted ? 3.0f : 2.0f, 8.0f, 6.0f);
    drawDashedLine(corner1, corner2, loopColor,
                   loopHighlighted ? 3.0f : 2.0f, 8.0f, 6.0f);
    drawDashedLine(corner2, end, loopColor,
                   loopHighlighted ? 3.0f : 2.0f, 8.0f, 6.0f);

    drawArrowHead(end, Vector2{ 0.0f, 1.0f }, 9.0f, loopColor);

    drawText("last->next = head", (corner1.x + corner2.x) / 2.0f - 55.0f,
             BOTTOM_ROUTE_Y - 18.0f, 11.0f, Fade(MUTED, 0.9f * alpha), true);
}

static Rectangle animatedRect(const AnimNode& animated,
                              const ChainLayout& oldLayout,
                              const ChainLayout& newLayout,
                              float& scale) {
    scale = 1.0f;

    if (animated.hadOld && animated.hasNew) {
        Rectangle oldRect = nodeRect(animated.oldIndex, app.oldCount, oldLayout);
        Rectangle newRect = nodeRect(animated.newIndex, app.newCount, newLayout);
        float t = easeInOutCubic(clampf(app.animT, 0.0f, 1.0f));
        Vector2 oldCenter = Vector2{ oldRect.x + oldRect.width / 2.0f,
                                     oldRect.y + oldRect.height / 2.0f };
        Vector2 newCenter = Vector2{ newRect.x + newRect.width / 2.0f,
                                     newRect.y + newRect.height / 2.0f };
        Vector2 center = lerpVector(oldCenter, newCenter, t);
        float width = lerpFloat(oldRect.width, newRect.width, t);
        return Rectangle{ center.x - width / 2.0f, NODE_Y, width, NODE_H };
    }

    if (animated.hasNew) {
        Rectangle newRect = nodeRect(animated.newIndex, app.newCount, newLayout);
        scale = 0.25f + 0.75f * easeOutCubic(app.animT);
        float width = newRect.width * scale;
        float height = newRect.height * scale;
        return Rectangle{
            newRect.x + (newRect.width - width) / 2.0f,
            newRect.y + (newRect.height - height) / 2.0f,
            width,
            height
        };
    }

    Rectangle oldRect = nodeRect(animated.oldIndex, app.oldCount, oldLayout);
    scale = 1.0f - 0.25f * easeInCubic(app.animT);
    float width = oldRect.width * scale;
    float height = oldRect.height * scale;
    return Rectangle{
        oldRect.x + (oldRect.width - width) / 2.0f,
        oldRect.y + (oldRect.height - height) / 2.0f,
        width,
        height
    };
}

static void drawAnimatedNodes() {
    ChainLayout oldLayout = chainLayout(app.oldCount);
    ChainLayout newLayout = chainLayout(app.newCount);

    for (const AnimNode& animated : app.animNodes) {
        float scale = 1.0f;
        Rectangle rect = animatedRect(animated, oldLayout, newLayout, scale);

        bool deletedTarget = animated.hadOld && !animated.hasNew &&
                             app.opType == OpType::Delete &&
                             animated.ptr == app.opTargetNode;
        if (deletedTarget) {
            // Keep the deleted node above the collapsed row so it does not
            // cover the node that now occupies its old position.
            rect.y -= NODE_H + 22.0f;
        }

        float alpha = 1.0f;
        if (!animated.hadOld && animated.hasNew) {
            alpha = app.animT;
        } else if (animated.hadOld && !animated.hasNew) {
            if (app.opType == OpType::Delete &&
                animated.ptr == app.opTargetNode) {
                // Keep the removed node visible while the pointer steps are
                // being explained; fade it only on the final step.
                float duration = 0.85f;
                if (app.stepAnimating &&
                    app.stepIndex == (int)app.steps.size() - 1) {
                    alpha = 1.0f - clampf(app.stepTimer / duration, 0.0f, 1.0f);
                } else {
                    alpha = 1.0f;
                }
            } else {
                alpha = 1.0f - app.animT;
            }
        }

        Color highlight = NO_HIGHLIGHT;
        nodeHighlight(animated.ptr, highlight);

        drawNodeCard(rect, animated.value, animated.next,
                     animated.hasNew ? animated.newIndex : animated.oldIndex,
                     animated.hasNew && animated.newIndex == 0,
                     !animated.hadOld && animated.hasNew,
                     animated.hadOld && !animated.hasNew,
                     highlight, alpha, animated.hasNew);
    }
}

static void drawIdleNodes() {
    if (app.list.empty()) {
        return;
    }

    ChainLayout layout = chainLayout(app.length);
    int searchIndex = searchCurrentIndex();

    for (int i = 0; i < app.length; i++) {
        Rectangle rect = nodeRect(i, app.length, layout);
        Rectangle drawRect = rect;

        float scale = 1.0f;
        if (app.highlightIndex == i && app.highlightTimer > 0.0f) {
            scale = 1.0f + 0.03f * (0.5f + 0.5f * sinf(GetTime() * 7.0f));
        }

        if (scale != 1.0f) {
            drawRect.width = rect.width * scale;
            drawRect.height = rect.height * scale;
            drawRect.x = rect.x + (rect.width - drawRect.width) / 2.0f;
            drawRect.y = rect.y + (rect.height - drawRect.height) / 2.0f;
        }

        Color highlight = NO_HIGHLIGHT;
        if (searchIndex == i || app.highlightIndex == i) {
            highlight = SUCCESS;
        }
        if (i == app.hoveredNode && highlight.a == 0) {
            highlight = NODE_HOVER;
        }
        if (i == 0 && highlight.a == 0) {
            highlight = HEAD_COLOR;
        }

        drawNodeCard(drawRect, app.list[i].value, app.list[i].next, i,
                     i == 0, false, false, highlight, 1.0f, true);
    }
}

static void drawEmptyState() {
    Vector2 center = Vector2{ SCREEN_W / 2.0f, 420.0f };
    Rectangle card = headCard(0);

    DrawRectangleRounded(card, 0.25f, 10, PANEL_ALT);
    DrawRectangleRoundedLinesEx(card, 0.25f, 10, 1.5f, BORDER);
    drawTextCentered("HEAD", card.x + card.width / 2.0f,
                     card.y + 7.0f, 12.0f, HEAD_COLOR, true);
    drawTextCentered("NULL", card.x + card.width / 2.0f,
                     card.y + 23.0f, 10.0f, MUTED, true);

    drawTextCentered("list is empty", center.x, center.y - 52.0f,
                     28.0f, MUTED, true);
    drawTextCentered("Insert a node from the controls above.",
                     center.x, center.y - 10.0f, 15.0f, MUTED);
}

static void drawListScene() {
    if (app.animNodes.empty()) {
        if (app.list.empty()) {
            drawEmptyState();
        } else {
            drawListArrows(1.0f);
            drawIdleNodes();
            drawHeadCard();
            drawWrapArrow(1.0f);
        }
        return;
    }

    float arrowAlpha = app.animating
        ? clampf((app.animT - 0.45f) / 0.55f, 0.0f, 1.0f)
        : 1.0f;

    drawListArrows(arrowAlpha);
    drawAnimatedNodes();
    drawHeadCard();
    drawWrapArrow(arrowAlpha);
}

static void drawBottomBar() {
    int bottomY = SCREEN_H - (int)BOTTOM_BAR_H;

    DrawRectangle(0, bottomY, SCREEN_W, (int)BOTTOM_BAR_H, PANEL);
    DrawLine(0, bottomY, SCREEN_W, bottomY, BORDER);

    DrawCircle(34, bottomY + 23, 5.0f, app.statusColor);
    drawText(app.status, 48.0f, (float)bottomY + 12.0f, 18.0f, TEXT);

    drawText("Click a field and type  |  Tab: switch field  |  "
             "Enter: insert end / at position  |  Esc: unfocus / quit  |  "
             "Max 10 nodes",
             24.0f, (float)bottomY + 38.0f, 13.0f, MUTED);
}

static void draw() {
    ClearBackground(BG);

    drawDotGrid(TOP_BAR_H + CONTROLS_H, SCREEN_H - BOTTOM_BAR_H);
    drawTopBar();
    drawControls();
    drawListScene();
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
    InitWindow(SCREEN_W, SCREEN_H, "Circular Linked List Visualizer");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    loadFonts();
    refreshList();

    if (app.smokeTest || app.demo) {
        vector<NodeSnapshot> empty;
        insert_eotl(10);
        insert_eotl(20);
        insert_eotl(30);
        insert_eotl(40);
        insert_eotl(50);
        refreshList();
        prepareTransition(empty, app.list);
        setStatus("Demo list: 5 nodes. Inspect data and next pointers, then choose an operation.",
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


