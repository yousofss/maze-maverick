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

void HardMood();

void saverec(const string &playerName, const chrono::seconds &game_duration, const string &filename, const string &mapname, bool win);

void updateLeaderboard(const string &playername, const string &leaderboardFilename);

void recursiveBacktrack(vector<vector<int>> &grid, int x, int y, int destX, int destY, int a_l, int a_u, int b_l, int b_u, int &path_sum, mt19937 &gen);

vector<vector<int>> create_grid(int x, int y, int a_l, int a_u, int b_l, int b_u, int path_length, mt19937 &gen);

void save_grid(const vector<vector<int>> &grid, const string &filename, int cell_width, int path_length);

void display_grid(const vector<vector<int>> &grid, const vector<vector<bool>> &path);

void handle_commands(vector<vector<int>> &grid, vector<vector<bool>> &path, int &x, int &y, int &path_length, const string &playername, const string &mapname);

void displayMaps();

void displayPlayerInfo(const string &playerName);

void displayLeaderboard(const string &filename); // leaderboard

void displayrec(const string &filename, int start, int count) // it's better but it should be fix for remain lees than 10 rows it shouldn't asl for showing another page
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

        reverse(playerRecords.begin(), playerRecords.end());

        TextTable rec;
        rec.setAlignment(0, TextTable::Alignment::LEFT);
        rec.setAlignment(1, TextTable::Alignment::LEFT);
        rec.setAlignment(2, TextTable::Alignment::LEFT);
        rec.setAlignment(3, TextTable::Alignment::RIGHT);
        rec.setAlignment(4, TextTable::Alignment::RIGHT);

        rec.add("Player");
        rec.add("Map");
        rec.add("Duration (seconds)");
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
    cout << "----- Maze Maverick Menu -----" << endl;
    cout << "1. Create a New Map" << endl;
    cout << "2. Playground" << endl;
    cout << "3. Solve a Maze" << endl;
    cout << "4. History" << endl;
    cout << "5. Player Information" << endl;
    cout << "6. Leaderboard" << endl;
    cout << "7. Exit" << endl;
    return;
}

void Select_Choice(int choice)
{
    string playerFilePath = "./Users/" + playername + ".csv";
    bool fileExists = doesFileExist(playerFilePath);
    if (choice == 1)
    {
        cout << "1.1 Easy" << endl;
        cout << "1.2 Hard" << endl;
        cout << "Enter your choice: ";
    }
    else if (choice == 2)
    {
        if (!fileExists)
        {
            ofstream playerFile("./Users/" + playername + ".csv", ios::app);
            playerFile.close();
        }
        cout << "2.1 Choose from Existing Maps" << endl;
        cout << "2.2 Import a Custom Map" << endl;
        cout << "Enter your choice: ";
    }
    else if (choice == 3)
    {
        cout << "3.1 Choose from Existing Maps" << endl;
        cout << "3.2 Import a Custom Map" << endl;
        cout << "Enter your choice: ";
    }
    else if (choice == 4)
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
    else if (choice == 5)
    {
        displayPlayerInfo(playername);
        displayMenu();
        cin >> choice;
        Select_Choice(choice);
        return;
    }
    else if (choice == 6)
    {
        displayLeaderboard("Leaderboard.csv");
        displayMenu();
        cin >> choice;
        Select_Choice(choice);
        return;
    }
    else if (choice == 7)
    {
        exit(1);
    }
    else
    {
        cout << "Invalid choice" << endl;
        displayMenu();
        cin >> choice;
        Select_Choice(choice);
    }
}

void do_Choice(double &subchoice)
{
    if (subchoice == 1.2)
    {
        HardMood();
    }
    else if (subchoice == 2.1)
    {
        displayMaps();
        cout << "Enter the name of the map you want to play with: ";
        string mapname;
        cin >> mapname;

        ifstream file("./Maps/" + mapname); // all maps are in the "./Maps/" directory
        if (file.is_open())
        {
            string line;
            int read_path_length = -1;
            while (getline(file, line))
            {
                if (line.find("PathLength: ") == 0)
                {
                    read_path_length = stoi(line.substr(12));
                    break;
                }
            }

            if (read_path_length == -1)
            {
                cout << "Error: PathLength information not found in the map file.\n";
                return;
            }

            path_length = read_path_length;
            cout << path_length;

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
        }
    }
    else if (subchoice == 2.2)
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

            // Read the first line for path length
            if (getline(file, line))
            {
                istringstream iss(line);
                string pathLengthLabel;
                if (!(iss >> pathLengthLabel >> pathLength) || pathLengthLabel != "PathLength:")
                {
                    cout << "Error: First line does not contain valid 'PathLength' format.\n";
                    return;
                }
            }
            else
            {
                cout << "Error: Unable to read the first line for 'PathLength'.\n";
                return;
            }
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
}

void Resetgame(int &x, int &y, vector<vector<int>> &grid, vector<vector<bool>> &path)
{
    x = 0;
    y = 0;
    path = vector<vector<bool>>(grid.size(), vector<bool>(grid[0].size(), false));
    path[0][0] = true;
    int choice;
    double subchoice;
    do
    {
        displayMenu();
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            Select_Choice(choice);
            cin >> subchoice;
            do_Choice(subchoice);
            break;
        case 2:
            Select_Choice(choice);
            cin >> subchoice;
            do_Choice(subchoice);
            break;
        case 3:
            Select_Choice(choice);
            break;
        case 4:
            Select_Choice(choice);
            break;
        case 5:
            Select_Choice(choice);
            break;
        case 6:
            Select_Choice(choice);
            break;
        case 7:
            cout << "Exiting the program. Goodbye!\n";
            return;
        default:
            cout << "Invalid choice. Please try again.\n";
            break;
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
        cout << "player with this name has founded , is this yourself?(y/n) " << endl;
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

    cout << "Hello, " + playername + " Welcome to Maze Maverick\n";
    displayMenu();
    int choice;
    cout << "Enter your choice: " << endl;
    cin >> choice;
    Select_Choice(choice);
    double subchoice;
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

void saverec(const string &playerName, const chrono::seconds &game_duration, const string &filename, const string &mapname, bool win) //  we should delate date and Ranking in the right way , find players time win and calculate wins for every map one time
{
    ofstream historyFile(filename, ios::app);
    if (historyFile.is_open())
    {
        auto current_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
        struct tm *timeinfo;
        timeinfo = localtime(&current_time);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%m/%d/%Y", timeinfo);

        historyFile << playerName << "," << game_duration.count() << "," << buffer << "," << mapname << "," << (win ? "win" : "loss") << "\n";
        historyFile.close();
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

void save_grid(const vector<vector<int>> &grid, const string &filename, int cell_width, int path_length)
{
    string full_filepath = "./Maps/" + filename;

    ofstream file(full_filepath);

    if (file.is_open())
    {
        file << "PathLength: " << path_length << "\n";

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
    int cell_width = to_string(largest_num).length() + 1;
    for (int i = 0; i < grid.size(); i++)
    {
        for (int j = 0; j < grid[i].size(); j++)
        {
            if (i == 0 && j == 0)
            {
                cout << "\x1B[" << START_COLOR << "m" << setw(cell_width) << grid[i][j] << "\x1B[0m"; // Print the start position in green
            }
            else if (i == grid.size() - 1 && j == grid[0].size() - 1)
            {
                cout << "\x1B[" << END_COLOR << "m" << setw(cell_width) << grid[i][j] << "\x1B[0m"; // Print the end position in blue
            }
            else if (path[i][j])
            {
                cout << "\x1B[" << PATH_COLOR << "m" << setw(cell_width) << grid[i][j] << "\x1B[0m"; // Print the cells in the path in yellow
            }
            else
            {
                cout << setw(cell_width) << grid[i][j];
            }
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
        moves++;
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
                cout << "GOOD GAME!\n";
            }
            return;
        }
        switch (command)
        {
        case 'W': // Move up
        case 'w':
            x--;                                         // Decrease the row index
            if (x <= 0 || grid[x][y] == 0 || path[x][y]) // invalid move set things back
            {
                x++;
                moves--;
                ncount--;
                playsum -= grid[x][y];
            }
            break;
        case 'A': // Move left
        case 'a':
            y--;                                         // Decrease the column index
            if (y <= 0 || grid[x][y] == 0 || path[x][y]) // invalid move set things back
            {
                y++;
                moves--;
                ncount--;
                playsum -= grid[x][y];
            }
            break;
        case 'S': // Move down
        case 's':
            x++;                                                      // Increase the row index
            if (x > grid.size() - 1 || grid[x][y] == 0 || path[x][y]) // invalid move set things back
            {
                x--;
                moves--;
                ncount--;
                playsum -= grid[x][y];
            }
            break;
        case 'D': // Move right
        case 'd':
            y++;                                                         // increase the column index
            if (y > grid[0].size() - 1 || grid[x][y] == 0 || path[x][y]) // invalid move set things back
            {
                y--;
                moves--;
                ncount--;
                playsum -= grid[x][y];
            }
            break;
        case 'Q':
        case 'q':
            return;
        case 'G':
        case 'g':
            cout << "You gave up. Your final position is (" << x << ", " << y << ").\n";
            cout << "if you want to play again (A or a) , if you want Quit (Q or q) : ";
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
                cout << "GOOD GAME!\n";
                return;
            }
        default:
            cout << "Invalid command. Please try again.\n";
            moves--;
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
                    cout << "GOOD GAME!\n";
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
                    cout << "GOOD GAME!\n";
                    return;
                }
            }
        }
        clear_screen();
        display_grid(grid, path);
    }
}

void HardMood()
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
    cout << "Enter the lower and upper bounds for the path period [a_l, a_u]: ";
    cin >> a_l >> a_u;
    cout << "Enter the lower and upper bounds for the rest of the grid [b_l, b_u]: ";
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

    save_grid(grid, mapname, cell_width, path_length);

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

        cout << "Player: " << playerName << "\n";
        cout << "Total Games played: " << totalGames << "\n";
        cout << "Total Wins: " << totalWins << "\n";
        cout << "Last Win Record: " << lastWinRec << "\n";
        cout << "Last Game Date: " << lastGameDate << "\n"; // Display the date of the last game
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
        rec.setAlignment(0, TextTable::Alignment::RIGHT);
        rec.setAlignment(1, TextTable::Alignment::LEFT);
        rec.setAlignment(2, TextTable::Alignment::RIGHT);
        rec.setAlignment(3, TextTable::Alignment::RIGHT);
        rec.setAlignment(4, TextTable::Alignment::RIGHT);

        // Add header row to the table
        rec.add("Rank");
        rec.add("Player");
        rec.add("Total Wins");
        rec.add("Total Best Time (seconds)");
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
        cout << "Leaderboard Table:\n"
             << rec << endl;

        leaderboardFile.close();
    }
    else
    {
        cout << "Unable to open " << filename << " for reading leaderboard.\n";
    }
}