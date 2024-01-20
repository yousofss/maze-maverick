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

#define TEXTTABLE_ENCODE_MULTIBYTE_STRINGS
#define TEXTTABLE_USE_EN_US_UTF8

#include "table.h"

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

    LeaderboardRecord(const string &name, int wins, int bestTime, int games)
        : playerName(name), totalWins(wins), totalBestRec(bestTime), totalGames(games) {}

    bool operator<(const LeaderboardRecord &other) const
    {
        // Sort by total wins first, then by lesser total best time for ties
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

void easyMode();

void hardMode();

void saverec(const string &playerName, const chrono::seconds &game_duration, const string &filename, const string &mapname, bool win);

void updateLeaderboard(const string &playername, const string &leaderboardFilename);

vector<int> ImpossiblePathLengths(int x, int y, int path_length);

bool isImpossiblePathLength(int x, int y, int path_length);

void recursiveBacktrack(vector<vector<int>> &grid, int x, int y, int destX, int destY, int a_l, int a_u, int b_l, int b_u, int &path_sum, mt19937 &gen);

vector<vector<int>> create_grid(int x, int y, int a_l, int a_u, int b_l, int b_u, int path_length, mt19937 &gen);

void save_grid(const vector<vector<int>> &grid, const string &filename, int cell_width, int path_length, const string &mode, const string &playerName);

void display_grid(const vector<vector<int>> &grid, const vector<vector<bool>> &path);

void handle_commands(vector<vector<int>> &grid, vector<vector<bool>> &path, int &x, int &y, int &path_length, const string &playername, const string &mapname);

void displayMaps();

void displayPlayerInfo(const string &playerName);

void displayLeaderboard(const string &filename); // leaderboard

bool isValidMove(int x, int y, const vector<vector<int>> &maze, const vector<vector<bool>> &visited);

bool dfs(vector<vector<int>> &maze, vector<vector<bool>> &visited, int x, int y, int sum, int target, int maxPathLength, int pathLength);

void solveMaze(vector<vector<int>> &maze, int maxPathLength);

void display_SolvedMaze(const vector<vector<int>> &grid, const vector<vector<pair<bool, Direction>>> &path);

void displayrec(const string &filename, int &start, int count) // it's better but it should be fix for remain lees than 10 rows it shouldn't ask for showing another page
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
                (iss >> duration) && iss.ignore() &&
                getline(iss, date, ',') &&
                getline(iss, mapname, ',') &&
                getline(iss, resultString, ','))
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
    cout << "----- Menu -----" << endl;
    cout << "1. Create a New Map" << endl;
    cout << "2. Playground" << endl;
    cout << "3. Solve a Maze" << endl;
    cout << "4. History" << endl;
    cout << "5. Player Information" << endl;
    cout << "6. Leaderboard" << endl;
    cout << "7. Exit" << endl;
    return;
}

void Select_Choice(string choice)
{
    string playerFilePath = "./Users/" + playername + ".csv";
    bool fileExists = doesFileExist(playerFilePath);
    if (choice == "1")
    {
        cout << "1.1 Easy" << endl;
        cout << "1.2 Hard" << endl;
        cout << "Enter your choice: (0 to return to the main menu) ";
    }
    else if (choice == "2")
    {
        if (!fileExists)
        {
            ofstream playerFile("./Users/" + playername + ".csv", ios::app);
            playerFile.close();
        }
        cout << "2.1 Choose from Existing Maps" << endl;
        cout << "2.2 Import a Custom Map" << endl;
        cout << "Enter your choice: (0 to return to the main menu) ";
    }
    else if (choice == "3")
    {
        cout << "3.1 Choose from Existing Maps" << endl;
        cout << "3.2 Import a Custom Map" << endl;
        cout << "Enter your choice: (0 to return to the main menu) ";
    }
    else if (choice == "4")
    {
        const int recordsPerPage = 10;

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
            cin >> choice;
            Select_Choice(choice);
        }
    }
    else if (choice == "5")
    {
        displayPlayerInfo(playername);
        displayMenu();
        cin >> choice;
        Select_Choice(choice);
        return;
    }
    else if (choice == "6")
    {
        displayLeaderboard("Leaderboard.csv");
        displayMenu();
        cin >> choice;
        Select_Choice(choice);
        return;
    }
    else if (choice == "7")
    {
        exit(0);
    }
    else
    {
        cout << "Invalid choice" << endl;
        displayMenu();
        cin >> choice;
        Select_Choice(choice);
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
        displayMaps();
        cout << "Please provide the name of the map you'd like to play with: ";
        string mapname;
        cin >> mapname;

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
        displayMaps();
        cout << "Enter the name of the map you want solved." << endl;
        string mapname;
        cin >> mapname;

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
            solveMaze(grid, path_length);
            displayMenu();
            string choice;
            cout << "Enter your choice: " << endl;
            cin >> choice;
            Select_Choice(choice);
            string subchoice;
            cin >> subchoice;
            do_Choice(subchoice);
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
            solveMaze(grid, pathLength);
            displayMenu();
            string choice;
            cout << "Enter your choice: " << endl;
            cin >> choice;
            Select_Choice(choice);
            string subchoice;
            cin >> subchoice;
            do_Choice(subchoice);
        }
        else
        {
            cout << "Error: Unable to open the specified grid file.\n";
        }
    }
    else if (subchoice == "0")
    {
        displayMenu();
        string choice;
        cout << "Enter your choice: " << endl;
        cin >> choice;
        Select_Choice(choice);
        string subchoice;
        cin >> subchoice;
        do_Choice(subchoice);
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
        displayMenu();
        cout << "Enter your choice: ";
        cin >> choice;

        if (choice == "1" || choice == "2" || choice == "3")
        {
            Select_Choice(choice);
            cin >> subchoice;
            do_Choice(subchoice);
        }
        else if (choice == "4" || choice == "5" || choice == "6")
        {
            Select_Choice(choice);
        }
        else if (choice == "7")
        {
            cout << "Exiting the program. Goodbye!\n";
            return;
        }
        else
        {
            cout << "Invalid choice. Please try again.\n";
        }
    } while (true);
}

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
    string choice;
    cout << "Enter your choice: " << endl;
    cin >> choice;
    Select_Choice(choice);
    string subchoice;
    cin >> subchoice;
    do_Choice(subchoice);

    return 0;
}

void displayMaps()
{
    string pathaddress = "./Maps";
    int index = 1;

    for (const auto &entry : fs::directory_iterator(pathaddress))
    {
        cout << index << ". " << entry.path().filename() << endl;
        index++;
    }
}

void saverec(const string &playerName, const chrono::seconds &game_duration, const string &filename, const string &mapname, bool win)
{
    // Read the existing records into a deque of strings
    deque<string> records;
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

    // Prepare the new record
    auto current_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm *timeinfo;
    timeinfo = localtime(&current_time);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%m/%d/%Y", timeinfo);
    string newRecord = playerName + "," + to_string(game_duration.count()) + "," + buffer + "," + mapname + "," + (win ? "win" : "loss");

    // Add the new record at the second position of the deque
    if (records.size() >= 1) // Check if there is at least one record
    {
        records.insert(records.begin() + 1, newRecord);
    }
    else // If there are no records, just add the new record
    {
        records.push_back(newRecord);
    }

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
    ofstream leaderboardFile(leaderboardFilename, ios::app);

    if (playerFile.is_open() && leaderboardFile.is_open())
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

        // Calculate total best times by summing the best times for each map
        int totalRecords = 0;
        for (const auto &pair : bestTimes)
        {
            totalRecords += pair.second;
        }

        // Read the leaderboard file
        map<string, tuple<int, int, int>> leaderboardMap;
        ifstream leaderboardInFile(leaderboardFilename);
        if (leaderboardInFile.is_open())
        {
            while (getline(leaderboardInFile, line))
            {
                istringstream iss(line);
                string name;
                int wins, bestTime, games;
                if (getline(iss, name, ',') &&
                    (iss >> wins) && iss.ignore() &&
                    (iss >> bestTime) && iss.ignore() &&
                    (iss >> games))
                {
                    leaderboardMap[name] = make_tuple(wins, bestTime, games);
                }
            }
            leaderboardInFile.close();

            // Update player leaderboard
            leaderboardMap[playername] = make_tuple(totalWins, totalRecords, totalGames);

            // Write the updated leaderboard back to the file
            ofstream leaderboardOutFile(leaderboardFilename);
            if (leaderboardOutFile.is_open())
            {
                for (const auto &entry : leaderboardMap)
                {
                    leaderboardOutFile << entry.first << ","
                                       << get<0>(entry.second) << ","
                                       << get<1>(entry.second) << ","
                                       << get<2>(entry.second) << "\n";
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
void recursiveBacktrack(vector<vector<int>> &grid, int x, int y, int destX, int destY, int a_l, int a_u, int &path_sum, mt19937 &gen, vector<pair<int, int>> &pathCells, int &maxPathCells)
{
    if (x < 0 || x >= grid.size() || y < 0 || y >= grid[0].size() || grid[x][y] != 0)
    {
        return;
    }

    if (x == destX && y == destY)
    {
        grid[x][y] = path_sum;
        pathCells.push_back({x, y});
        return;
    }

    if (maxPathCells <= 0)
    { // Check if maximum path length is reached
        return;
    }

    grid[x][y] = getRandomInt(a_l, a_u, gen);
    if (grid[x][y] == 0)
    {
        grid[x][y] = getRandomInt(a_l, a_u, gen);
    }
    path_sum += grid[x][y];
    pathCells.push_back({x, y});
    maxPathCells--; // Decrement the number of remaining cells that can be added to the path

    vector<pair<int, int>> Orientations = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    shuffle(Orientations.begin(), Orientations.end(), gen);

    for (const auto &Orientation : Orientations)
    {
        int newX = x + Orientation.first;
        int newY = y + Orientation.second;

        int savedPathSum = path_sum;
        vector<pair<int, int>> savedPath = pathCells;
        int savedMaxPathCells = maxPathCells;

        recursiveBacktrack(grid, newX, newY, destX, destY, a_l, a_u, path_sum, gen, pathCells, maxPathCells);

        if (grid[destX][destY] != 0)
        {
            return; // Path found
        }
        else
        {
            path_sum = savedPathSum;
            pathCells = savedPath;
            maxPathCells = savedMaxPathCells;
        }
    }
}

vector<vector<int>> create_grid(int x, int y, int a_l, int a_u, int b_l, int b_u, int path_length, mt19937 &gen)
{
    vector<vector<int>> grid(x, vector<int>(y, 0));
    vector<pair<int, int>> pathCells(path_length);
    int path_sum = 0;
    // Generate path with random numbers within the range [a_l, a_u] excluding 0
    recursiveBacktrack(grid, 0, 0, x - 1, y - 1, a_l, a_u, path_sum, gen, pathCells, path_length);

    for (int i = 0; i < x; i++)
    {
        for (int j = 0; j < y; j++)
        {
            if (grid[i][j] == 0)
            {
                grid[i][j] = getRandomInt(a_l, a_u, gen);
            }
        }
    }
    // change some non-path cells to 0 but not the start and end cells and also not the cells in the path (pathCells)
    int num_cells_to_change = getRandomInt(b_l, b_u, gen);
    int cells_changed = 0;
    while (cells_changed < num_cells_to_change)
    {
        int x_pos = getRandomInt(0, x - 1, gen);
        int y_pos = getRandomInt(0, y - 1, gen);
        if (!(x_pos == 0 && y_pos == 0) && !(x_pos == x - 1 && y_pos == y - 1) && find(pathCells.begin(), pathCells.end(), make_pair(x_pos, y_pos)) == pathCells.end())
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
                     << "\x1B[32m" << cell << "\x1B[0m" << setw(pad_right) << ""
                     << "|"; // Start position in green
            }
            else if (i == grid.size() - 1 && j == grid[0].size() - 1)
            {
                cout << setw(pad_left) << ""
                     << "\x1B[34m" << cell << "\x1B[0m" << setw(pad_right) << ""
                     << "|"; // End position in blue
            }
            else if (path[i][j])
            {
                cout << setw(pad_left) << ""
                     << "\x1B[33m" << cell << "\x1B[0m" << setw(pad_right) << ""
                     << "|"; // Cells in the path in yellow
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
    cout << "Enter command (W:up, A:left, S:down, D:right) : ";

    while (true)
    {
        cin >> command;

        // Increment moves only when a valid move is made
        int prev_x = x;
        int prev_y = y;
        playsum += grid[x][y];
        if (ncount == 0)
        {
            cout << "You got to deadend ." << endl;
            cout << "if you want to play again (A or a) , if you want Quite (Q or q) : ";
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
            return;
        }
        switch (command)
        {
        case 'W': // Move up
        case 'w':
            x--;
            if (x >= 0 && grid[x][y] != 0 && !path[x][y]) // valid move
            {
                moves++;
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
            cout << "You gave up. Your final position is (" << x << ", " << y << ").\n";
            cout << "if you want to play again (A or a) , if you want Quit (Q or q) : ";
            int choice;
            double subchoice;
            char playAgain;
            cin >> playAgain;

            if (playAgain == 'A' || playAgain == 'a')
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
                    return;
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
                    return;
                }
            }
        }
        clear_screen();
        display_grid(grid, path);
    }
}

void easyMode()
{
    int x, y, a_l = -3, a_u = 3, b_l = 2, b_u = 5, path_length;
    int min_plen, max_plen;
    cout << "Enter the number of rows: ";
    cin >> x;
    cout << "Enter the number of columns: ";
    cin >> y;
    path_length = x + y - 2;
    cout << "Path length: " << path_length << endl;
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
    int x, y, a_l, a_u, b_l, b_u, path_length;
    int min_plen, max_plen;
    cout << "Enter the number of rows: ";
    cin >> x;
    cout << "Enter the number of columns: ";
    cin >> y;
    min_plen = x + y - 2;
    max_plen = x * y - 2;
    cout << "Enter the path length: ";
    cin >> path_length;
    while (path_length < min_plen || path_length >= max_plen)
    {
        cout << "Invalid path length. Please enter a value between " << min_plen << " and " << max_plen << ": ";
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
    cout << "Please enter the path period's lower (a_l) and upper (a_u) bounds: ";
    cin >> a_l >> a_u;
    cout << "Please enter the lower (b_l) and upper (b_u) bounds for the rest of the grid: ";
    cin >> b_l >> b_u;
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

            if (getline(iss, playerName, ',') &&
                (iss >> totalWins) && iss.ignore() &&
                (iss >> totalBestRec) && iss.ignore() &&
                (iss >> totalGames))
            {
                playerRecords.push_back({playerName, totalWins, totalBestRec, totalGames});
            }
        }

        // Sort the player records. (total win -> lesser total time)
        sort(playerRecords.begin(), playerRecords.end());

        TextTable rec;
        rec.setAlignment(0, TextTable::Alignment::CENTER);
        rec.setAlignment(1, TextTable::Alignment::LEFT);
        rec.setAlignment(2, TextTable::Alignment::CENTER);
        rec.setAlignment(3, TextTable::Alignment::CENTER);
        rec.setAlignment(4, TextTable::Alignment::CENTER);

        // Add header row to the table
        rec.add("Rank");
        rec.add("Player");
        rec.add("Total Wins");
        rec.add("Total Records");
        rec.add("Total Games");
        rec.endOfRow();

        // Add player records to the table
        for (size_t i = 0; i < playerRecords.size(); ++i)
        {
            rec.add(to_string(i + 1));
            rec.add(playerRecords[i].playerName);
            rec.add(to_string(playerRecords[i].totalWins));
            rec.add(to_string(playerRecords[i].totalBestRec));
            rec.add(to_string(playerRecords[i].totalGames));
            rec.endOfRow();
        }

        // Display the leaderboard table
        cout << rec << endl;

        leaderboardFile.close();
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

void printPath(const vector<vector<pair<bool, Direction>>> &visited)
{
    for (int i = 0; i < visited.size(); i++)
    {
        for (int j = 0; j < visited[i].size(); j++)
        {
            if (visited[i][j].first)
            {
                // Adjust the spacing based on the position
                for (int k = 0; k < j; k++)
                {
                    cout << " ";
                }
                cout << visited[i][j].second.symbol << "\n";
            }
        }
    }
}

void solveMaze(vector<vector<int>> &maze, int maxPathLength)
{
    int targetSum = maze.back().back();
    vector<vector<pair<bool, Direction>>> visited(maze.size(), vector<pair<bool, Direction>>(maze[0].size(), {false, {0, 0, ""}}));
    int pathLength = 0;

    if (dfs(maze, visited, 0, 0, 0, targetSum, maxPathLength, pathLength))
    {
        display_SolvedMaze(maze, visited);
        cout << "Path:\n";
        printPath(visited);
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
                     << "\x1B[32m" << cell << "\x1B[0m" << setw(pad_right) << ""
                     << "|"; // Start position in green
            }
            else if (i == grid.size() - 1 && j == grid[0].size() - 1)
            {
                cout << setw(pad_left) << ""
                     << "\x1B[34m" << cell << "\x1B[0m" << setw(pad_right) << ""
                     << "|"; // End position in blue
            }
            else if (path[i][j].first)
            {
                cout << setw(pad_left) << ""
                     << "\x1B[31m" << cell << "\x1B[0m" << setw(pad_right) << ""
                     << "|"; // Cells in the path in red
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
