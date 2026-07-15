#include <algorithm>
#include <chrono>
#include <conio.h>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>

using namespace std;

struct Point
{
    int r = 0;
    int c = 0;
};

bool operator==(const Point& a, const Point& b)
{
    return a.r == b.r && a.c == b.c;
}

struct MazeGame
{
    int rows = 0;
    int cols = 0;
    int timeLimitSeconds = 60;
    vector<string> grid;
    Point start;
    Point goal;
};

void MoveCursorTo(short x, short y)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {x, y};
    SetConsoleCursorPosition(hOut, pos);
}

void HideCursor()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

void WaitForAnyKey()
{
    cout << "\nPress any key to continue...";
    _getch();
}

MazeGame CreateDefaultMaze()
{
    MazeGame game;
    game.timeLimitSeconds = 60;
    game.grid = {
        "#########",
        "#...#...#",
        "#.#.#.#.#",
        "#.#...#.#",
        "#.......#",
        "###.#.###",
        "#...#...#",
        "#.#...#.#",
        "#########"
    };
    game.rows = static_cast<int>(game.grid.size());
    game.cols = static_cast<int>(game.grid[0].size());
    game.start = {game.rows / 2, game.cols / 2};
    game.goal = {game.rows - 2, game.cols - 2};
    game.grid[game.start.r][game.start.c] = '.';
    game.grid[game.goal.r][game.goal.c] = '.';
    return game;
}

bool InBounds(const MazeGame& game, int r, int c)
{
    return r >= 0 && r < game.rows && c >= 0 && c < game.cols;
}

bool IsWalkable(const MazeGame& game, int r, int c)
{
    return InBounds(game, r, c) && game.grid[r][c] != '#';
}

void DrawMaze(
    const MazeGame& game,
    const Point& player,
    bool showPlayer,
    bool editing = false,
    const Point& cursor = {-1, -1},
    const vector<Point>* highlightPath = nullptr)
{
    vector<string> canvas = game.grid;

    if (highlightPath != nullptr)
    {
        for (size_t i = 0; i < highlightPath->size(); ++i)
        {
            Point p = (*highlightPath)[i];
            if (!(p == game.start) && !(p == game.goal))
            {
                canvas[p.r][p.c] = '*';
            }
        }
    }

    for (int r = 0; r < game.rows; ++r)
    {
        for (int c = 0; c < game.cols; ++c)
        {
            char ch = canvas[r][c];

            if (showPlayer && player.r == r && player.c == c)
            {
                cout << 'M';
            }
            else if (!showPlayer && game.start.r == r && game.start.c == c)
            {
                cout << 'M';
            }
            else if (game.goal.r == r && game.goal.c == c)
            {
                cout << 'G';
            }
            else if (editing && cursor.r == r && cursor.c == c)
            {
                cout << '@';
            }
            else if (ch == '#')
            {
                cout << '#';
            }
            else if (ch == '*')
            {
                cout << '*';
            }
            else
            {
                cout << ' ';
            }
        }
        cout << '\n';
    }
}

bool SaveMazeToFile(const MazeGame& game, const string& fileName)
{
    ofstream out(fileName, ios::binary);
    if (!out)
    {
        return false;
    }

    int version = 1;
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
    out.write(reinterpret_cast<const char*>(&game.rows), sizeof(game.rows));
    out.write(reinterpret_cast<const char*>(&game.cols), sizeof(game.cols));
    out.write(reinterpret_cast<const char*>(&game.timeLimitSeconds), sizeof(game.timeLimitSeconds));
    out.write(reinterpret_cast<const char*>(&game.start.r), sizeof(game.start.r));
    out.write(reinterpret_cast<const char*>(&game.start.c), sizeof(game.start.c));
    out.write(reinterpret_cast<const char*>(&game.goal.r), sizeof(game.goal.r));
    out.write(reinterpret_cast<const char*>(&game.goal.c), sizeof(game.goal.c));

    for (int r = 0; r < game.rows; ++r)
    {
        out.write(game.grid[r].c_str(), game.cols);
    }

    return static_cast<bool>(out);
}

bool LoadMazeFromFile(MazeGame& game, const string& fileName)
{
    ifstream in(fileName, ios::binary);
    if (!in)
    {
        return false;
    }

    MazeGame loaded;
    int version = 0;
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (!in || version != 1)
    {
        return false;
    }

    in.read(reinterpret_cast<char*>(&loaded.rows), sizeof(loaded.rows));
    in.read(reinterpret_cast<char*>(&loaded.cols), sizeof(loaded.cols));
    in.read(reinterpret_cast<char*>(&loaded.timeLimitSeconds), sizeof(loaded.timeLimitSeconds));
    in.read(reinterpret_cast<char*>(&loaded.start.r), sizeof(loaded.start.r));
    in.read(reinterpret_cast<char*>(&loaded.start.c), sizeof(loaded.start.c));
    in.read(reinterpret_cast<char*>(&loaded.goal.r), sizeof(loaded.goal.r));
    in.read(reinterpret_cast<char*>(&loaded.goal.c), sizeof(loaded.goal.c));

    if (!in || loaded.rows <= 0 || loaded.cols <= 0)
    {
        return false;
    }

    loaded.grid.assign(loaded.rows, string(loaded.cols, '.'));
    for (int r = 0; r < loaded.rows; ++r)
    {
        in.read(&loaded.grid[r][0], loaded.cols);
        if (!in)
        {
            return false;
        }
    }

    if (!InBounds(loaded, loaded.start.r, loaded.start.c) ||
        !InBounds(loaded, loaded.goal.r, loaded.goal.c))
    {
        return false;
    }

    loaded.grid[loaded.start.r][loaded.start.c] = '.';
    loaded.grid[loaded.goal.r][loaded.goal.c] = '.';
    game = loaded;
    return true;
}

void TryMovePlayer(const MazeGame& game, Point& player, int dr, int dc)
{
    int nr = player.r + dr;
    int nc = player.c + dc;
    if (IsWalkable(game, nr, nc))
    {
        player.r = nr;
        player.c = nc;
    }
}

void StartPlayMode(MazeGame& game)
{
    Point player = game.start;
    HideCursor();
    system("cls");

    auto beginTime = chrono::steady_clock::now();

    while (true)
    {
        auto now = chrono::steady_clock::now();
        int elapsed = static_cast<int>(chrono::duration_cast<chrono::seconds>(now - beginTime).count());
        int remain = game.timeLimitSeconds - elapsed;

        MoveCursorTo(0, 0);
        cout << "Maze Game\n";
        cout << "Use arrow keys to move. ESC to return.\n";
        cout << "Reach G before time runs out.\n";
        cout << "Time left: " << setw(3) << remain << " seconds\n\n";
        DrawMaze(game, player, true);

        if (player == game.goal)
        {
            cout << "\nSuccess! The mouse reached the granary in time.\n";
            WaitForAnyKey();
            return;
        }

        if (remain <= 0)
        {
            cout << "\nFailed! Time is over.\n";
            WaitForAnyKey();
            return;
        }

        if (_kbhit())
        {
            int ch = _getch();
            if (ch == 27)
            {
                return;
            }

            if (ch == 0 || ch == 224)
            {
                int arrow = _getch();
                if (arrow == 72)
                {
                    TryMovePlayer(game, player, -1, 0);
                }
                else if (arrow == 80)
                {
                    TryMovePlayer(game, player, 1, 0);
                }
                else if (arrow == 75)
                {
                    TryMovePlayer(game, player, 0, -1);
                }
                else if (arrow == 77)
                {
                    TryMovePlayer(game, player, 0, 1);
                }
            }
        }

        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

void ToggleWall(MazeGame& game, const Point& p)
{
    if (p == game.start || p == game.goal)
    {
        return;
    }

    if (game.grid[p.r][p.c] == '#')
    {
        game.grid[p.r][p.c] = '.';
    }
    else
    {
        game.grid[p.r][p.c] = '#';
    }
}

void StartEditMode(MazeGame& game)
{
    Point cursor = game.start;
    HideCursor();
    system("cls");

    while (true)
    {
        MoveCursorTo(0, 0);
        cout << "Edit Mode\n";
        cout << "Arrow keys: move cursor   Space: toggle wall/road\n";
        cout << "S: save maze.dat   L: load maze.dat   Q: return\n";
        cout << "Start and goal positions are protected.\n\n";
        DrawMaze(game, game.start, false, true, cursor);

        if (_kbhit())
        {
            int ch = _getch();

            if (ch == 'q' || ch == 'Q' || ch == 27)
            {
                return;
            }
            else if (ch == 's' || ch == 'S')
            {
                MoveCursorTo(0, static_cast<short>(game.rows + 6));
                if (SaveMazeToFile(game, "maze.dat"))
                {
                    cout << "Saved to maze.dat                  ";
                }
                else
                {
                    cout << "Save failed.                       ";
                }
            }
            else if (ch == 'l' || ch == 'L')
            {
                MoveCursorTo(0, static_cast<short>(game.rows + 6));
                if (LoadMazeFromFile(game, "maze.dat"))
                {
                    cursor = game.start;
                    cout << "Loaded from maze.dat               ";
                }
                else
                {
                    cout << "Load failed.                       ";
                }
            }
            else if (ch == ' ')
            {
                ToggleWall(game, cursor);
            }
            else if (ch == 0 || ch == 224)
            {
                int arrow = _getch();
                if (arrow == 72 && cursor.r > 0)
                {
                    --cursor.r;
                }
                else if (arrow == 80 && cursor.r < game.rows - 1)
                {
                    ++cursor.r;
                }
                else if (arrow == 75 && cursor.c > 0)
                {
                    --cursor.c;
                }
                else if (arrow == 77 && cursor.c < game.cols - 1)
                {
                    ++cursor.c;
                }
            }
        }

        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

void FindAllPathsDFS(
    const MazeGame& game,
    const Point& current,
    vector<vector<bool>>& visited,
    vector<Point>& currentPath,
    vector<vector<Point>>& allPaths)
{
    if (current == game.goal)
    {
        allPaths.push_back(currentPath);
        return;
    }

    static const int dr[4] = {-1, 1, 0, 0};
    static const int dc[4] = {0, 0, -1, 1};

    for (int i = 0; i < 4; ++i)
    {
        int nr = current.r + dr[i];
        int nc = current.c + dc[i];

        if (IsWalkable(game, nr, nc) && !visited[nr][nc])
        {
            visited[nr][nc] = true;
            currentPath.push_back({nr, nc});
            FindAllPathsDFS(game, {nr, nc}, visited, currentPath, allPaths);
            currentPath.pop_back();
            visited[nr][nc] = false;
        }
    }
}

vector<Point> FindShortestPathBFS(const MazeGame& game)
{
    vector<vector<bool>> visited(game.rows, vector<bool>(game.cols, false));
    vector<vector<Point>> parent(game.rows, vector<Point>(game.cols, {-1, -1}));
    queue<Point> q;

    q.push(game.start);
    visited[game.start.r][game.start.c] = true;

    static const int dr[4] = {-1, 1, 0, 0};
    static const int dc[4] = {0, 0, -1, 1};

    while (!q.empty())
    {
        Point cur = q.front();
        q.pop();

        if (cur == game.goal)
        {
            break;
        }

        for (int i = 0; i < 4; ++i)
        {
            int nr = cur.r + dr[i];
            int nc = cur.c + dc[i];
            if (IsWalkable(game, nr, nc) && !visited[nr][nc])
            {
                visited[nr][nc] = true;
                parent[nr][nc] = cur;
                q.push({nr, nc});
            }
        }
    }

    if (!visited[game.goal.r][game.goal.c])
    {
        return {};
    }

    vector<Point> path;
    for (Point p = game.goal; !(p == Point{-1, -1}); p = parent[p.r][p.c])
    {
        path.push_back(p);
        if (p == game.start)
        {
            break;
        }
    }

    reverse(path.begin(), path.end());
    return path;
}

void PrintOnePath(const vector<Point>& path, int index)
{
    cout << "Path " << index << ": ";
    for (size_t i = 0; i < path.size(); ++i)
    {
        cout << "(" << path[i].r << "," << path[i].c << ")";
        if (i + 1 < path.size())
        {
            cout << " -> ";
        }
    }
    cout << '\n';
}

void ShowAllPathsAndShortest(const MazeGame& game)
{
    system("cls");
    cout << "Searching paths...\n\n";

    vector<vector<Point>> allPaths;
    vector<vector<bool>> visited(game.rows, vector<bool>(game.cols, false));
    vector<Point> currentPath;

    visited[game.start.r][game.start.c] = true;
    currentPath.push_back(game.start);
    FindAllPathsDFS(game, game.start, visited, currentPath, allPaths);

    vector<Point> shortestPath = FindShortestPathBFS(game);

    system("cls");
    cout << "Path Analysis\n";
    cout << "All valid paths count: " << allPaths.size() << "\n\n";

    if (allPaths.empty())
    {
        cout << "No valid path exists from M to G.\n";
    }
    else
    {
        for (size_t i = 0; i < allPaths.size(); ++i)
        {
            PrintOnePath(allPaths[i], static_cast<int>(i + 1));
        }
    }

    cout << "\nShortest Path\n";
    if (shortestPath.empty())
    {
        cout << "No shortest path because the goal is unreachable.\n";
    }
    else
    {
        cout << "Shortest path length (steps): " << shortestPath.size() - 1 << "\n";
        PrintOnePath(shortestPath, 1);
        cout << "\nMaze view of shortest path:\n";
        DrawMaze(game, game.start, false, false, {-1, -1}, &shortestPath);
    }

    WaitForAnyKey();
}

void ShowMenu()
{
    system("cls");
    cout << "==============================\n";
    cout << "        MAZE GAME MENU        \n";
    cout << "==============================\n";
    cout << "1. Start game\n";
    cout << "2. Edit maze\n";
    cout << "3. Show all paths and shortest path\n";
    cout << "4. Save maze to file\n";
    cout << "5. Load maze from file\n";
    cout << "0. Exit\n";
    cout << "==============================\n";
    cout << "Choose: ";
}

int main()
{
    MazeGame game = CreateDefaultMaze();
    HideCursor();

    while (true)
    {
        ShowMenu();
        char choice;
        cin >> choice;

        if (choice == '1')
        {
            StartPlayMode(game);
        }
        else if (choice == '2')
        {
            StartEditMode(game);
        }
        else if (choice == '3')
        {
            ShowAllPathsAndShortest(game);
        }
        else if (choice == '4')
        {
            system("cls");
            if (SaveMazeToFile(game, "maze.dat"))
            {
                cout << "Maze saved successfully to maze.dat\n";
            }
            else
            {
                cout << "Save failed.\n";
            }
            WaitForAnyKey();
        }
        else if (choice == '5')
        {
            system("cls");
            if (LoadMazeFromFile(game, "maze.dat"))
            {
                cout << "Maze loaded successfully from maze.dat\n";
            }
            else
            {
                cout << "Load failed. Make sure maze.dat exists.\n";
            }
            WaitForAnyKey();
        }
        else if (choice == '0')
        {
            break;
        }
        else
        {
            system("cls");
            cout << "Invalid choice.\n";
            WaitForAnyKey();
        }
    }

    return 0;
}
