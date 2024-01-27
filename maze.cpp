#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <random>
#include <string>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <deque>
#include <cstdlib>
#include <thread>

#define TEXTTABLE_ENCODE_MULTIBYTE_STRINGS
#define TEXTTABLE_USE_EN_US_UTF8

#include "table.h"
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#include <direct.h>
#include <curses.h>
#else
#include <ncurses.h>
#include <unistd.h>
#endif

using namespace std;
namespace fs = filesystem;

random_device rd;
mt19937 gen(rd());
const int START_COLOR = 32;
const int END_COLOR = 34;
const int PATH_COLOR = 33;
int path_length, path_sum;
int startIndex = 0;

string playername;

#ifdef __unix__ // includes Linux and macOS
#include <termios.h>
#include <unistd.h>

// Function to set the terminal to raw mode
void enableRawMode(struct termios &orig_termios)
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

// Function to restore the terminal to its original state
void disableRawMode(struct termios &orig_termios)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}
#endif

struct Direction
{
    int dx, dy;
    string symbol;

    Direction(int x, int y, const string &s) : dx(x), dy(y), symbol(s) {}
};

struct PlayerRecord
{
    string playerName;
    string mapname;
    int duration;
    bool win;
    string date;

    PlayerRecord(const string &name, const string &map, int dur, bool w, const string &dt)
        : playerName(name), mapname(map), duration(dur), win(w), date(dt) {}

    bool operator<(const PlayerRecord &other) const
    {
        return duration < other.duration;
    }
};
struct GameRecord
{
    string playerName;
    string mapname;
    string timeSpent;
    string gameResult;
    string date;
};

struct LeaderboardRecord
{
    string playerName;
    int totalWins;
    int totalBestRec;
    int totalGames;
    double winRate; // New member for win rate

    LeaderboardRecord(const string &name, int wins, int bestTime, int games, double rate)
        : playerName(name), totalWins(wins), totalBestRec(bestTime), totalGames(games), winRate(rate) {}

    bool operator<(const LeaderboardRecord &other) const
    {
        // Sort by total wins first, then by lesser total best time for ties
        if (winRate != other.winRate)
        {
            return winRate > other.winRate;
        }
        if (totalWins != other.totalWins)
        {
            return totalWins > other.totalWins;
        }
        return totalBestRec < other.totalBestRec;
    }
};

void clear_screen()
{
    cout << "\x1B[2J\x1B[H"; // ANSI escape codes to clear the screen
}
int getRandomInt(int a_l, int a_u, mt19937 &gen)
{
    uniform_int_distribution<> dis_a(a_l, a_u);
    return dis_a(gen);
}

string findfile(const string &path)
{
    auto last_slash = max(path.find_last_of('/'), path.find_last_of('\\'));

    string file_name = path.substr(last_slash + 1);

    file_name = file_name.substr(0, file_name.find_last_of('.'));

    replace(file_name.begin(), file_name.end(), '_', ' ');

    return file_name;
}

bool doesFileExist(const string &filePath)
{
    ifstream file(filePath);
    return file.good();
}

bool doesDirectoryExist(const std::string &dirName)
{
    struct stat info;
    if (stat(dirName.c_str(), &info) != 0)
    {
        return false; // Cannot access the path
    }
    else if (info.st_mode & S_IFDIR) // S_IFDIR means it's a directory
    {
        return true;
    }
    else
    {
        return false; // Path exists but it's not a directory
    }
}

void createDirectory(const std::string &dirPath)
{
    if (!doesDirectoryExist(dirPath))
    {
#ifdef _WIN32
        _mkdir(dirPath.c_str());
#else
        mkdir(dirPath.c_str(), 0777); // Use mkdir with permission flags on Unix
#endif
    }
}

void easyMode();

void hardMode();

void saverec(const string &playerName, const chrono::seconds &game_duration, const string &filename, const string &mapname, bool win);

void updateLeaderboard(const string &playername, const string &leaderboardFilename);

vector<int> ImpossiblePathLengths(int x, int y, int path_length);

bool isImpossiblePathLength(int x, int y, int path_length);

void recursiveBacktrack(vector<vector<int>> &grid, vector<vector<bool>> &visited, int x, int y, int destX, int destY, int a_l, int a_u, int &path_sum, mt19937 &gen, vector<pair<int, int>> &pathCells, int &maxPathCells);

vector<vector<int>> create_grid(int x, int y, int a_l, int a_u, int b_l, int b_u, int path_length, mt19937 &gen, vector<pair<int, int>> &path);

void save_grid(const vector<vector<int>> &grid, const string &filename, int cell_width, int path_length, const string &mode, const string &playerName);

void display_grid(const vector<vector<int>> &grid, const vector<vector<bool>> &path);

void displayClock(int &hours, int &minutes, int &seconds);

void handle_commands(vector<vector<int>> &grid, vector<vector<bool>> &path, int &x, int &y, int &path_length, const string &playername, const string &mapname);

string displayMaps();

void displayPlayerInfo(const string &playerName);

void displayLeaderboard(const string &filename); // leaderboard

bool isValidMove(int x, int y, const vector<vector<int>> &maze, const vector<vector<bool>> &visited);

bool dfs(vector<vector<int>> &maze, vector<vector<bool>> &visited, int x, int y, int sum, int target, int maxPathLength, int pathLength);

void solveMaze(vector<vector<int>> &maze, int maxPathLength);

void display_SolvedMaze(const vector<vector<int>> &grid, const vector<vector<pair<bool, Direction>>> &path);

void do_Choice(string subchoice);

void Select_Choice(int choice);

void displayrec(const string &filename, int &start, int count);

void displayMenu();

void Resetgame(int &x, int &y, vector<vector<int>> &grid, vector<vector<bool>> &path);

int main()
{
    cout << "Enter your name: ";
    getline(cin, playername);

    string playerFilePath = "./Users/" + playername + ".csv";
    bool fileExists = doesFileExist(playerFilePath);
    char answer;
    while (fileExists)
    {
        cout << "Is " << playername << " you? Reply 'Yes' (Y) or 'No' (N)." << endl;
        cin >> answer;
        if (answer == 'n' || answer == 'N')
        {
            cout << "make a newname : ";
            cin >> playername;
        }
        else if (answer == 'y' || answer == 'Y')
        {
            break;
        }
        else
        {
            cout << "Invalid choice. Please try again.\n";
            return 1;
        }
    }
    cout << "Hello, " + playername + "! Welcome to Maze Maverick\n";
    displayMenu();
}

void displayrec(const string &filename, int &start, int count)
{
    ifstream historyFile(filename);
    if (historyFile.is_open())
    {
        vector<PlayerRecord> playerRecords;
        int recordCount = 0;
        string line;

        // Skip records until reaching the starting index
        while (getline(historyFile, line) && recordCount < start)
        {
            recordCount++;
        }
        // Read and display the next 'count' records
        while (getline(historyFile, line) && recordCount < start + count)
        {
            istringstream iss(line);
            string playerName, mapname, date, resultString;
            int duration;
            bool win;
            if (getline(iss, playerName, ',') &&
                getline(iss, mapname, ',') &&
                (iss >> duration) && iss.ignore() &&
                getline(iss, resultString, ',') &&
                getline(iss, date, ','))
            {
                win = (resultString == "win");
                playerRecords.push_back({playerName, mapname, duration, win, date});
                recordCount++;
            }
        }

        TextTable rec;

        rec.setAlignment(0, TextTable::Alignment::LEFT);
        rec.setAlignment(1, TextTable::Alignment::CENTER);
        rec.setAlignment(2, TextTable::Alignment::CENTER);
        rec.setAlignment(3, TextTable::Alignment::CENTER);
        rec.setAlignment(4, TextTable::Alignment::CENTER);

        rec.add("Player");
        rec.add("Map");
        rec.add("Duration/sec");
        rec.add("Result");
        rec.add("Date");
        rec.endOfRow();

        for (const auto &record : playerRecords)
        {
            rec.add(record.playerName);
            rec.add(record.mapname);
            rec.add(to_string(record.duration));
            rec.add(record.win ? "Win" : "Loss");
            rec.add(record.date);
            rec.endOfRow();
        }

        cout << "Records Table :\n"
             << rec << endl;

        historyFile.close();
    }
    else
    {
        cout << "Unable to open " << filename << " for reading player history.\n";
    }
}

void displayMenu()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    string Menu[7] = {"Create a New Map", "Playground", "Solve a Maze", "History", "Player Information", "Leaderboard", "Exit"};
    int pointer = 0;

    while (true)
    {
        clear();
        printw("----Main Menu----\n");

        for (int i = 0; i < 7; ++i)
        {
            if (i == pointer)
            {
                attron(A_STANDOUT);
                printw("%s\n", Menu[i].c_str());
                attroff(A_STANDOUT);
            }
            else
            {
                printw("%s\n", Menu[i].c_str());
            }
        }

        int ch = getch();

        switch (ch)
        {
        case KEY_UP:
            pointer = pointer > 0 ? pointer - 1 : 6;
            break;
        case KEY_DOWN:
            pointer = pointer < 6 ? pointer + 1 : 0;
            break;
        case '\n': // Enter key
        case '\r':
        case KEY_ENTER:
            switch (pointer)
            {
            case 0:
                Select_Choice(0);
                break;
            case 1:
                Select_Choice(1);
                break;
            case 2:
                Select_Choice(2);
                break;
            case 3:
                nocbreak();
                echo();
                keypad(stdscr, FALSE);

                // End ncurses mode
                endwin();

                Select_Choice(3);

                initscr();
                cbreak();
                noecho();
                keypad(stdscr, TRUE);
                break;
            case 4:
                nocbreak();
                echo();
                keypad(stdscr, FALSE);

                // End ncurses mode
                endwin();

                Select_Choice(4);

                initscr();
                cbreak();
                noecho();
                keypad(stdscr, TRUE);
                break;
            case 5:
                nocbreak();
                echo();
                keypad(stdscr, FALSE);

                // End ncurses mode
                endwin();

                Select_Choice(5);

                initscr();
                cbreak();
                noecho();
                keypad(stdscr, TRUE);
                break;
            case 6:
                nocbreak();
                echo();
                keypad(stdscr, FALSE);

                // End ncurses mode
                endwin();
                clear_screen();
                exit(0);
            }
            break;
        }

        refresh();
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
    }
    // Reverse the settings
    nocbreak();
    echo();
    keypad(stdscr, FALSE);

    // End ncurses mode
    endwin();
    return;
}

void Select_Choice(int choice)
{
    bool historyExists;
    const int recordsPerPage = 10;
    string playerFilePath;
    bool leaderExists;
    int viewMore;
    int ch;
    switch (choice)
    {
    case 0:
        break;
    case 1:
        break;
    case 2:
        break;
    case 3:
        historyExists = doesFileExist("./play_history.csv");
        if (!historyExists)
        {
            ofstream playerFile("play_history.csv", ios::app);
            playerFile.close();
        }

        displayrec("play_history.csv", startIndex, recordsPerPage);

        startIndex += recordsPerPage;

        char viewMore;
        cout << "Do you want to see more records? (y/n): ";
        cin >> viewMore;

        if (viewMore == 'y' || viewMore == 'Y')
        {
            // If yes, recursively call Select_Choice with the same choice
            Select_Choice(choice);
        }
        else
        {
            // If no, reset the startIndex for the next time the user chooses to view records
            startIndex = 0;
            displayMenu();
        }
        break;
    case 4:
        clear_screen();
        createDirectory("./Users/");
        playerFilePath = "./Users/" + playername + ".csv";
        displayPlayerInfo(playername);
        ch = getch();
        // cin >> ch;
        if (ch == '\n')
        {
            displayMenu();
        }
        break;
    case 5:
        clear_screen();
        leaderExists = doesFileExist("./Leaderboard.csv");
        if (!leaderExists)
        {
            ofstream playerFile("./Leaderboard.csv", ios::app);
            playerFile.close();
        }
        displayLeaderboard("Leaderboard.csv");
        ch = getch();
        // cin >> ch;
        if (ch == '\n')
        {
            displayMenu();
        }
        break;
    default:
        printw("%s\n", "Invalid choice\n");
        displayMenu();
        break;
    }
    int pointer = 0;

    if (choice == 0)
    {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        string Menu[3] = {"Easy", "Hard", "Back to main menu"};
        while (true)
        {
            clear();
            printw("Select the mode:\n\n");

            for (int i = 0; i < 3; ++i)
            {
                if (i == pointer)
                {
                    attron(A_STANDOUT);
                    printw("%s\n", Menu[i].c_str());
                    attroff(A_STANDOUT);
                }
                else
                {
                    printw("%s\n", Menu[i].c_str());
                }
            }

            int ch = getch();

            switch (ch)
            {
            case KEY_UP:
                pointer = pointer > 0 ? pointer - 1 : 2;
                break;
            case KEY_DOWN:
                pointer = pointer < 2 ? pointer + 1 : 0;
                break;
            case '\n': // Enter key
                switch (pointer)
                {
                case 0:
                    // Reverse the settings
                    nocbreak();
                    echo();
                    keypad(stdscr, FALSE);

                    // End ncurses mode
                    endwin();

                    do_Choice("1.1");

                    initscr();
                    cbreak();
                    noecho();
                    keypad(stdscr, TRUE);
                    break;
                case 1:
                    // Reverse the settings
                    nocbreak();
                    echo();
                    keypad(stdscr, FALSE);

                    // End ncurses mode
                    endwin();

                    do_Choice("1.2");

                    initscr();
                    cbreak();
                    noecho();
                    keypad(stdscr, TRUE);
                    break;
                case 2:
                    displayMenu();
                    break;
                }
                break;
            }

            refresh();
#ifdef _WIN32
            Sleep(100);
#else
            usleep(100000);
#endif
        }
        // Reverse the settings
        nocbreak();
        echo();
        keypad(stdscr, FALSE);

        // End ncurses mode
        endwin();
    }
    else if (choice == 1)
    {
        createDirectory("./Users/");
        createDirectory("./Maps/");
        string playerFilePath = "./Users/" + playername + ".csv";
        bool fileExists = doesFileExist(playerFilePath);
        if (!fileExists)
        {
            ofstream playerFile("./Users/" + playername + ".csv", ios::app);
            playerFile.close();
        }
        bool file = doesFileExist("./Leaderboard.csv");
        if (!file)
        {
            ofstream playerFile("./Leaderboard.csv", ios::app);
            playerFile.close();
        }
        string Menu[3] = {"Choose from Existing Maps", "Import a Custom Map", "Back to main menu"};
        while (true)
        {
            clear();
            printw("Choose one of the followings:\n\n");

            for (int i = 0; i < 3; ++i)
            {
                if (i == pointer)
                {
                    attron(A_STANDOUT);
                    printw("%s\n", Menu[i].c_str());
                    attroff(A_STANDOUT);
                }
                else
                {
                    printw("%s\n", Menu[i].c_str());
                }
            }

            int ch = getch();

            switch (ch)
            {
            case KEY_UP:
                pointer = pointer > 0 ? pointer - 1 : 2;
                break;
            case KEY_DOWN:
                pointer = pointer < 2 ? pointer + 1 : 0;
                break;
            case '\n': // Enter key
                switch (pointer)
                {
                case 0:
                    do_Choice("2.1");
                    break;
                case 1:
                    // Reverse the settings
                    nocbreak();
                    echo();
                    keypad(stdscr, FALSE);

                    // End ncurses mode
                    endwin();

                    do_Choice("2.2");

                    initscr();
                    cbreak();
                    noecho();
                    keypad(stdscr, TRUE);
                    break;
                case 2:
                    displayMenu();
                    break;
                }
                break;
            }

            refresh();
#ifdef _WIN32
            Sleep(100);
#else
            usleep(100000);
#endif
        }
    }
    else if (choice == 2)
    {
        createDirectory("./Users/");
        string Menu[3] = {"Choose from Existing Maps", "Import a Custom Map", "Back to main menu"};
        while (true)
        {
            clear();
            printw("Choose one of the followings:\n\n");

            for (int i = 0; i < 3; ++i)
            {
                if (i == pointer)
                {
                    attron(A_STANDOUT);
                    printw("%s\n", Menu[i].c_str());
                    attroff(A_STANDOUT);
                }
                else
                {
                    printw("%s\n", Menu[i].c_str());
                }
            }

            int ch = getch();

            switch (ch)
            {
            case KEY_UP:
                pointer = pointer > 0 ? pointer - 1 : 2;
                break;
            case KEY_DOWN:
                pointer = pointer < 2 ? pointer + 1 : 0;
                break;
            case '\n': // Enter key
                switch (pointer)
                {
                case 0:
                    do_Choice("3.1");
                    break;
                case 1:
                    // Reverse the settings
                    nocbreak();
                    echo();
                    keypad(stdscr, FALSE);

                    // End ncurses mode
                    endwin();

                    do_Choice("3.2");

                    initscr();
                    cbreak();
                    noecho();
                    keypad(stdscr, TRUE);
                    break;
                case 2:
                    displayMenu();
                    break;
                }
                break;
            }

            refresh();
#ifdef _WIN32
            Sleep(100);
#else
            usleep(100000);
#endif
        }
    }
}

void do_Choice(string subchoice)
{
    if (subchoice == "1.1")
    {
        easyMode();
    }
    else if (subchoice == "1.2")
    {
        hardMode();
    }
    else if (subchoice == "2.1")
    {
        string mapname = displayMaps();
        // Reverse the settings
        nocbreak();
        echo();
        keypad(stdscr, FALSE);

        // End ncurses mode
        endwin();

        ifstream file("./Maps/" + mapname); // all maps are in the "./Maps/" directory
        if (file.is_open())
        {
            string line;
            int read_path_length = -1;
            string read_mode;
            string read_creator;

            // Read mode, path length, and creator from the file
            while (getline(file, line))
            {
                if (line.find("PathLength: ") == 0)
                {
                    read_path_length = stoi(line.substr(12));
                }
                else if (line.find("Mode: ") == 0)
                {
                    read_mode = line.substr(6);
                }
                else if (line.find("Creator: ") == 0)
                {
                    read_creator = line.substr(9);
                }

                if (read_path_length != -1 && !read_mode.empty() && !read_creator.empty())
                {
                    break; // Break out of the loop if all information is found
                }
            }

            if (read_path_length == -1 || read_mode.empty() || read_creator.empty())
            {
                cout << "Error: Mode, PathLength, or Creator information not found in the map file.\n";
                return;
            }

            path_length = read_path_length;
            string mode = read_mode;
            string creator = read_creator;

            cout << "Mode: " << mode << " | Path Length: " << path_length << " | Creator: " << creator << "\n";

            vector<vector<int>> grid;

            while (getline(file, line))
            {

                vector<int> row;
                istringstream iss(line);
                int cell_value;
                while (iss >> cell_value)
                {
                    row.push_back(cell_value);
                }
                grid.push_back(row);
            }

            int x_pos = 0;
            int y_pos = 0;
            vector<vector<bool>> path(grid.size(), vector<bool>(grid[0].size(), false));
            path[0][0] = true;

            display_grid(grid, path);
            handle_commands(grid, path, x_pos, y_pos, path_length, playername, mapname);
        }
        else
        {
            cout << "Error: Unable to open the selected map.\n";
            string subchoice = "2.1";
            do_Choice(subchoice);
        }
    }
    else if (subchoice == "2.2")
    {
        // clear_screen();
        string gridPath, mapname;
        cout << "Enter the path to the grid file: " << endl;
        cin >> gridPath;
        cout << "Enter the map name: " << endl;
        cin >> mapname;

        ifstream file(gridPath);
        if (file.is_open())
        {
            string line;
            int pathLength;
            string read_mode;
            string read_creator;

            // Read mode, path length, and creator from the file
            while (getline(file, line))
            {
                if (line.find("PathLength: ") == 0)
                {
                    pathLength = stoi(line.substr(12));
                }
                else if (line.find("Mode: ") == 0)
                {
                    read_mode = line.substr(6);
                }
                else if (line.find("Creator: ") == 0)
                {
                    read_creator = line.substr(9);
                }

                if (!read_mode.empty() && !read_creator.empty())
                {
                    break; // Break out of the loop if all information is found
                }
            }

            if (read_mode.empty() || read_creator.empty())
            {
                cout << "Error: Mode or Creator information not found in the grid file.\n";
                return;
            }
            string mode = read_mode;
            string creator = read_creator;

            cout << "Mode: " << mode << " | Path Length: " << pathLength << " | Creator: " << creator << "\n";

            vector<vector<int>> grid;

            while (getline(file, line))
            {
                istringstream iss(line);
                vector<int> row;
                int cell_value;
                while (iss >> cell_value)
                {
                    row.push_back(cell_value);
                }
                if (!row.empty())
                {
                    grid.push_back(row);
                }
            }
            if (grid.empty())
            {
                cout << "Error: Grid is empty or improperly formatted.\n";
                return;
            }
            int x_pos = 0;
            int y_pos = 0;
            vector<vector<bool>> path(grid.size(), vector<bool>(grid[0].size(), false));
            path[0][0] = true;
            display_grid(grid, path);
            handle_commands(grid, path, x_pos, y_pos, pathLength, playername, mapname);
        }
        else
        {
            cout << "Error: Unable to open the specified grid file.\n";
        }
    }
    else if (subchoice == "3.1")
    {
        string mapname = displayMaps();
        createDirectory("./Maps/");
        ifstream file("./Maps/" + mapname); // all maps are in the "./Maps/" directory
        if (file.is_open())
        {
            string line;
            int read_path_length = -1;
            string read_mode;
            string read_creator;

            // Read mode, path length, and creator from the file
            while (getline(file, line))
            {
                if (line.find("PathLength: ") == 0)
                {
                    read_path_length = stoi(line.substr(12));
                }
                else if (line.find("Mode: ") == 0)
                {
                    read_mode = line.substr(6);
                }
                else if (line.find("Creator: ") == 0)
                {
                    read_creator = line.substr(9);
                }

                if (read_path_length != -1 && !read_mode.empty() && !read_creator.empty())
                {
                    break; // Break out of the loop if all information is found
                }
            }

            if (read_path_length == -1 || read_mode.empty() || read_creator.empty())
            {
                cout << "Error: Mode, PathLength, or Creator information not found in the map file.\n";
                return;
            }

            path_length = read_path_length;
            string mode = read_mode;
            string creator = read_creator;

            cout << "Mode: " << mode << " | Path Length: " << path_length << " | Creator: " << creator << "\n";

            vector<vector<int>> grid;

            while (getline(file, line))
            {

                vector<int> row;
                istringstream iss(line);
                int cell_value;
                while (iss >> cell_value)
                {
                    row.push_back(cell_value);
                }
                grid.push_back(row);
            }

            int x_pos = 0;
            int y_pos = 0;
            vector<vector<bool>> path(grid.size(), vector<bool>(grid[0].size(), false));
            path[0][0] = true;
            // Reverse the settings
            nocbreak();
            echo();
            keypad(stdscr, FALSE);

            // End ncurses mode
            endwin();

            solveMaze(grid, path_length);
            cout << "Press enter to continue." << endl;
            int ch = getch();
            if (ch == '\n')
            {
                displayMenu();
            }
        }
        else
        {
            cout << "Error: Unable to open the selected map.\n";
        }
    }
    else if (subchoice == "3.2")
    {
        string gridPath, mapname;
        cout << "Enter the path to the grid file: " << endl;
        cin >> gridPath;
        cout << "Enter the map name: " << endl;
        cin >> mapname;

        ifstream file(gridPath);
        if (file.is_open())
        {
            string line;
            int pathLength;
            string read_mode;
            string read_creator;

            // Read mode, path length, and creator from the file
            while (getline(file, line))
            {
                if (line.find("PathLength: ") == 0)
                {
                    pathLength = stoi(line.substr(12));
                }
                else if (line.find("Mode: ") == 0)
                {
                    read_mode = line.substr(6);
                }
                else if (line.find("Creator: ") == 0)
                {
                    read_creator = line.substr(9);
                }

                if (!read_mode.empty() && !read_creator.empty())
                {
                    break; // Break out of the loop if all information is found
                }
            }

            if (read_mode.empty() || read_creator.empty())
            {
                cout << "Error: Mode or Creator information not found in the grid file.\n";
                return;
            }

            string mode = read_mode;
            string creator = read_creator;

            cout << "Mode: " << mode << " | Path Length: " << pathLength << " | Creator: " << creator << "\n";

            vector<vector<int>> grid;

            while (getline(file, line))
            {
                istringstream iss(line);
                vector<int> row;
                int cell_value;
                while (iss >> cell_value)
                {
                    row.push_back(cell_value);
                }
                if (!row.empty())
                {
                    grid.push_back(row);
                }
            }
            if (grid.empty())
            {
                cout << "Error: Grid is empty or improperly formatted.\n";
                return;
            }
            int x_pos = 0;
            int y_pos = 0;
            vector<vector<bool>> path(grid.size(), vector<bool>(grid[0].size(), false));
            path[0][0] = true;
            // Reverse the settings
            nocbreak();
            echo();
            keypad(stdscr, FALSE);

            // End ncurses mode
            endwin();

            solveMaze(grid, pathLength);
            cout << "Press enter to continue." << endl;
            int ch = getch();
            if (ch == '\n')
            {
                displayMenu();
            }
        }
        else
        {
            cout << "Error: Unable to open the specified grid file.\n";
        }
    }
}

void Resetgame(int &x, int &y, vector<vector<int>> &grid, vector<vector<bool>> &path)
{
    x = 0;
    y = 0;
    path = vector<vector<bool>>(grid.size(), vector<bool>(grid[0].size(), false));
    path[0][0] = true;
    string choice;
    string subchoice;
    do
    {
        cout << "Press enter to continue." << endl;
        int ch = getch();
        if (ch == '\n')
        {
            displayMenu();
        }
    } while (true);
}

string displayMaps()
{
    createDirectory("./Maps/");
    string pathaddress = "./Maps";
    vector<string> Menu;
    int pointer = 0;

    for (const auto &entry : fs::directory_iterator(pathaddress))
    {
#ifdef _WIN32
        Menu.push_back(entry.path().filename().string());
#else
        Menu.push_back(entry.path().filename());
#endif
    }

    int menuSize = Menu.size();

    // Check if there are no maps
    if (menuSize == 0)
    {
        clear();
        printw("No maps found. Press any key to return to the menu.\n");
        getch(); // Wait for user input
        Select_Choice(2);
    }

    while (true)
    {
        clear();
        printw("Choose one of the followings:\n\n");

        for (int i = 0; i < menuSize; ++i)
        {
            if (i == pointer)
            {
                attron(A_STANDOUT);
                printw("%s\n", Menu[i].c_str());
                attroff(A_STANDOUT);
            }
            else
            {
                printw("%s\n", Menu[i].c_str());
            }
        }

        int ch = getch();

        switch (ch)
        {
        case KEY_UP:
            pointer = pointer > 0 ? pointer - 1 : menuSize - 1;
            break;
        case KEY_DOWN:
            pointer = pointer < menuSize - 1 ? pointer + 1 : 0;
            break;
        case '\n': // Enter key
            return Menu[pointer];
            break;
        }

        refresh();
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
    }
}

void saverec(const string &playerName, const chrono::seconds &game_duration, const string &filename, const string &mapname, bool win)
{
    // Read the existing records into a deque of strings
    deque<string> records;
    bool fileExists = doesFileExist(filename);
    if (!fileExists)
    {
        // Create the file if it doesn't exist
        ofstream playerFile(filename, ios::app);
        playerFile.close();
    }
    else
    {
        ifstream historyInFile(filename);
        if (historyInFile.is_open())
        {
            string line;
            while (getline(historyInFile, line))
            {
                records.push_back(line);
            }
            historyInFile.close();
        }
        else
        {
            cout << "Unable to open " << filename << " for reading player history.\n";
            return;
        }
    }

    // Prepare the new record
    auto current_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm *timeinfo;
    timeinfo = localtime(&current_time);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%m/%d/%Y", timeinfo);
    string newRecord = playerName + "," + mapname + "," + to_string(game_duration.count()) + "," + (win ? "win" : "loss") + "," + buffer;

    // Add the new record at the second position of the deque
    if (records.empty()) // If there are no records, add the header first
    {
        records.push_back("Player,Map,Duration/Sec,Result,Date");
    }
    records.insert(records.begin() + 1, newRecord);

    // Write the deque back to the file
    ofstream historyOutFile(filename);
    if (historyOutFile.is_open())
    {
        for (const auto &record : records)
        {
            historyOutFile << record << "\n";
        }
        historyOutFile.close();
    }
    else
    {
        cout << "Unable to open " << filename << " for recording player history.\n";
    }
}

void updateLeaderboard(const string &playername, const string &leaderboardFilename)
{
    ifstream playerFile("./Users/" + playername + ".csv");
    if (playerFile.is_open())
    {
        unordered_map<string, int> bestTimes; // Map to store best times for each map

        int totalGames = 0;
        int totalWins = 0;
        int totalTime = 0;

        string line;
        while (getline(playerFile, line))
        {
            istringstream iss(line);
            string mapName, duration, gameResult, date;
            if (getline(iss, mapName, ',') &&
                getline(iss, duration, ',') && getline(iss, gameResult, ',') && getline(iss, date))
            {
                totalGames++;
                int durationSeconds = stoi(duration);
                totalTime += durationSeconds;

                if (gameResult == "win")
                {
                    totalWins++;

                    // Check if the map has a recorded best time
                    if (bestTimes.find(mapName) == bestTimes.end() || durationSeconds < bestTimes[mapName])
                    {
                        bestTimes[mapName] = durationSeconds;
                    }
                }
            }
        }

        playerFile.close();

        // Calculate win rate
        double winRate;
        if (totalGames > 0)
        {
            winRate = static_cast<double>(totalWins) / totalGames;
        }
        // Calculate total best times by summing the best times for each map
        int totalRecords = 0;
        for (const auto &pair : bestTimes)
        {
            totalRecords += pair.second;
        }

        // Read the leaderboard file
        map<string, tuple<int, int, int, double>> leaderboardMap;
        ifstream leaderboardInFile(leaderboardFilename);
        if (leaderboardInFile.is_open())
        {
            while (getline(leaderboardInFile, line))
            {
                istringstream iss(line);
                string name;
                int wins, bestTime, games;
                double winRate;
                if (getline(iss, name, ',') &&
                    (iss >> wins) && iss.ignore() &&
                    (iss >> bestTime) && iss.ignore() &&
                    (iss >> games) && iss.ignore() && (iss >> winRate))
                {
                    leaderboardMap[name] = make_tuple(wins, bestTime, games, winRate);
                }
            }
            leaderboardInFile.close();

            // Update player leaderboard
            leaderboardMap[playername] = make_tuple(totalWins, totalRecords, totalGames, winRate);

            // Write the updated leaderboard back to the file
            ofstream leaderboardOutFile(leaderboardFilename); // Open without ios::app
            if (leaderboardOutFile.is_open())
            {
                for (const auto &entry : leaderboardMap)
                {
                    leaderboardOutFile << entry.first << ","
                                       << get<0>(entry.second) << ","
                                       << get<1>(entry.second) << ","
                                       << get<2>(entry.second) << ","
                                       << fixed << setprecision(4) << get<3>(entry.second) << "\n"; // Add win rate to the file
                }
                leaderboardOutFile.close();
            }
            else
            {
                cout << "Error: Unable to open leaderboard file for writing.\n";
            }
        }
        else
        {
            cout << "Error: Unable to open leaderboard file for reading.\n";
        }
    }
    else
    {
        cout << "Error: Unable to open player or leaderboard file.\n";
    }
}

vector<int> ImpossiblePathLengths(int x, int y, int path_length)
{
    vector<int> impossibleLengths;
    int minPathLength = x + y - 2;

    int maxPathLength = x * y;
    if (maxPathLength % 2 == 0)
    {
        maxPathLength -= 2;
    }
    else
    {
        maxPathLength--;
    }

    for (int i = minPathLength + 1; i <= maxPathLength; i++)
    {
        if (i % 2 != 0)
        {
            impossibleLengths.push_back(i);
        }
    }
    return impossibleLengths;
}

bool isImpossiblePathLength(int x, int y, int path_length)
{
    vector<int> impossibleLengths = ImpossiblePathLengths(x, y, path_length);
    // check if the path length is in the vector of impossible lengths
    for (int i = 0; i < impossibleLengths.size(); i++)
    {
        if (path_length == impossibleLengths[i])
        {
            return false;
        }
    }
    return true;
}

// Function to perform recursive backtracking and generate a path
void recursiveBacktrack(vector<vector<int>> &grid, vector<vector<bool>> &visited, int x, int y, int destX, int destY, int a_l, int a_u, int &path_sum, mt19937 &gen, vector<pair<int, int>> &pathCells, int &maxPathCells)
{
    if (x < 0 || x >= grid.size() || y < 0 || y >= grid[0].size() || visited[x][y])
    {
        return;
    }

    visited[x][y] = true; // Mark the cell as visited
    pathCells.push_back({x, y});
    if (x == destX && y == destY)
    {
        if (maxPathCells == 0) // Check if the path length is correct
        {
            grid[x][y] = path_sum;
        }
        else
        {
            visited[x][y] = false; // Unvisit the cell as path length is incorrect
            pathCells.pop_back();
        }
        return;
    }

    if (maxPathCells <= 0)
    {
        visited[x][y] = false; // Unvisit the cell as maximum path length reached
        pathCells.pop_back();
        return;
    }

    grid[x][y] = getRandomInt(a_l, a_u, gen);
    while (grid[x][y] == 0)
    {
        grid[x][y] = getRandomInt(a_l, a_u, gen);
    }
    path_sum += grid[x][y];
    maxPathCells--;

    vector<pair<int, int>> Orientations = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    shuffle(Orientations.begin(), Orientations.end(), gen);

    for (const auto &Orientation : Orientations)
    {
        int newX = x + Orientation.first;
        int newY = y + Orientation.second;

        recursiveBacktrack(grid, visited, newX, newY, destX, destY, a_l, a_u, path_sum, gen, pathCells, maxPathCells);

        if (grid[destX][destY] == path_sum)
        {
            return; // Path found
        }
    }

    // Backtracking
    visited[x][y] = false;
    pathCells.pop_back();
    path_sum -= grid[x][y];
    // grid[x][y] = 0; // Reset the cell's value on backtracking
    maxPathCells++;
}

vector<vector<int>> create_grid(int x, int y, int a_l, int a_u, int b_l, int b_u, int path_length, mt19937 &gen)
{
    vector<vector<int>> grid(x, vector<int>(y, 0));
    vector<pair<int, int>> pathCells(path_length);
    vector<vector<bool>> visited(grid.size(), vector<bool>(grid[0].size(), false));
    int path_sum = 0;
    // Generate path with random numbers within the range [a_l, a_u] excluding 0
    recursiveBacktrack(grid, visited, 0, 0, x - 1, y - 1, a_l, a_u, path_sum, gen, pathCells, path_length);
    for (int i = 0; i < x; i++)
    {
        for (int j = 0; j < y; j++)
        {
            if (grid[i][j] == 0)
            {
                grid[i][j] = getRandomInt(a_l, a_u, gen);
                while (grid[i][j] == 0)
                {
                    grid[i][j] = getRandomInt(a_l, a_u, gen);
                }
            }
        }
    }
    // change some non-path cells to 0 but not the start and end cells and also not the cells in the path (pathCells)
    if (b_l > x * y - path_length)
    {
        b_l = 0;
        b_u = x * y - path_length - 1;
    }
    int num_cells_to_change = getRandomInt(b_l, b_u, gen);

    int cells_changed = 0;
    while (cells_changed < num_cells_to_change)
    {
        int x_pos = getRandomInt(0, x - 1, gen);
        int y_pos = getRandomInt(0, y - 1, gen);
        if (!(x_pos == 0 && y_pos == 0) && !(x_pos == x - 1 && y_pos == y - 1) && grid[x_pos][y_pos] != 0 && find(pathCells.begin(), pathCells.end(), make_pair(x_pos, y_pos)) == pathCells.end())
        {
            grid[x_pos][y_pos] = 0;
            cells_changed++;
        }
    }
    return grid;
}

void save_grid(const vector<vector<int>> &grid, const string &filename, int cell_width, int path_length, const string &mode, const string &playerName)
{
    string full_filepath = "./Maps/" + filename;

    ofstream file(full_filepath);

    if (file.is_open())
    {
        file << "Mode: " << mode << "\n";
        file << "PathLength: " << path_length << "\n";
        file << "Creator: " << playerName << "\n";

        for (const auto &row : grid)
        {
            for (const auto &val : row)
            {
                file << setw(cell_width) << val;
            }
            file << "\n";
        }

        file.close();
        cout << "Grid saved to " << full_filepath << endl;
    }
    else
    {
        cout << "Unable to open file\n";
    }
}

void display_grid(const vector<vector<int>> &grid, const vector<vector<bool>> &path)
{
    int largest_num = 0;
    for (const auto &row : grid)
    {
        for (const auto &val : row)
        {
            largest_num = max(largest_num, val);
        }
    }
    int cell_width = to_string(largest_num).length() + 2; // Include 1 space on each side for centering

    // Print top border
    cout << "+";
    for (int j = 0; j < grid[0].size(); j++)
    {
        cout << setw(cell_width + 1) << setfill('-') << "+" << setfill(' ');
    }
    cout << "\n";

    for (int i = 0; i < grid.size(); i++)
    {
        // Print left border
        cout << "|";

        for (int j = 0; j < grid[i].size(); j++)
        {
            string cell = to_string(grid[i][j]);
            int padding = cell_width - cell.length(); // Calculate total padding
            int pad_left = padding / 2;               // Padding for the left side
            int pad_right = padding - pad_left;       // Padding for the right side, adjusted for odd padding

            if (i == 0 && j == 0)
            {
                cout << setw(pad_left) << ""
                     << "\033[38;5;76m" << cell << "\x1B[0m" << setw(pad_right) << ""
                     << "|"; // Start position in green
            }
            else if (i == grid.size() - 1 && j == grid[0].size() - 1)
            {
                cout << setw(pad_left) << ""
                     << "\033[38;5;21m" << cell << "\x1B[0m" << setw(pad_right) << ""
                     << "|"; // End position in blue
            }
            else if (path[i][j])
            {
                cout << setw(pad_left) << ""
                     << "\033[38;5;202m" << cell << "\x1B[0m" << setw(pad_right) << ""
                     << "|"; // Cells in the path in orange
            }
            else
            {
                cout << setw(pad_left) << "" << cell << setw(pad_right) << ""
                     << "|";
            }
        }
        cout << "\n";

        // Print bottom border
        cout << "+";
        for (int j = 0; j < grid[0].size(); j++)
        {
            cout << setw(cell_width + 1) << setfill('-') << "+" << setfill(' ');
        }
        cout << "\n";
    }
}

void displayClock(int &hours, int &minutes, int &seconds)
{
    int consoleWidth = 80; // Assuming a console width of 80 columns

    // Calculate the position to align in the right corner
    int position = consoleWidth;

    // Move the cursor to the correct position
    cout << "\033[" << position << "G";

    // Display the timer
    cout << "\033[38;5;162m"
         << "| "
         << "\x1B[0m" << setfill(' ') << setw(2) << "\033[38;5;162m" << hours << " hrs | "
         << "\x1B[0m";
    cout << setfill(' ') << setw(2) << "\033[38;5;162m" << minutes << " min | "
         << "\x1B[0m";
    cout << setfill(' ') << setw(2) << "\033[38;5;162m" << seconds << " sec |"
         << "\x1B[0m" << endl;
}

void handle_commands(vector<vector<int>> &grid, vector<vector<bool>> &path, int &x, int &y, int &path_length, const string &playername, const string &mapname)
{
    chrono::seconds game_duration;
    auto start_time = chrono::steady_clock::now();
    bool win = false;
    auto current_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm *timeinfo;
    timeinfo = localtime(&current_time);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%m/%d/%Y", timeinfo);
    char command;
    int x_pos, y_pos;
    int moves = 0;
    int playsum = 0;
    int ncount = 4;
    cout << "Enter command (W:up, A:left, S:down, D:right): (Q for quit, G for give up)";

    int hours = 0, minutes = 0, seconds = 0;

    displayClock(hours, minutes, seconds);
    while (true)
    {
#ifdef __unix__
        struct termios orig_termios;
        enableRawMode(orig_termios);
        command = getchar();
        disableRawMode(orig_termios);
#else
        cin >> command;
#endif

        // Increment moves only when a valid move is made
        int prev_x = x;
        int prev_y = y;
        playsum += grid[x][y];

        if (ncount == 0)
        {
            win = false;
            cout << "You got to deadend ." << endl;
            cout << "if you want to play again (Y) , if you want Quite (N) : ";
            int choice;
            double subchoice;
            char playAgain;
            cin >> playAgain;
            saverec(playername, game_duration, "play_history.csv", mapname, win);

            if (playAgain == 'Y' || playAgain == 'y')
            {
                Resetgame(x_pos, y_pos, grid, path);
            }
            else
            {
                cout << "gg\n";
                exit(0);
            }
        }
        switch (command)
        {
        case 'W': // Move up
        case 'w':
            x--;
            if (x >= 0 && grid[x][y] != 0 && !path[x][y]) // valid move
            {
                moves++;
                ncount = 4;
            }
            else
            {
                x = prev_x;
                playsum -= grid[x][y];
                ncount--;
            }
            break;
        case 'A': // Move left
        case 'a':
            y--;
            if (y >= 0 && grid[x][y] != 0 && !path[x][y]) // valid move
            {
                moves++;
                ncount = 4;
            }
            else
            {
                y = prev_y;
                playsum -= grid[x][y];
                ncount--;
            }
            break;
        case 'S': // Move down
        case 's':
            x++;
            if (x < grid.size() && grid[x][y] != 0 && !path[x][y]) // valid move
            {
                moves++;
                ncount = 4;
            }
            else
            {
                x = prev_x;
                playsum -= grid[x][y];
                ncount--;
            }
            break;
        case 'D': // Move right
        case 'd':
            y++;
            if (y < grid[0].size() && grid[x][y] != 0 && !path[x][y]) // valid move
            {
                moves++;
                ncount = 4;
            }
            else
            {
                y = prev_y;
                playsum -= grid[x][y];
                ncount--;
            }
            break;
        case 'Q':
        case 'q':
            cout << "gg\n";
            exit(0);
        case 'G':
        case 'g':
            win = false;
            saverec(playername, game_duration, "play_history.csv", mapname, win);
            cout << "You gave up. Your final position is (" << x << ", " << y << ").\n";
            cout << "if you want to play again (Y) , if you want Quit (N) : ";
            int choice;
            double subchoice;
            char playAgain;
            cin >> playAgain;

            if (playAgain == 'Y' || playAgain == 'y')
            {
                Resetgame(x_pos, y_pos, grid, path);
            }
            else
            {
                cout << "gg\n";
                exit(0);
            }
        default:
            cout << "Invalid command. Please try again.\n";
            continue;
        }

        x_pos = x;
        y_pos = y;
        path[x][y] = true;

        auto current_time = chrono::steady_clock::now();
        game_duration = chrono::duration_cast<chrono::seconds>(current_time - start_time);

        if ((moves >= path_length) || (x == grid.size() - 1 && y == grid[0].size() - 1))
        {
            if ((x == grid.size() - 1 && y == grid[0].size() - 1) && ((moves == path_length) && (playsum == grid[x][y])))
            {
                win = true;
                saverec(playername, game_duration, "play_history.csv", mapname, win);
                ofstream playerFile("./Users/" + playername + ".csv", ios::app);
                if (playerFile.is_open())
                {
                    playerFile << mapname << "," << game_duration.count() << "," << (win ? "win" : "loss") << ',' << buffer << "\n";
                    playerFile.close();
                }
                else
                {
                    cout << "Unable to open player's record file.\n";
                }
                updateLeaderboard(playername, "Leaderboard.csv");
                cout << "YOU WON!\n";
                int choice;
                double subchoice;
                // Ask if the player wants to play again
                char playAgain;
                cout << "Do you want to play again? (Y/N): ";
                cin >> playAgain;

                if (playAgain == 'Y' || playAgain == 'y')
                {
                    Resetgame(x_pos, y_pos, grid, path);
                }
                else
                {
                    cout << "gg\n";
                    exit(0);
                }
            }
            else
            {
                win = false;
                saverec(playername, game_duration, "play_history.csv", mapname, win);
                ofstream playerFile("./Users/" + playername + ".csv", ios::app);
                if (playerFile.is_open())
                {
                    playerFile << mapname << "," << game_duration.count() << "," << (win ? "win" : "loss") << ',' << buffer << "\n";
                    playerFile.close();
                }
                else
                {
                    cout << "Unable to open player's record file.\n";
                }
                updateLeaderboard(playername, "Leaderboard.csv");
                if ((x == grid.size() - 1 && y == grid[0].size() - 1) && ((moves == path_length) && (playsum != grid[x][y])))
                {

                    cout << "YOU LOSE! Path you founded is wrong Sum is: " << playsum << endl;
                }
                else
                {
                    cout << "YOU LOSE! Reached maximum allowed moves\n";
                }

                int choice;
                double subchoice;

                // Ask if the player wants to play again
                char playAgain;
                cout << "Do you want to play again? (Y/N): ";
                cin >> playAgain;

                if (playAgain == 'Y' || playAgain == 'y')
                {
                    Resetgame(x_pos, y_pos, grid, path);
                }
                else
                {
                    cout << "gg\n";
                    exit(0);
                }
            }
        }

        hours = game_duration.count() / 3600;
        minutes = (game_duration.count() % 3600) / 60;
        seconds = game_duration.count() % 60;

        clear_screen();
        displayClock(hours, minutes, seconds);
        display_grid(grid, path);
    }
}

void easyMode()
{
    clear_screen();
    int x, y, a_l = -3, a_u = 3, b_l = 2, b_u = 5, path_length;
    int min_plen, max_plen;
    cout << "Enter the number of rows: ";
    cin >> x;
    cout << "Enter the number of columns: ";
    cin >> y;
    path_length = x + y - 2;
    cout << "Path length: " << path_length << endl;
    createDirectory("./Maps/");
    vector<vector<int>> grid = create_grid(x, y, a_l, a_u, b_l, b_u, path_length, gen);
    string mapname;
    cout << "Say your grid name : ";
    cin >> mapname;
    int x_pos = 0;
    int y_pos = 0;
    int largest_num = 0;
    for (const auto &row : grid)
    {
        for (const auto &val : row)
        {
            largest_num = max(largest_num, val);
        }
    }
    int cell_width = to_string(largest_num).length() + 1;

    save_grid(grid, mapname, cell_width, path_length, "Easy", playername);

    vector<vector<bool>> path(x, vector<bool>(y, false));
    path[0][0] = true;
    Resetgame(x, y, grid, path);

    return;
}

void hardMode()
{
    clear_screen();
    int x, y, a_l, a_u, b_l, b_u, path_length;
    int min_plen, max_plen;
    cout << "Enter the number of rows: ";
    cin >> x;
    cout << "Enter the number of columns: ";
    cin >> y;
    min_plen = x + y - 2;
    max_plen = x * y - 2;
    cout << "Enter the path length between [" << min_plen << "," << max_plen << "]: ";
    cin >> path_length;
    while (path_length < min_plen || path_length > max_plen)
    {
        cout << "Invalid path length. Please enter a value between [" << min_plen << "," << max_plen << "]: ";
        cin >> path_length;
    }
    while (true)
    {
        if (!isImpossiblePathLength(x, y, path_length))
        {
            cout << "path_length that you've give me is impossible to make " << endl;
            vector<int> impossibleLengths = ImpossiblePathLengths(x, y, path_length);
            cout << "Impossible path lengths are: ";
            for (int length : impossibleLengths)
            {
                cout << length << " ";
            }
            cout << "\nplease provide me a possible path_length : ";
            cin >> path_length;
        }
        else
        {
            break;
        }
    }
    cout << "Please enter the cells period's lower and upper bounds: ";
    cin >> a_l >> a_u;
    while (true)
    {
        if (a_l > a_u)
        {
            cout << "Invalid bounds. Please enter the lower and upper bounds: ";
            cin >> a_l >> a_u;
        }
        else
        {
            break;
        }
    }
    cout << "Please enter the lower and upper bounds in range [0," << (x * y - path_length) - 1 << "] for blocks: ";
    cin >> b_l >> b_u;
    while (true)
    {
        if (b_l < 0 || b_l > (x * y - path_length) - 1 || b_u < 0 || b_u > (x * y - path_length) - 1)
        {
            cout << "Invalid bounds. Please enter values between [0, " << (x * y - path_length) - 1 << "]: ";
            cin >> b_l >> b_u;
        }
        else
        {
            break;
        }
    }

    createDirectory("./Maps/");
    vector<vector<int>>
        grid = create_grid(x, y, a_l, a_u, b_l, b_u, path_length, gen);
    string mapname;
    cout << "Say your grid name : ";
    cin >> mapname;
    int x_pos = 0;
    int y_pos = 0;
    int largest_num = 0;
    for (const auto &row : grid)
    {
        for (const auto &val : row)
        {
            largest_num = max(largest_num, val);
        }
    }
    int cell_width = to_string(largest_num).length() + 1;

    save_grid(grid, mapname, cell_width, path_length, "Hard", playername);

    vector<vector<bool>> path(x, vector<bool>(y, false));
    path[0][0] = true;
    Resetgame(x, y, grid, path);

    return;
}

void displayPlayerInfo(const string &playerName)
{
    ifstream playerFile("./Users/" + playerName + ".csv");
    if (playerFile.is_open())
    {
        int totalGames = 0;
        int totalWins = 0;
        string lastWinRec = "";
        int totalTime = 0;
        string lastGameDate = ""; // Added to store the date of the last game

        string line;
        while (getline(playerFile, line))
        {
            istringstream iss(line);
            string mapname, duration, gameResult, date; // Added date
            if (getline(iss, mapname, ',') &&
                getline(iss, duration, ',') && getline(iss, gameResult, ',') && getline(iss, date))
            {
                lastGameDate = date; // Set the date for the last win
                totalGames++;
                if (gameResult == "win")
                {
                    totalWins++;
                    // Extracting the date and time from duration (assuming it's in seconds)
                    int durationSeconds = stoi(duration);
                    totalTime += durationSeconds;
                    lastWinRec = to_string(durationSeconds / 3600) + "h " +
                                 to_string((durationSeconds % 3600) / 60) + "m " +
                                 to_string(durationSeconds % 60) + "s";
                }
                else if (gameResult == "loss")
                {
                    int durationSeconds = stoi(duration);
                    totalTime += durationSeconds;
                }
            }
        }

        playerFile.close();

        TextTable t;
        t.setAlignment(0, TextTable::Alignment::LEFT);
        t.setAlignment(1, TextTable::Alignment::CENTER);
        t.setAlignment(2, TextTable::Alignment::CENTER);
        t.setAlignment(3, TextTable::Alignment::CENTER);
        t.setAlignment(4, TextTable::Alignment::CENTER);
        // Add headers
        t.add("Player");
        t.add("Total Games");
        t.add("Total Wins");
        t.add("Last Win Record");
        t.add("Last Game Date");
        t.endOfRow();

        // Add player data
        t.add(playerName);
        t.add(to_string(totalGames));
        t.add(to_string(totalWins));
        t.add(lastWinRec);
        t.add(lastGameDate);
        t.endOfRow();
        cout << t;
        cout << "Press Enter to go back to the main menu.\n";
    }
    else
    {
        cout << "No player information found for " << playerName << ".\n";
    }
}

void displayLeaderboard(const string &filename)
{
    ifstream leaderboardFile(filename);
    if (leaderboardFile.is_open())
    {
        vector<LeaderboardRecord> playerRecords;
        string line;
        while (getline(leaderboardFile, line))
        {
            istringstream iss(line);
            string playerName;
            int totalWins, totalBestRec, totalGames;
            double winRate;

            if (getline(iss, playerName, ',') &&
                (iss >> totalWins) && iss.ignore() &&
                (iss >> totalBestRec) && iss.ignore() &&
                (iss >> totalGames) && iss.ignore() &&
                (iss >> winRate))
            {
                playerRecords.push_back({playerName, totalWins, totalBestRec, totalGames, winRate});
            }
        }

        // Sort the player records.
        sort(playerRecords.begin(), playerRecords.end());

        TextTable rec;
        rec.setAlignment(0, TextTable::Alignment::CENTER);
        rec.setAlignment(1, TextTable::Alignment::LEFT);
        rec.setAlignment(2, TextTable::Alignment::CENTER);
        rec.setAlignment(3, TextTable::Alignment::CENTER);
        rec.setAlignment(4, TextTable::Alignment::CENTER);
        rec.setAlignment(5, TextTable::Alignment::CENTER);

        // Add header row to the table
        rec.add("Rank");
        rec.add("Player");
        rec.add("Total Wins");
        rec.add("Total Records");
        rec.add("Total Games");
        rec.add("Win Rate"); // New column for win rate
        rec.endOfRow();

        // Add player records to the table
        for (size_t i = 0; i < playerRecords.size(); ++i)
        {
            ostringstream streamObj;
            streamObj << fixed << setprecision(2) << playerRecords[i].winRate * 100;
            string winRateStr = streamObj.str();
            rec.add(to_string(i + 1));
            rec.add(playerRecords[i].playerName);
            rec.add(to_string(playerRecords[i].totalWins));
            rec.add(to_string(playerRecords[i].totalBestRec));
            rec.add(to_string(playerRecords[i].totalGames));
            rec.add(winRateStr + "%");
            rec.endOfRow();
        }

        // Display the leaderboard table
        cout << rec << endl;

        leaderboardFile.close();
        cout << "Press Enter to go back to the main menu.\n";
    }
    else
    {
        cout << "Unable to open " << filename << " for reading leaderboard.\n";
    }
}

//  function to check if a move is valid or not
bool isValidMove(int x, int y, const vector<vector<int>> &maze, const vector<vector<pair<bool, Direction>>> &visited)
{
    return x >= 0 && x < maze.size() && y >= 0 && y < maze[0].size() && maze[x][y] != 0 && !visited[x][y].first;
}

bool dfs(vector<vector<int>> &maze, vector<vector<pair<bool, Direction>>> &visited, int x, int y, int sum, int target, int maxPathLength, int &pathLength)
{
    if (x == maze.size() - 1 && y == maze[0].size() - 1)
    {
        if (sum == target && (pathLength == maxPathLength))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    visited[x][y].first = true;

    if (pathLength > maxPathLength)
    {
        visited[x][y].first = false;
        return false;
    }

    vector<Direction> directions = {{0, 1, "→"}, {1, 0, "↓"}, {0, -1, "←"}, {-1, 0, "↑"}};
    for (const auto &dir : directions)
    {
        int newX = x + dir.dx;
        int newY = y + dir.dy;
        if (isValidMove(newX, newY, maze, visited))
        {
            visited[newX][newY].second = dir;
            pathLength++;
            if (dfs(maze, visited, newX, newY, sum + maze[x][y], target, maxPathLength, pathLength))
            {
                return true;
            }
            pathLength--;
        }
    }
    visited[x][y].first = false;
    return false;
}

void solveMaze(vector<vector<int>> &maze, int maxPathLength)
{
    int targetSum = maze.back().back();
    vector<vector<pair<bool, Direction>>> visited(maze.size(), vector<pair<bool, Direction>>(maze[0].size(), {false, {0, 0, ""}}));
    int pathLength = 0;

    if (dfs(maze, visited, 0, 0, 0, targetSum, maxPathLength, pathLength))
    {
        display_SolvedMaze(maze, visited);
    }
    else
    {
        cout << "No path found that meets the conditions." << endl;
    }
}

void display_SolvedMaze(const vector<vector<int>> &grid, const vector<vector<pair<bool, Direction>>> &path)
{
    int largest_num = 0;
    for (const auto &row : grid)
    {
        for (const auto &val : row)
        {
            largest_num = max(largest_num, val);
        }
    }
    int cell_width = to_string(largest_num).length() + 2; // Include 1 space on each side for centering

    // Print top border
    cout << "+";
    for (int j = 0; j < grid[0].size(); j++)
    {
        cout << setw(cell_width + 2) << setfill('-') << "+" << setfill(' ');
    }
    cout << "\n";

    for (int i = 0; i < grid.size(); i++)
    {
        // Print left border
        cout << "|";

        for (int j = 0; j < grid[i].size(); j++)
        {
            string cell = to_string(grid[i][j]);
            int padding = cell_width - cell.length(); // Calculate total padding
            int pad_left = padding / 2;               // Padding for the left side
            int pad_right = padding - pad_left;       // Padding for the right side, adjusted for odd padding

            if (i == 0 && j == 0)
            {
                cout << setw(pad_left) << ""
                     << "\033[38;5;76m" << cell << "\x1B[0m" << setw(pad_right + 1) << ""
                     << "|"; // Start position in green
            }
            else if (i == grid.size() - 1 && j == grid[0].size() - 1)
            {
                cout << setw(pad_left) << ""
                     << "\033[38;5;21m" << cell << "\x1B[0m" << setw(pad_right + 1) << ""
                     << "|"; // End position in blue
            }
            else if (path[i][j].first)
            {
                if (path[i][j].second.symbol == "←")
                {
                    cout << setw(pad_left) << ""
                         << "\033[38;5;198m" << cell << path[i][j].second.symbol << "\x1B[0m" << setw(pad_right) << ""
                         << "|";
                }
                else
                {
                    cout << setw(pad_left) << ""
                         << "\033[38;5;198m" << path[i][j].second.symbol << cell << "\x1B[0m" << setw(pad_right) << ""
                         << "|"; // Path cells in pink
                }
            }
            else
            {
                cout << setw(pad_left + 1) << "" << cell << setw(pad_right) << ""
                     << "|";
            }
        }

        // Print right border and newline
        cout << "\n";

        // Print bottom border
        cout << "+";
        for (int j = 0; j < grid[0].size(); j++)
        {
            cout << setw(cell_width + 2) << setfill('-') << "+" << setfill(' ');
        }
        cout << "\n";
    }
}