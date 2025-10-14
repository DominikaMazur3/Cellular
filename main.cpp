#include "raylib.h"
#include <climits>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

Color COLORS[4][2] = {{Color{128,128,128,255},Color{192,192,192,255}},
    {Color{223,255,29,255},Color{152,198,0,255}},
    {Color{255,138,183,255},Color{255,217,222,255}},
    {Color{38,0,66,255},{Color{0,115,155,255}}}};

class Selectable {
        public:
            virtual void select(bool s) {}
};

class Button : public Selectable {
    private:
        Rectangle rect;
        const char* text;
        bool isSelected = false;
        void drawButtonText()
        {
            DrawText(text, rect.x + rect.width/2 - MeasureText(text, 20)/2, rect.y, 20, BLACK);
        }
    public:
        Button(int x, int y, int w, int h, const char* buttontext = "Button")
            {rect.x = x; rect.y = y; rect.width = w; rect.height = h; text = buttontext;}
        Rectangle get_rect() {return rect;}
        void set_rect(Rectangle new_rect) {rect = new_rect;}
        bool isInside() {return CheckCollisionPointRec(GetMousePosition(), rect);}
        bool isPressed() {return (isInside() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));}
        void select(bool selected) {isSelected = selected;}
        void draw() {
            if ((isInside() && IsMouseButtonDown(MOUSE_BUTTON_LEFT)))
                {
                    DrawRectangleRounded(rect,0.1,1,GREEN);
                    DrawRectangleRoundedLinesEx(rect,0.1,1,2,BLACK);
                    drawButtonText();
                }
            else if (isSelected)
                {
                    DrawRectangleRoundedLinesEx(rect,0.1,1,3,WHITE);                    
                    DrawRectangleRounded(rect,0.1,1,BLUE);
                    DrawRectangleRoundedLinesEx(rect,0.1,1,2,BLACK);
                    drawButtonText();
                }
            else if (isInside())
                {
                    DrawRectangleRoundedLinesEx(rect,0.1,1,3,WHITE);                    
                    DrawRectangleRounded(rect,0.1,1,LIGHTGRAY);
                    DrawRectangleRoundedLinesEx(rect,0.1,1,2,BLACK);
                    drawButtonText();
                }
            else {
                    DrawRectangleRounded(rect,0.1,1,WHITE);
                    DrawRectangleRoundedLinesEx(rect,0.1,1,2,BLACK);
                    drawButtonText();
                }
        }
    
};

class numberInput : public Selectable {
    private:
        Rectangle rect = {pos.x,pos.y,32,32};
        int value = 0;
        bool focused = false;
        bool new_focus = false;
        Vector2 pos = {0,0};
        const int max_value = 999;
    public:
        numberInput(float x, float y)
            {pos = Vector2{x,y}; set_pos(pos);}
        void select(bool f=true) {focused = f; new_focus = f;}
        bool isFocusNew() {if (new_focus) {new_focus = false; return true;} else return false;}
        bool clickedInside() {return (CheckCollisionPointRec(GetMousePosition(), rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));}
        void set_pos(Vector2 new_pos) {pos = new_pos; rect = {pos.x,pos.y,96,48};}
        void update() {
            if (clickedInside()) {select();}
            if (focused) {
                int c = GetCharPressed();
                if (c <= '9' and c >= '0') {
                    int new_value = value*10 + (c-'0');
                    if (new_value <= max_value) value = new_value;
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {value = value/10;}
            }
            const char* text = std::to_string(value).c_str();
            if (focused) DrawRectangleRec(rect,GREEN);
            else DrawRectangleRec(rect,WHITE);
            DrawRectangleLinesEx(rect,1,BLACK);
            DrawText(text, pos.x+16, pos.y+16, 32, BLACK);
    }
    int get_value() {return value;}
};

const int EDGE_NEIGHBOUR = INT_MAX;

int CELLS_X = 64;
int CELLS_Y = 64;

struct NEIGHBOURS8 {
    int LT;
    int T;
    int RT;
    int R;
    int RB;
    int B;
    int LB;
    int L;
};

struct NEIGHBOURS4 {
    int TOP;
    int LEFT;
    int RIGHT;
    int BOTTOM;
};

struct RULE2 {int a; int b;};

int countNeighbours(NEIGHBOURS4 neighbours, int value) {
    int count = 0;
    if (neighbours.TOP == value) {count++;}
    if (neighbours.LEFT == value) {count++;}
    if (neighbours.RIGHT == value) {count++;}
    if (neighbours.BOTTOM == value) {count++;}
    return count;
}

int countNeighbours(NEIGHBOURS8 neighbours, int value) {
    int count = 0;
    if (neighbours.LT == value) {count++;}
    if (neighbours.T == value) {count++;}
    if (neighbours.RT == value) {count++;}
    if (neighbours.R == value) {count++;}
    if (neighbours.RB == value) {count++;}
    if (neighbours.B == value) {count++;}
    if (neighbours.LB == value) {count++;}
    if (neighbours.L == value) {count++;}
    return count;
}

NEIGHBOURS4 returnNeighbours(std::vector<std::vector<int>> &cell, int x, int y) {
    NEIGHBOURS4 returned;
    if (y <= 0) returned.TOP = EDGE_NEIGHBOUR;
    else returned.TOP = cell[x][y-1];
    if (y >= CELLS_Y-1) returned.BOTTOM = EDGE_NEIGHBOUR;
    else returned.BOTTOM = cell[x][y+1];
    if (x <= 0) returned.LEFT = EDGE_NEIGHBOUR;
    else returned.LEFT = cell[x-1][y];
    if (x >= CELLS_X-1) returned.RIGHT = EDGE_NEIGHBOUR;
    else returned.RIGHT = cell[x+1][y];
    return returned;
}

NEIGHBOURS8 returnNeighbours8(std::vector<std::vector<int>> &cell, int x, int y) {
    NEIGHBOURS8 returned;
    if (y <= 0) returned.T = EDGE_NEIGHBOUR;
    else returned.T = cell[x][y-1];
    if (y >= CELLS_Y-1) returned.B = EDGE_NEIGHBOUR;
    else returned.B = cell[x][y+1];
    if (x <= 0) returned.L = EDGE_NEIGHBOUR;
    else returned.L = cell[x-1][y];
    if (x >= CELLS_X-1) returned.R = EDGE_NEIGHBOUR;
    else returned.R = cell[x+1][y];
    if (y <= 0 || x <= 0) returned.LT = EDGE_NEIGHBOUR;
    else returned.LT = cell[x-1][y-1];
    if (y >= CELLS_Y-1 || x >= CELLS_X-1) returned.RB = EDGE_NEIGHBOUR;
    else returned.RB = cell[x+1][y+1];
    if (x <= 0 || y >= CELLS_Y-1) returned.LB = EDGE_NEIGHBOUR;
    else returned.LB = cell[x-1][y+1];
    if (x >= CELLS_X-1 || y <= 0) returned.RT = EDGE_NEIGHBOUR;
    else returned.RT = cell[x+1][y-1];
    return returned;
}

int returnNeighboursNum(NEIGHBOURS4 n, int value=1) {
    int num = 0;
    if (n.TOP == value) {num += 1;}
    if (n.RIGHT == value) {num += 2;}
    if (n.BOTTOM == value) {num += 4;}
    if (n.LEFT == value) {num += 8;}
    return num;
}

int returnNeighboursNum(NEIGHBOURS8 n, int value=1) {
    int num = 0;
    if (n.T == value) {num += 1;}
    if (n.RT == value) {num += 2;}
    if (n.R == value) {num += 4;}
    if (n.RB == value) {num += 8;}
    if (n.B == value) {num += 16;}
    if (n.LB == value) {num += 32;}
    if (n.L == value) {num += 64;}
    if (n.LT == value) {num += 128;}
    return num;
}

void update(std::vector<std::vector<int>> &cell, std::vector<RULE2> rules, int neighboursMode)
{
    std::vector<std::vector<int>> updated(CELLS_X, std::vector<int>(CELLS_Y,0));
    for (int i = 0; i < CELLS_X; i++) {for (int j = 0; j < CELLS_Y; j++) {updated[i][j] = cell[i][j];}}
    for (int i = 0; i < CELLS_X; i++) {for (int j = 0; j < CELLS_Y; j++) {
        int cellnum = returnNeighboursNum(returnNeighbours8(cell,i,j));
        for(std::vector<RULE2>::iterator it = rules.begin(); it != rules.end(); it++) {
            if (neighboursMode == 8) {
            //{if (cellnum == (*it).a) {updated[i][j] = (*it).b;}} // both options will be available later
            if (countNeighbours(returnNeighbours8(cell,i,j),1) == (*it).a) {updated[i][j] = (*it).b;}}
            else if (neighboursMode == 4) {
                if (countNeighbours(returnNeighbours(cell,i,j),1) == (*it).a) {updated[i][j] = (*it).b;}
            }
        }
    }}
    for (int i = 0; i < CELLS_X; i++) {for (int j = 0; j < CELLS_Y; j++) {cell[i][j] = updated[i][j];}}
}

bool pattern = false;

std::vector<RULE2> readRules(bool &rulesFound, int &neighbours)
{
    std::vector<RULE2> rules;    
    std::ifstream file("rules.txt");
    std::string line;
    int GRID_SIZE = false; // the next line is the grid size
    if (file.is_open()) {
        while (std::getline(file,line)) {
            std::stringstream stream(line);
            RULE2 i;
            if (GRID_SIZE) {
                GRID_SIZE = false;
                int cx, cy;
                if (stream >> cx) {if (stream >> cy) {CELLS_X = cx; CELLS_Y = cy; continue;}}
            }
            if (stream >> i.a)
            {
                if (stream >> i.b)
                    {rules.push_back(i);}
            }
            else {
                if (line.find("mode") != std::string::npos)
                {
                    if (line.find("8") != std::string::npos) {neighbours = 8;}                    
                    else if (line.find("4") != std::string::npos) {neighbours = 4;}
                }
                else if (line.find("grid") != std::string::npos) {GRID_SIZE = true;}
                else if (line.find("pattern") != std::string::npos) {pattern = true;;}
            }
        }
    }
    else {rulesFound = false;}
    file.close();
    return rules;
}

std::vector<std::vector<int>> createGrid() {
    std::vector<std::vector<int>> cell(CELLS_X, std::vector<int>(CELLS_Y,0));
    return cell;
}

std::vector<std::vector<int>> drawpattern(std::vector<std::vector<int>> cell) {
    cell[1][1] = 1;
    cell[2][2] = 1;
    cell[3][1] = 1;
    cell[1][3] = 1;
    cell[3][3] = 1;
    return cell;
}

void drawGrid(std::vector<std::vector<int>> &cell, Color c0, Color c1)
{
    const int toolsHeight = 32;    
    // window scaling, work in progress
    int square = std::min(GetScreenWidth(),GetScreenHeight()-toolsHeight)/std::max(CELLS_X,CELLS_Y);
    int margin = (GetScreenWidth()-square*CELLS_X)/2;
    for (int i = 0; i < CELLS_X; i++) {
            for (int j = 0; j < CELLS_Y; j++) {
                if (cell[i][j] == 0) {
                    DrawRectangle(margin+square*i, square*j+toolsHeight, square, square, c0);
                }
                else if (cell[i][j] == 1) {
                    DrawRectangle(margin+square*i, square*j+toolsHeight, square, square, c1);
                }
            }
        }
}

int main ()
{
    bool gridSize = true;
    bool rulesFound = true;
    int neighboursMode = 8;
    std::vector<RULE2> rules = readRules(rulesFound, neighboursMode);    
    std::vector<std::vector<int>> cell = createGrid();
    const float updateTime = 1.25;
    float speed = 1.0;
    float sinceUpdate = 0;
    bool mode_auto = false;
    bool mode_more_tools = false;

    numberInput widthInput(32,82);
    numberInput heightInput(144,82);
    Button gridButton(256, 82, 128, 48, "OK");
    Button gridButton2(32, 146, 352, 48, "Load from file or default");
    
    std::vector<Button> tools_auto = {
    Button(16,0,128,32, "manual"),
    Button(176,0,128,32, "slower"),
    Button(336,0,128,32, "faster"),
    Button(496,0,128,32, "more"),
    };
    
    std::vector<Button> tools = {
    Button(16,0,128,32, "auto"),
    Button(176,0,128,32, "+1"),
    Button(336,0,128,32, "+10"),
    Button(496,0,128,32, "more"),
    };
    
    std::vector<Button> more_tools = {
    Button(16,0,128,32, "reload rules"),
    Button(176,0,128,32, "soon"),
    Button(336,0,128,32, "colors"),
    Button(496,0,128,32, "back"),
    };

    std::vector<Button> *active_tools = &tools;
    
    int selectedButton = 0;
    int selectedColor = 0;

	InitWindow(640, 672, "Cellular");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(640,64);
    SetTargetFPS(60);
    widthInput.select(true);
    int neighboursIndex = 0;
    bool keyPressed = false;
    Selectable* neighbours[3] = {&widthInput,&heightInput, &gridButton};

	while (!WindowShouldClose())
	{
        if (gridSize) {
            BeginDrawing();
		    ClearBackground(BLUE);
            if (IsKeyPressed(KEY_LEFT)) {
                neighboursIndex = std::max(0,neighboursIndex-1);
                keyPressed = true;
            }
            else if (IsKeyPressed(KEY_RIGHT)) {
                neighboursIndex = std::min(2,neighboursIndex+1);
                keyPressed = true;
            }
            if (keyPressed) {
                keyPressed = false;
                for (int i = 0; i < 3; i++) {if (i != neighboursIndex) neighbours[i]->select(false);}
                neighbours[neighboursIndex]->select(true);
            }
            widthInput.update();
            heightInput.update();
            if (heightInput.isFocusNew()) {widthInput.select(false);}
            if (widthInput.isFocusNew()) {heightInput.select(false);}
            gridButton.draw();
            gridButton2.draw();
            DrawText("Grid size (width, height):", 32, 32, 32, BLACK);
            if ((gridButton.isPressed() || (IsKeyPressed(KEY_ENTER) && neighboursIndex == 2)) && widthInput.get_value() != 0 && heightInput.get_value() != 0) {
                CELLS_X = widthInput.get_value();
                CELLS_Y = heightInput.get_value();
                cell = createGrid();
                gridSize = false;}
            if (gridButton2.isPressed()) {gridSize = false;}
            EndDrawing();
            continue;
        }
        cell = drawpattern(cell);
        int buttonPressed = -1;
        if (mode_more_tools) active_tools = &more_tools;
        else if (mode_auto) active_tools = &tools_auto;
        else active_tools = &tools;
        selectedButton = std::min(selectedButton,int((*active_tools).size())-1);
        float delta = GetFrameTime();
        for (int i = 0; i < 4; i++) {
            if ((*active_tools)[i].isPressed())
            {
                buttonPressed = i;
                break;
            }
        }
        if (mode_auto) {sinceUpdate += delta*speed;}
        while (sinceUpdate >= updateTime) {update(cell, rules, neighboursMode); sinceUpdate -= updateTime;}

        if (IsKeyPressed(KEY_LEFT)) {selectedButton = std::max(selectedButton-1,0);}
        else if (IsKeyPressed(KEY_RIGHT)) {selectedButton = std::min(selectedButton+1,int((*active_tools).size())-1);}
        for (int i = 0; i < 4; i++) {(*active_tools)[i].select(false);}
        (*active_tools)[selectedButton].select(true);

        if (IsKeyPressed(KEY_ENTER)) {buttonPressed = selectedButton;}
        
        if (buttonPressed == 0) {
            if (mode_more_tools) {int last_x = CELLS_X; int last_y = CELLS_Y; rules = readRules(rulesFound, neighboursMode); if (last_x != CELLS_X || last_y != CELLS_Y) {cell = createGrid();}}
            else {mode_auto = !mode_auto;}
        }
        else if (buttonPressed == 1) {
            if (mode_auto) {speed = std::max(0.5f, speed/2);}
            else {update(cell, rules, neighboursMode);}
        }
        else if (buttonPressed == 2) {
            if (mode_more_tools) {selectedColor = (selectedColor+1)%4;}
            else if (mode_auto) {speed = std::min(16.0f, speed*2);}
            else {for (int i = 0; i < 10; i++) {update(cell, rules, neighboursMode);}}
        }
        else if (buttonPressed == 3) {mode_more_tools = !mode_more_tools;}

        BeginDrawing();
		ClearBackground(BLACK);
		drawGrid(cell, COLORS[selectedColor][0],COLORS[selectedColor][1]);
        for (int i = 0; i < 4; i++) {(*active_tools)[i].draw();}
		if (!rulesFound) {DrawText("[!] Rules not found! make sure the file is named rules.txt", 16, 48, 20, BLACK);}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
