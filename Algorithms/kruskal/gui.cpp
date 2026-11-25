// Graph Builder - a raylib GUI for the kruskal graph functions.
//
// All graph data and graph operations live in graph.h. This file only adds
// rendering and input handling. It does not define any new graph operation:
//   - startGraph()  -> create_graph()
//   - connectEdge() -> edge_exists() + add_edge()
//   - stats/colors  -> unuse_vertex()
//
// Build with build_gui.bat (or the VS Code task "Build Graph GUI").

#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

#include "graph.h"

using namespace std;

// ---------------------------------------------------------------------------
// Layout and colors
// ---------------------------------------------------------------------------

static const int SCREEN_W = 1180;
static const int SCREEN_H = 760;
static const float TOP_BAR_H = 88.0f;
static const float BOTTOM_BAR_H = 64.0f;
static const float MARGIN = 24.0f;

static const Color BG           = { 15, 19, 28, 255 };
static const Color PANEL        = { 24, 30, 43, 255 };
static const Color PANEL_ALT    = { 33, 41, 58, 255 };
static const Color BORDER       = { 55, 68, 92, 255 };
static const Color TEXT         = { 231, 237, 245, 255 };
static const Color MUTED        = { 139, 153, 175, 255 };
static const Color ACCENT       = { 59, 130, 246, 255 };
static const Color ACCENT_DARK  = { 37, 99, 235, 255 };
static const Color NODE_COLOR   = { 56, 128, 235, 255 };
static const Color NODE_HOVER   = { 96, 165, 250, 255 };
static const Color NODE_DRAG    = { 251, 146, 60, 255 };
static const Color NODE_UNUSED  = { 71, 85, 105, 255 };
static const Color SELECTED     = { 250, 204, 21, 255 };
static const Color EDGE_COLOR   = { 130, 145, 168, 255 };
static const Color SUCCESS      = { 74, 222, 128, 255 };
static const Color ERROR        = { 248, 113, 113, 255 };
static const Color INFO         = { 96, 165, 250, 255 };

// ---------------------------------------------------------------------------
// Application state
// ---------------------------------------------------------------------------

enum class Screen {
    Setup,
    Builder
};

struct AppState {
    Screen screen = Screen::Setup;

    // The graph comes from graph.h and is never modified outside its functions.
    Graph graph = create_graph(0);
    int vertexCount = 0;
    vector<Vector2> positions;
    vector<int> unusedVertices;
    float nodeRadius = 24.0f;

    // Setup screen
    string countInput = "6";

    // Edge creation
    int selectedSource = -1;
    int pendingDest = -1;
    bool weightDialog = false;
    string weightInput = "1";
    bool weightFresh = true;

    // Dragging
    int dragCandidate = -1;
    int draggingVertex = -1;
    bool dragMoved = false;
    Vector2 dragStart = { 0.0f, 0.0f };
    int hoveredVertex = -1;

    // UI
    string status = "Choose the number of vertices and press Enter.";
    Color statusColor = INFO;
    bool quit = false;

    // Automated startup check; used by: gui.exe --smoke-test
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
                       bool enabled = true, float fontSize = 20.0f) {
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
// Screens / rectangles
// ---------------------------------------------------------------------------

static Rectangle newGraphButton() {
    return Rectangle{ SCREEN_W - 184.0f, 24.0f, 152.0f, 40.0f };
}

static Rectangle setupPanel() {
    return Rectangle{ SCREEN_W / 2.0f - 300.0f, 220.0f, 600.0f, 360.0f };
}

static Rectangle setupInputBox() {
    return Rectangle{ SCREEN_W / 2.0f - 80.0f, 328.0f, 160.0f, 68.0f };
}

static Rectangle setupMinusButton() {
    Rectangle input = setupInputBox();
    return Rectangle{ input.x - 76.0f, input.y + 8.0f, 52.0f, 52.0f };
}

static Rectangle setupPlusButton() {
    Rectangle input = setupInputBox();
    return Rectangle{ input.x + input.width + 24.0f, input.y + 8.0f, 52.0f, 52.0f };
}

static Rectangle setupCreateButton() {
    return Rectangle{ SCREEN_W / 2.0f - 120.0f, 458.0f, 240.0f, 56.0f };
}

static Rectangle dialogPanel() {
    return Rectangle{ SCREEN_W / 2.0f - 230.0f, SCREEN_H / 2.0f - 170.0f, 460.0f, 340.0f };
}

static Rectangle dialogInputBox() {
    Rectangle panel = dialogPanel();
    return Rectangle{ panel.x + 40.0f, panel.y + 130.0f, panel.width - 80.0f, 58.0f };
}

static Rectangle dialogAddButton() {
    Rectangle panel = dialogPanel();
    return Rectangle{ panel.x + 40.0f, panel.y + panel.height - 78.0f, 170.0f, 46.0f };
}

static Rectangle dialogCancelButton() {
    Rectangle panel = dialogPanel();
    return Rectangle{ panel.x + panel.width - 210.0f,
                      panel.y + panel.height - 78.0f, 170.0f, 46.0f };
}

// ---------------------------------------------------------------------------
// Graph setup / layout
// ---------------------------------------------------------------------------

static void rebuildLayout(int count) {
    app.positions.assign(count, Vector2{ 0.0f, 0.0f });

    float centerX = SCREEN_W / 2.0f;
    float centerY = TOP_BAR_H + (SCREEN_H - TOP_BAR_H - BOTTOM_BAR_H) / 2.0f + 4.0f;

    float maxRadiusX = (SCREEN_W - 240.0f) / 2.0f;
    float maxRadiusY = (SCREEN_H - TOP_BAR_H - BOTTOM_BAR_H - 150.0f) / 2.0f;
    float ringRadius = min(maxRadiusX, maxRadiusY);

    if (count <= 1) {
        ringRadius = 0.0f;
    }

    for (int i = 0; i < count; i++) {
        float angle = -PI / 2.0f + (2.0f * PI * i) / (float)count;
        app.positions[i] = Vector2{
            centerX + cosf(angle) * ringRadius,
            centerY + sinf(angle) * ringRadius
        };
    }

    float gap = (count > 1) ? (2.0f * PI * ringRadius / (float)count) : 1000.0f;
    app.nodeRadius = clampf(gap * 0.34f, 14.0f, 24.0f);
}

static void startGraph(int count) {
    count = (int)clampf((float)count, 1.0f, 30.0f);

    // The graph is created only through the shared function.
    app.graph = create_graph(count);
    app.vertexCount = count;
    app.unusedVertices = unuse_vertex(app.graph);

    app.selectedSource = -1;
    app.pendingDest = -1;
    app.weightDialog = false;
    app.dragCandidate = -1;
    app.draggingVertex = -1;
    app.dragMoved = false;
    app.hoveredVertex = -1;

    rebuildLayout(count);
    app.screen = Screen::Builder;

    setStatus("Graph created with " + to_string(count) + " vertices. Click a vertex to start.", INFO);
}

static void openSetup() {
    app.screen = Screen::Setup;
    app.countInput = to_string(app.vertexCount > 0 ? app.vertexCount : 6);

    app.selectedSource = -1;
    app.pendingDest = -1;
    app.weightDialog = false;
    app.dragCandidate = -1;
    app.draggingVertex = -1;
    app.dragMoved = false;
    app.hoveredVertex = -1;

    setStatus("Choose the number of vertices and press Enter.", INFO);
}

static void clickNode(int vertex) {
    if (app.selectedSource == -1) {
        app.selectedSource = vertex;
        setStatus("Vertex " + to_string(vertex) + " selected. Click another vertex to connect.", SELECTED);
    } else if (app.selectedSource == vertex) {
        app.selectedSource = -1;
        setStatus("Selection cleared.", INFO);
    } else {
        app.pendingDest = vertex;
        app.weightDialog = true;
        app.weightInput = "1";
        app.weightFresh = true;
        setStatus("Enter a weight for edge " + to_string(app.selectedSource) +
                  " - " + to_string(vertex) + ".", INFO);
    }
}

static void cancelEdgeDialog() {
    app.weightDialog = false;
    app.pendingDest = -1;
    app.selectedSource = -1;
    setStatus("Edge creation cancelled.", INFO);
}

static void confirmEdge() {
    if (app.selectedSource < 0 || app.pendingDest < 0) {
        app.weightDialog = false;
        return;
    }

    int source = app.selectedSource;
    int dest = app.pendingDest;
    int weight = app.weightInput.empty() ? 1 : atoi(app.weightInput.c_str());

    if (source == dest) {
        setStatus("Self-loops are not allowed.", ERROR);
        return;
    }

    if (weight <= 0) {
        setStatus("Weight must be a positive integer.", ERROR);
        return;
    }

    if (edge_exists(app.graph, source, dest)) {
        setStatus("Edge " + to_string(source) + " - " + to_string(dest) +
                  " already exists.", ERROR);
        app.weightDialog = false;
        app.pendingDest = -1;
        app.selectedSource = -1;
        return;
    }

    // The only place where a new edge is added: the shared function.
    add_edge(app.graph, source, dest, weight);
    app.unusedVertices = unuse_vertex(app.graph);

    setStatus("Edge " + to_string(source) + " - " + to_string(dest) +
              " added with weight " + to_string(weight) + ".", SUCCESS);

    app.weightDialog = false;
    app.pendingDest = -1;
    app.selectedSource = -1;
}

static void clampNodeToCanvas(Vector2& point) {
    point.x = clampf(point.x, MARGIN + app.nodeRadius, SCREEN_W - MARGIN - app.nodeRadius);
    point.y = clampf(point.y, TOP_BAR_H + MARGIN + app.nodeRadius,
                     SCREEN_H - BOTTOM_BAR_H - MARGIN - app.nodeRadius);
}

static int nodeAt(Vector2 point) {
    for (int i = app.vertexCount - 1; i >= 0; i--) {
        if (CheckCollisionPointCircle(point, app.positions[i], app.nodeRadius + 5.0f)) {
            return i;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

static void changeCount(int delta) {
    int value = app.countInput.empty() ? 6 : atoi(app.countInput.c_str());
    value = (int)clampf((float)(value + delta), 1.0f, 30.0f);
    app.countInput = to_string(value);
    setStatus("Choose the number of vertices and press Enter.", INFO);
}

static void updateSetup() {
    int character = 0;
    while ((character = GetCharPressed()) > 0) {
        if (character >= '0' && character <= '9' && app.countInput.size() < 2) {
            if (app.countInput == "0") {
                app.countInput.clear();
            }
            app.countInput.push_back((char)character);
            setStatus("Choose the number of vertices and press Enter.", INFO);
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !app.countInput.empty()) {
        app.countInput.pop_back();
    }

    if (IsKeyPressed(KEY_UP)) {
        changeCount(1);
    }
    if (IsKeyPressed(KEY_DOWN)) {
        changeCount(-1);
    }

    if (buttonClicked(setupMinusButton())) {
        changeCount(-1);
    }
    if (buttonClicked(setupPlusButton())) {
        changeCount(1);
    }

    bool create = IsKeyPressed(KEY_ENTER) || buttonClicked(setupCreateButton());

    if (IsKeyPressed(KEY_ESCAPE)) {
        app.quit = true;
    }

    if (create) {
        int count = app.countInput.empty() ? 0 : atoi(app.countInput.c_str());
        if (count < 1 || count > 30) {
            setStatus("Enter a number between 1 and 30.", ERROR);
        } else {
            startGraph(count);
        }
    }

    bool overButton = buttonHovered(setupMinusButton()) ||
                      buttonHovered(setupPlusButton()) ||
                      buttonHovered(setupCreateButton());
    SetMouseCursor(overButton ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_IBEAM);
}

static void updateWeightDialog() {
    Vector2 mouse = GetMousePosition();

    if (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        cancelEdgeDialog();
        return;
    }

    if (buttonClicked(dialogCancelButton())) {
        cancelEdgeDialog();
        return;
    }

    if (buttonClicked(dialogAddButton()) || IsKeyPressed(KEY_ENTER)) {
        confirmEdge();
        return;
    }

    int character = 0;
    while ((character = GetCharPressed()) > 0) {
        if (character >= '0' && character <= '9' && app.weightInput.size() < 6) {
            if (app.weightFresh) {
                app.weightInput.clear();
                app.weightFresh = false;
            }
            if (app.weightInput == "0") {
                app.weightInput.clear();
            }
            app.weightInput.push_back((char)character);
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (app.weightFresh) {
            app.weightInput.clear();
            app.weightFresh = false;
        } else if (!app.weightInput.empty()) {
            app.weightInput.pop_back();
        }
    }

    if (buttonHovered(dialogAddButton()) || buttonHovered(dialogCancelButton())) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else if (pointInRectangle(mouse, dialogInputBox())) {
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
}

static void updateBuilder() {
    Vector2 mouse = GetMousePosition();

    if (buttonClicked(newGraphButton()) || IsKeyPressed(KEY_N)) {
        openSetup();
        return;
    }

    if (app.weightDialog) {
        app.hoveredVertex = -1;
        updateWeightDialog();
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        if (app.selectedSource != -1) {
            app.selectedSource = -1;
            setStatus("Selection cleared.", INFO);
        }
        return;
    }

    int hovered = nodeAt(mouse);
    app.hoveredVertex = hovered;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (hovered >= 0) {
            app.dragCandidate = hovered;
            app.dragStart = mouse;
            app.dragMoved = false;
        } else if (app.selectedSource != -1) {
            app.selectedSource = -1;
            setStatus("Selection cleared.", INFO);
        }
    }

    if (app.dragCandidate >= 0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float dx = mouse.x - app.dragStart.x;
        float dy = mouse.y - app.dragStart.y;

        if (dx * dx + dy * dy > 16.0f) {
            app.dragMoved = true;
        }

        if (app.dragMoved) {
            Vector2 position = mouse;
            clampNodeToCanvas(position);
            app.positions[app.dragCandidate] = position;
            app.draggingVertex = app.dragCandidate;
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && app.dragCandidate >= 0) {
        int vertex = app.dragCandidate;
        bool moved = app.dragMoved;

        app.dragCandidate = -1;
        app.draggingVertex = -1;
        app.dragMoved = false;

        if (moved) {
            setStatus("Vertex " + to_string(vertex) + " moved.", INFO);
        } else {
            clickNode(vertex);
        }
    }

    if (buttonHovered(newGraphButton()) || hovered >= 0) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
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

static void drawSetup() {
    ClearBackground(BG);

    DrawCircleLines(SCREEN_W / 2, SCREEN_H / 2, 340, Color{ 59, 130, 246, 24 });
    DrawCircleLines(SCREEN_W / 2, SCREEN_H / 2, 430, Color{ 59, 130, 246, 14 });
    drawDotGrid(0.0f, (float)SCREEN_H);

    drawTextCentered("Graph Builder", SCREEN_W / 2.0f, 84.0f, 54.0f, TEXT, true);
    drawTextCentered("Build a weighted graph, then visualize Kruskal's algorithm",
                     SCREEN_W / 2.0f, 152.0f, 20.0f, MUTED);

    Rectangle panel = setupPanel();
    DrawRectangleRounded(panel, 0.06f, 12, PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.06f, 12, 1.0f, BORDER);

    drawTextCentered("Number of vertices", SCREEN_W / 2.0f, panel.y + 34.0f, 22.0f, TEXT, true);

    Rectangle input = setupInputBox();
    DrawRectangleRounded(input, 0.18f, 10, PANEL_ALT);
    DrawRectangleRoundedLinesEx(input, 0.18f, 10, 2.0f, ACCENT);

    drawTextCentered(app.countInput, input.x + input.width / 2.0f,
                     input.y + 13.0f, 38.0f, TEXT, true);

    if (fmod(GetTime(), 1.0) < 0.5) {
        float textWidth = measureText(app.countInput, 38.0f, true);
        DrawRectangle((int)(input.x + input.width / 2.0f + textWidth / 2.0f + 4.0f),
                      (int)(input.y + 18.0f), 2, 30, ACCENT);
    }

    drawButton(setupMinusButton(), "-", false, true, 30.0f);
    drawButton(setupPlusButton(), "+", false, true, 30.0f);

    drawTextCentered("1 - 30 vertices", SCREEN_W / 2.0f, input.y + input.height + 14.0f,
                     16.0f, MUTED);

    drawButton(setupCreateButton(), "Create Graph", true, true, 22.0f);

    drawTextCentered("Press Enter to create  |  Esc to quit",
                     SCREEN_W / 2.0f, panel.y + panel.height - 32.0f, 15.0f, MUTED);

    drawTextCentered(app.status, SCREEN_W / 2.0f, panel.y + panel.height + 22.0f,
                     16.0f, app.statusColor);

    drawTextCentered("raylib 6.2  |  graph operations from graph.h",
                     SCREEN_W / 2.0f, SCREEN_H - 38.0f, 14.0f, Color{ 100, 116, 139, 255 });

    const string complexity = "Kruskal: O(E log E)";
    float complexityWidth = measureText(complexity, 14.0f, true);
    drawText(complexity, SCREEN_W - complexityWidth - 24.0f,
             SCREEN_H - 38.0f, 14.0f, ACCENT, true);
}

static void drawEdgeWeight(const Edge& edge, Vector2 from, Vector2 to, bool active) {
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float length = sqrtf(dx * dx + dy * dy);
    if (length < 0.001f) {
        length = 1.0f;
    }

    // Offset the weight label perpendicular to the line so crossing edges
    // do not put their labels on top of each other.
    Vector2 normal = { -dy / length, dx / length };
    Vector2 middle = {
        (from.x + to.x) / 2.0f + normal.x * 17.0f,
        (from.y + to.y) / 2.0f + normal.y * 17.0f
    };

    string text = to_string(edge.weight);
    float fontSize = 17.0f;
    float textWidth = measureText(text, fontSize, true);

    Rectangle pill = {
        middle.x - textWidth / 2.0f - 8.0f,
        middle.y - 13.0f,
        textWidth + 16.0f,
        26.0f
    };

    DrawRectangleRounded(pill, 0.5f, 8, PANEL);
    DrawRectangleRoundedLinesEx(pill, 0.5f, 8, 1.0f, active ? SELECTED : BORDER);
    drawTextCentered(text, middle.x, middle.y - fontSize / 2.0f - 1.0f,
                     fontSize, active ? SELECTED : TEXT, true);
}

static void drawBuilder() {
    ClearBackground(BG);

    drawDotGrid(TOP_BAR_H, SCREEN_H - BOTTOM_BAR_H);

    // Top bar ---------------------------------------------------------------
    DrawRectangle(0, 0, SCREEN_W, (int)TOP_BAR_H, PANEL);
    DrawLine(0, (int)TOP_BAR_H, SCREEN_W, (int)TOP_BAR_H, BORDER);

    drawText("Graph Builder", 24.0f, 16.0f, 30.0f, TEXT, true);
    drawText("Kruskal visualizer", 24.0f, 52.0f, 16.0f, MUTED);

    string stats = "Vertices: " + to_string(app.vertexCount) +
                   "     Edges: " + to_string(app.graph.edges.size()) +
                   "     Isolated: " + to_string(app.unusedVertices.size());
    drawText(stats, 500.0f, 34.0f, 18.0f, MUTED);

    drawButton(newGraphButton(), "New Graph", false, true, 18.0f);

    // Edges -----------------------------------------------------------------
    for (const Edge& edge : app.graph.edges) {
        Vector2 from = app.positions[edge.src];
        Vector2 to = app.positions[edge.dest];

        bool active = app.selectedSource != -1 &&
                      (edge.src == app.selectedSource || edge.dest == app.selectedSource);

        DrawLineEx(from, to, active ? 7.0f : 5.0f, PANEL);
        DrawLineEx(from, to, active ? 4.0f : 2.5f, active ? SELECTED : EDGE_COLOR);

        drawEdgeWeight(edge, from, to, active);
    }

    // Pending connection ----------------------------------------------------
    if (app.selectedSource != -1 && !app.weightDialog) {
        Vector2 from = app.positions[app.selectedSource];
        Vector2 mouse = GetMousePosition();
        DrawLineEx(from, mouse, 2.0f, Fade(SELECTED, 0.55f));
    }

    // Vertices --------------------------------------------------------------
    for (int i = 0; i < app.vertexCount; i++) {
        Vector2 position = app.positions[i];
        bool unused = find(app.unusedVertices.begin(), app.unusedVertices.end(), i) !=
                      app.unusedVertices.end();

        Color fill = unused ? NODE_UNUSED : NODE_COLOR;
        if (i == app.hoveredVertex) {
            fill = NODE_HOVER;
        }
        if (i == app.draggingVertex) {
            fill = NODE_DRAG;
        }
        if (i == app.selectedSource) {
            fill = SELECTED;
        }

        if (i == app.selectedSource) {
            DrawCircleLinesV(position, app.nodeRadius + 8.0f, Fade(SELECTED, 0.45f));
        }

        DrawCircleV(position, app.nodeRadius + 4.0f, BG);
        DrawCircleV(position, app.nodeRadius, fill);

        Color border = (i == app.selectedSource) ? WHITE : Fade(WHITE, 0.18f);
        DrawCircleLinesV(position, app.nodeRadius, border);

        string label = to_string(i);
        float labelSize = app.nodeRadius * 0.95f;
        float labelWidth = measureText(label, labelSize, true);
        float maxLabelWidth = app.nodeRadius * 1.45f;

        if (labelWidth > maxLabelWidth) {
            labelSize *= maxLabelWidth / labelWidth;
        }

        Color labelColor = (i == app.selectedSource) ? Color{ 20, 24, 33, 255 } : TEXT;
        drawTextCentered(label, position.x, position.y - labelSize / 2.0f - 1.0f,
                         labelSize, labelColor, true);
    }

    // Bottom bar ------------------------------------------------------------
    int bottomY = SCREEN_H - (int)BOTTOM_BAR_H;
    DrawRectangle(0, bottomY, SCREEN_W, (int)BOTTOM_BAR_H, PANEL);
    DrawLine(0, bottomY, SCREEN_W, bottomY, BORDER);

    DrawCircle(34, bottomY + 23, 5.0f, app.statusColor);
    drawText(app.status, 48.0f, (float)bottomY + 13.0f, 19.0f, TEXT);

    drawText("Drag to move  |  Click two vertices to add an edge  |  "
             "Right-click / Esc to cancel  |  N: new graph",
             24.0f, (float)bottomY + 39.0f, 14.0f, MUTED);

    drawText("Gray vertex = no edges", 24.0f, (float)bottomY - 26.0f, 14.0f, MUTED);

    const string complexity = "Kruskal: O(E log E)";
    float complexityWidth = measureText(complexity, 15.0f, true);
    drawText(complexity, SCREEN_W - complexityWidth - 24.0f,
             (float)bottomY + 22.0f, 15.0f, ACCENT, true);

    // Weight dialog ---------------------------------------------------------
    if (app.weightDialog) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.62f));

        Rectangle panel = dialogPanel();
        DrawRectangleRounded(panel, 0.05f, 12, PANEL);
        DrawRectangleRoundedLinesEx(panel, 0.05f, 12, 1.0f, BORDER);

        drawTextCentered("Add Edge", SCREEN_W / 2.0f, panel.y + 26.0f, 30.0f, TEXT, true);
        drawTextCentered("Vertex " + to_string(app.selectedSource) +
                         "  <->  Vertex " + to_string(app.pendingDest),
                         SCREEN_W / 2.0f, panel.y + 70.0f, 18.0f, MUTED);

        drawText("Weight", panel.x + 40.0f, panel.y + 106.0f, 16.0f, MUTED, true);

        Rectangle input = dialogInputBox();
        DrawRectangleRounded(input, 0.18f, 10, PANEL_ALT);
        DrawRectangleRoundedLinesEx(input, 0.18f, 10, 2.0f, ACCENT);

        string shown = app.weightInput.empty() ? "1" : app.weightInput;
        Color shownColor = app.weightInput.empty() ? MUTED : TEXT;
        drawText(shown, input.x + 20.0f, input.y + 15.0f, 28.0f, shownColor, true);

        if (fmod(GetTime(), 1.0) < 0.5) {
            float shownWidth = measureText(shown, 28.0f, true);
            DrawRectangle((int)(input.x + 20.0f + shownWidth + 3.0f),
                          (int)(input.y + 18.0f), 2, 26, ACCENT);
        }

        drawText("Enter a positive integer", input.x, input.y + 66.0f, 14.0f, MUTED);

        drawButton(dialogAddButton(), "Add Edge", true, true, 20.0f);
        drawButton(dialogCancelButton(), "Cancel", false, true, 20.0f);

        drawTextCentered("Enter: add  |  Esc: cancel",
                         SCREEN_W / 2.0f, panel.y + panel.height - 22.0f, 14.0f, MUTED);
    }
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
    InitWindow(SCREEN_W, SCREEN_H, "Graph Builder - Kruskal");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    loadFonts();

    if (app.smokeTest || app.demo) {
        startGraph(6);

        // Exercise the shared functions in a real run, not only at compile time.
        add_edge(app.graph, 0, 1, 4);
        add_edge(app.graph, 1, 2, 8);
        add_edge(app.graph, 2, 3, 7);
        add_edge(app.graph, 3, 4, 9);
        add_edge(app.graph, 4, 5, 10);
        add_edge(app.graph, 5, 0, 6);
        app.unusedVertices = unuse_vertex(app.graph);

        if (app.smokeTest) {
            app.selectedSource = 0;
            setStatus("Smoke test mode: graph drawn with 6 edges.", INFO);
        } else {
            setStatus("Demo graph: 6 vertices, 6 edges. Drag vertices or add more edges.", INFO);
        }
    }

    while (!WindowShouldClose() && !app.quit) {
        if (app.screen == Screen::Setup) {
            updateSetup();
            BeginDrawing();
            drawSetup();
            EndDrawing();
        } else {
            updateBuilder();
            BeginDrawing();
            drawBuilder();
            EndDrawing();
        }

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
