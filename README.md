1.整体思路

  这份程序的运行主线在 main.cpp。

  程序先创建一个默认迷宫，再反复显示菜单，让你选择“开始游戏、编辑迷宫、路径分析、保存、读取、退出”。

  游戏模式里用方向键移动老鼠 M，目标是到达粮仓 G。

  编辑模式里可以把墙 # 和路 . 互相切换。

  路径分析用 DFS 求所有路径，用 BFS 求最短路径。

  保存和读取用的是二进制文件序列化，文件名是 maze.dat。




2.运行流程

  程序入口是 main。

  main() 先调用 CreateDefaultMaze() 生成默认地图。

  然后进入 while(true) 死循环，不断显示菜单。

  你输入菜单选项后，程序根据 if / else if 跳到不同功能。

  各功能结束后再回到菜单，直到输入 0 才退出。

3.逐段讲解


1.头文件

main.cpp:L1 #include <algorithm>：引入算法库，这里主要给 reverse() 用，把最短路径反转成“起点到终点”的顺序。

main.cpp:L2 #include <chrono>：时间库，用来计时和倒计时。

main.cpp:L3 #include <conio.h>：控制台输入库，_getch() 和 _kbhit() 都在这里，专门处理按键。

main.cpp:L4 #include <cstdlib>：为了使用 system("cls")。

main.cpp:L5 #include <fstream>：文件输入输出，保存和读取迷宫用。

main.cpp:L6 #include <iomanip>：格式化输出，这里用 setw(3) 让倒计时更整齐。

main.cpp:L7 #include <iostream>：标准输入输出，cin、cout 都在这里。

main.cpp:L8 #include <queue>：队列，BFS 求最短路径要用。

main.cpp:L9 #include <string>：字符串类型，迷宫每一行都用字符串存。

main.cpp:L10 #include <thread>：线程库，这里只用来 sleep_for() 暂停一点时间。

main.cpp:L11 #include <vector>：动态数组，保存地图、路径、访问标记。

main.cpp:L12 #include <windows.h>：Windows 控制台 API，移动光标和隐藏光标用。

main.cpp:L14 using namespace std;：以后写 cout、vector 不用每次都写 std::。



2.坐标结构体 Point


main.cpp:L16-L20 struct Point：定义一个点，保存迷宫中的行号 r 和列号 c。

main.cpp:L18 int r = 0;：默认行号是 0。

main.cpp:L19 int c = 0;：默认列号是 0。

这个结构体的知识点是“自定义数据类型”，把两个相关数据打包在一起，后面表示老鼠位置、粮仓位置、路径结点都更方便。

3. 重载相等运算符

main.cpp:L22-L25 bool operator==(...)：重载 ==，让两个 Point 可以直接比较。

main.cpp:L24 只有当行和列都相同，两个点才算一样。

这样后面就能直接写 if (player == game.goal)，代码可读性更好。

4. 迷宫总体数据结构 MazeGame

main.cpp:L27-L35 struct MazeGame：把迷宫所有核心信息打包到一个结构体里。

main.cpp:L29 rows：迷宫行数。

main.cpp:L30 cols：迷宫列数。

main.cpp:L31 timeLimitSeconds：时间限制，默认 60 秒。

main.cpp:L32 vector<string> grid：二维迷宫，外层是多行，内层每个字符串是一行。

main.cpp:L33 start：老鼠起点。

main.cpp:L34 goal：粮仓终点。

这是典型的“面向数据设计”，相关属性集中管理。

5. 控制台光标函数

MoveCursorTo 的作用是把控制台输出光标移动到指定位置。

main.cpp:L39 GetStdHandle(STD_OUTPUT_HANDLE)：获取控制台输出句柄。

main.cpp:L40 COORD pos = {x, y};：构造目标坐标。

main.cpp:L41 SetConsoleCursorPosition(...)：真正移动光标。

这样程序就能“原地刷新”画面，而不是每一帧都往下打印很多行。

6. 隐藏光标函数

HideCursor 用来隐藏闪烁的控制台光标。

main.cpp:L47 定义 CONSOLE_CURSOR_INFO 结构体。

main.cpp:L48 dwSize = 1：设置光标大小。

main.cpp:L49 bVisible = FALSE：设为不可见。

main.cpp:L50 应用这个配置。

7. 暂停函数

WaitForAnyKey 是一个小工具函数。

main.cpp:L55 先提示用户按任意键继续。

main.cpp:L56 _getch() 等待按键，不需要按回车。

8. 创建默认迷宫

CreateDefaultMaze 是初始化迷宫的函数。

main.cpp:L61 MazeGame game;：创建一个迷宫对象。

main.cpp:L62 设置时间限制 60 秒。

main.cpp:L63-L73 用字符串数组直接写出迷宫形状。

main.cpp:L74 game.grid.size() 求行数。

main.cpp:L75 game.grid[0].size() 求列数。

main.cpp:L76 起点设为中心点。

main.cpp:L77 终点设为右下角内部一点，也就是靠近右下角的粮仓位置。

main.cpp:L78-L79 强制起点和终点所在格子变成路，避免它们刚好落在墙上。

main.cpp:L80 返回这个迷宫对象。

9. 越界判断和可走判断


InBounds 判断一个坐标是否还在地图里面。

main.cpp:L85 行和列都必须在合法范围。

IsWalkable 在 InBounds 基础上再判断是否不是墙。

这两个函数是后面移动、DFS、BFS 的基础。

10. 绘制迷宫


DrawMaze 是显示地图的核心函数。

main.cpp:L94 const MazeGame& game：传入迷宫对象，用引用避免拷贝。

main.cpp:L95 player：当前玩家位置。

main.cpp:L96 showPlayer：是否显示玩家。

main.cpp:L97 editing = false：是否处于编辑模式，默认不是。

main.cpp:L98 cursor = {-1,-1}：编辑光标的默认无效位置。

main.cpp:L99 highlightPath = nullptr：可选参数，如果传入路径，就把路径标出来。

main.cpp:L101 canvas = game.grid：先复制一份迷宫，用于画面显示，不直接改原始数据。

main.cpp:L103-L113 如果传入了路径，就遍历路径，把除起点终点外的路径格子标成 *。

main.cpp:L115-L150 双重循环扫描整个地图，决定每个位置输出什么字符。

main.cpp:L121-L124 如果此处是玩家位置，就输出 M。

main.cpp:L125-L128 如果不显示玩家，就把起点位置仍然显示为 M。

main.cpp:L129-L132 终点显示为 G。

main.cpp:L133-L136 编辑模式下光标位置显示为 @。

main.cpp:L137-L140 墙输出 #。

main.cpp:L141-L144 高亮路径输出 *。

main.cpp:L145-L148 普通道路输出空格，让地图看起来更清楚。

main.cpp:L150 每一行结尾换行。

11. 保存迷宫到文件

SaveMazeToFile 实现存盘。

main.cpp:L156 ofstream out(fileName, ios::binary)：以二进制写方式打开文件。

main.cpp:L157-L160 如果文件打不开，返回 false。

main.cpp:L162 版本号 version = 1，为了以后文件格式升级做准备。

main.cpp:L163-L170 依次写入版本、行数、列数、时间限制、起点坐标、终点坐标。

reinterpret_cast<const char*> 的知识点是：把整型变量的地址临时转换成“字节流地址”，这样 write() 才能按字节写入文件。

main.cpp:L172-L175 再把每一行地图字符写进去。

main.cpp:L177 返回文件流状态，写成功就是 true。

12. 从文件读取迷宫

LoadMazeFromFile 实现读档。

main.cpp:L182 ifstream in(fileName, ios::binary)：以二进制读方式打开文件。

main.cpp:L183-L186 打不开就返回失败。

main.cpp:L188 MazeGame loaded;：先读到临时变量里，防止读坏了直接污染当前迷宫。

main.cpp:L189-L194 先读版本号并校验版本是否等于 1。

main.cpp:L196-L202 依次读回行数、列数、限时、起点、终点。

main.cpp:L204-L207 检查读取是否成功，以及尺寸是否合法。

main.cpp:L209 先创建一个 rows * cols 的空地图。

main.cpp:L210-L217 逐行把地图内容读入。

main.cpp:L219-L223 检查起点和终点是否还在迷宫范围内。

main.cpp:L225-L226 强制保证起点终点是可走的路。

main.cpp:L227 如果一切正常，才把临时迷宫赋值给当前迷宫。

main.cpp:L228 返回读取成功。

13. 单步移动函数


TryMovePlayer 用来尝试移动一次。

main.cpp:L233-L234 根据方向增量 dr、dc 计算新位置。

main.cpp:L235-L239 只有新位置可走时，才真正修改玩家坐标。

这就是“不能穿墙”的关键逻辑。

14. 游戏模式


StartPlayMode 是真正玩游戏的循环。

main.cpp:L244 玩家一开始在起点。

main.cpp:L245 隐藏光标。

main.cpp:L246 清屏。

main.cpp:L248 记录开始时间。

main.cpp:L250 while(true)：进入游戏循环。

main.cpp:L252-L254 每次循环计算已经过了多少秒、还剩多少秒。

main.cpp:L256 光标回到左上角，准备刷新画面。

main.cpp:L257-L260 打印标题、说明和剩余时间。

main.cpp:L260 setw(3) 的作用是让倒计时宽度固定为 3。

main.cpp:L261 调用 DrawMaze() 画出地图和老鼠。

main.cpp:L263-L268 如果玩家已经到终点，提示成功并返回菜单。

main.cpp:L270-L275 如果时间耗尽，提示失败并返回菜单。

main.cpp:L277 _kbhit()：判断是否有按键输入，避免程序卡住。

main.cpp:L279 _getch()：读取一个键值。

main.cpp:L280-L283 如果按的是 ESC，直接退出当前游戏模式。

main.cpp:L285-L304 判断是不是方向键并分别处理。

Windows 控制台里方向键通常会先返回 0 或 224，再返回具体方向码。

main.cpp:L288-L291 72 表示上。

main.cpp:L292-L295 80 表示下。

main.cpp:L296-L299 75 表示左。

main.cpp:L300-L303 77 表示右。

main.cpp:L307 sleep_for(50ms)：稍微休眠一下，避免 CPU 占用过高，也让画面更平稳。

15. 切换墙和路

ToggleWall 是编辑模式里的核心函数。

main.cpp:L313-L316 如果当前光标在起点或终点，就直接返回，不允许改坏关键点。

main.cpp:L318-L325 如果当前位置是墙就改成路，不是墙就改成墙。

这是典型的“状态切换”写法。


16. 编辑模式

StartEditMode 用于修改地图。

main.cpp:L330 编辑光标默认放在起点。

main.cpp:L331-L332 隐藏光标并清屏。

main.cpp:L334 进入编辑循环。

main.cpp:L336-L340 打印编辑说明。

main.cpp:L341 调用 DrawMaze()，这里不显示“正在移动的玩家”，但会显示起点 M 和编辑光标 @。

main.cpp:L343-L345 如果有按键，就读取按键。

main.cpp:L347-L350 Q、q 或 ESC 退出编辑模式。

main.cpp:L351-L362 S 保存文件，并把结果提示输出到地图下方。

main.cpp:L363-L375 L 读取文件，如果成功，光标重置回起点。

main.cpp:L376-L379 空格键切换墙和路。

main.cpp:L380-L399 方向键控制编辑光标上下左右移动。

这里没有调用 IsWalkable()，因为编辑光标可以停在墙上，也可以停在路上。

main.cpp:L383-L397 只做边界检查，不让光标移出地图。

main.cpp:L402 同样休眠 50 毫秒，降低 CPU 占用。

17. DFS 找所有路径


FindAllPathsDFS 是递归深搜函数。

main.cpp:L407-L411 传入当前点、访问标记数组、当前路径、所有结果路径。

main.cpp:L413-L417 递归终止条件：如果当前点已经是终点，就把当前路径保存起来并返回。

main.cpp:L419-L420 方向数组，表示上、下、左、右四个方向。

main.cpp:L422-L435 遍历四个方向，尝试扩展路径。

main.cpp:L427 只有当下一个点能走且没访问过时，才继续。

main.cpp:L429 先标记为已访问。

main.cpp:L430 把这个点加入当前路径。

main.cpp:L431 递归搜索更深层。

main.cpp:L432 回溯时把当前点弹出。

main.cpp:L433 回溯时取消访问标记。

这就是“回溯法”的标准套路：选一个点、走下去、走不通再退回来换路。


FindShortestPathBFS 用广度优先搜索求最短路径。

main.cpp:L440 visited：记录某个格子是否访问过。

main.cpp:L441 parent：记录每个格子的前驱结点，方便最后倒推整条路径。

main.cpp:L442 queue<Point> q：BFS 的核心结构，先进先出。

main.cpp:L444-L445 起点先入队，并标记访问。

main.cpp:L447-L448 同样是四个方向数组。

main.cpp:L450-L471 BFS 主循环。

main.cpp:L452-L453 取出队头元素。

main.cpp:L455-L458 如果已经到终点，就结束搜索。

main.cpp:L460-L469 扫描四个邻居。

main.cpp:L464 只有可走且未访问的结点，才入队。

main.cpp:L466 标记访问。

main.cpp:L467 记录它是从哪个点来的。

main.cpp:L468 把它加入队列。

main.cpp:L473-L476 如果终点根本没访问到，说明无路可走，直接返回空路径。

main.cpp:L478-L486 从终点开始，根据 parent 一步步倒推回起点。

main.cpp:L479 Point{-1, -1} 是无效前驱，表示“没有了”。

main.cpp:L482-L485 到起点后停止。

main.cpp:L488 reverse()：因为刚才是从终点往前推的，所以要翻转成起点到终点顺序。

main.cpp:L489 返回最短路径。

19. 打印一条路径

PrintOnePath 用于把路径坐标打印出来。

main.cpp:L494 打印第几条路径。

main.cpp:L495-L502 把每个坐标按 (r,c) 格式打印，并用 -> 连接。

这部分的作用主要是方便你检查 DFS 和 BFS 的结果。

20. 展示所有路径和最短路径

ShowAllPathsAndShortest 是菜单选项 3 对应的功能。

main.cpp:L508-L509 清屏并提示开始搜索。

main.cpp:L511-L513 准备存放 DFS 结果的容器。

main.cpp:L515 把起点标为已访问

main.cpp:L516 当前路径先压入起点。

main.cpp:L517 调用 DFS 搜索所有路径。

main.cpp:L519 调用 BFS 得到最短路径。

main.cpp:L521-L523 清屏并打印所有路径数目。

main.cpp:L525-L535 如果没有路，就提示无路；否则逐条打印所有路径。

main.cpp:L537-L548 打印最短路径信息。

main.cpp:L544 shortestPath.size() - 1：路径上的结点数减 1，才是“走的步数”。

main.cpp:L547 再用 DrawMaze() 把最短路径画在地图上，路径用 * 表示。

main.cpp:L550 等待用户按键返回。

21. 菜单显示

ShowMenu 只负责把主菜单打印出来。

main.cpp:L555 每次进菜单先清屏。

main.cpp:L556-L565 打印菜单项。

main.cpp:L566 提示用户输入选择。

22. 主函数 main()


main 是整个程序入口。

main.cpp:L571 创建默认迷宫对象。

main.cpp:L572 隐藏控制台光标。

main.cpp:L574 用无限循环反复执行菜单。


main.cpp:L577-L578 读取用户输入的菜单编号。

main.cpp:L580-L583 输入 1 就进入游戏模式。

main.cpp:L584-L587 输入 2 就进入编辑模式。

main.cpp:L588-L591 输入 3 就分析路径。

main.cpp:L592-L604 输入 4 就保存迷宫。

main.cpp:L605-L617 输入 5 就读取迷宫。

main.cpp:L618-L621 输入 0 就 break 跳出循环，程序结束。

main.cpp:L622-L627 其他非法输入就提示错误。

main.cpp:L630 return 0;：程序正常结束。
