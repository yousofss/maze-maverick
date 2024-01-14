#include <sstream>
#include <vector>
#include <fstream>
#include <random>
#include <string>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <iostream>

#define TEXTTABLE_ENCODE_MULTIBYTE_STRINGS
#define TEXTTABLE_USE_EN_US_UTF8

#include "TextTable.h"

using namespace std;
namespace fs = filesystem;

random_device rd;
mt19937 gen(rd());
const int START_COLOR = 32;
const int END_COLOR = 34;
const int PATH_COLOR = 33;
int path_length, path_sum;
string playername;

void clear_screen()
{
    cout << "\x1B[2J\x1B[H"; // ANSI escape codes to clear the screen
}
int getRandomInt(int a_l, int a_u, mt19937 &gen)
{
    uniform_int_distribution<> dis_a(a_l, a_u);
    return dis_a(gen);
}

void HardMood();

void saverec(const string &playerName, const chrono::seconds &game_duration, const string &filename);

void recursiveBacktrack(vector<vector<int>> &grid, int x, int y, int destX, int destY, int a_l, int a_u, int b_l, int b_u, int &path_sum, mt19937 &gen);

vector<vector<int>> create_grid(int x, int y, int a_l, int a_u, int b_l, int b_u, int path_length, mt19937 &gen);

void save_grid(const vector<vector<int>> &grid, const string &filename, int cell_width, int path_length);

void display_grid(const vector<vector<int>> &grid, const vector<vector<bool>> &path);

void handle_commands(vector<vector<int>> &grid, vector<vector<bool>> &path, int &x, int &y, int &path_length, const string &playername);

void displayMaps();

struct PlayerRecord
{
    string playerName;
    int duration;

    bool operator<(const PlayerRecord &other) const
    {
        return duration < other.duration;
    }
};

void diplayrec(const string &filename)
{
    ifstream historyFile(filename);
    if (historyFile.is_open())
    {
        vector<PlayerRecord> playerRecords;

        string line;
        while (getline(historyFile, line))
        {
            istringstream iss(line);
            string playerName;
            int duration;

            if (getline(iss, playerName, ',') && iss >> duration)
            {
                playerRecords.push_back({playerName, duration});
            }
        }

        sort(playerRecords.begin(), playerRecords.end());

        TextTable rec;
        rec.setAlignment(0, TextTable::Alignment::LEFT);
        rec.setAlignment(1, TextTable::Alignment::LEFT);
        rec.setAlignment(2, TextTable::Alignment::RIGHT);

        rec.add("Rank");
        rec.add("Player");
        rec.add("Duration (seconds)");
        rec.endOfRow();

        for (size_t i = 0; i < playerRecords.size(); ++i)
        {
            rec.add(to_string(i + 1));                     // Rank
            rec.add(playerRecords[i].playerName);          // Player
            rec.add(to_string(playerRecords[i].duration)); // Duration
            rec.endOfRow();
        }

        cout << "Record Table:\n"
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
    cout << "5. Leaderboard" << endl;
    cout << "6. Exit" << endl;
    return;
}

void Select_Choice(int choice)
{
    if (choice == 1)
    {
        cout << "1.1 Easy" << endl;
        cout << "1.2 Hard" << endl;
        cout << "Enter your choice: ";
    }
    else if (choice == 2)
    {
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
        diplayrec("player_history.csv");
        displayMenu();
        cin >> choice;
        Select_Choice(choice);
    }
    else if (choice == 5)
    {
        cout << "null5" << endl;
        return;
    }
    else if (choice == 6)
    {
        return;
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
        string mapName;
        cin >> mapName;

        ifstream file("./Maps/" + mapName); // all maps are in the "./Maps/" directory
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
            handle_commands(grid, path, x_pos, y_pos, path_length, playername);
        }
        else
        {
            cout << "Error: Unable to open the selected map.\n";
        }
    }
    else if (subchoice == 2.2)
    {
        cout << "Enter the path to the grid file: " << endl;
        string gridPath;
        cin >> gridPath;

        ifstream file(gridPath);
        if (file.is_open())
        {
            
            vector<vector<int>> grid;
            
            int cell_value;
            while (file >> cell_value)
            {
                grid.push_back({cell_value});
            }
            int x_pos = 0;
            int y_pos = 0;
            vector<vector<bool>> path(grid.size(), vector<bool>(grid[0].size(), false));
            path[0][0] = true; 
            display_grid(grid, path);
            handle_commands(grid, path, x_pos, y_pos, path_length, playername);
        }
        else
        {
            cout << "Error: Unable to open the specified grid file.\n";
        }
    }
}

int main()
{
    int rows, columns, pathLength, minCellValue, maxCellValue;
    cout << "Enter your name: ";
    getline(cin, playername);
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

void saverec(const string &playerName, const chrono::seconds &game_duration, const string &filename)
{
    ofstream historyFile(filename, ios::app);
    if (historyFile.is_open())
    {
        historyFile << playerName << "," << game_duration.count() << "\n";
        historyFile.close();
    }
    else
    {
        cout << "Unable to open " << filename << " for recording player history.\n";
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
                grid[i][j] = getRandomInt(b_l, b_u, gen);
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

void handle_commands(vector<vector<int>> &grid, vector<vector<bool>> &path, int &x, int &y, int &path_length, const string &playername)
{
    chrono::seconds game_duration;
    auto start_time = chrono::steady_clock::now();

    char command;
    int x_pos, y_pos;
    int moves = 0;

    cout << "Enter command (W:up, A:left, S:down, D:right) : ";

    while (true)
    {
        cin >> command;
        moves++;

        switch (command)
        {
        case 'W': // Move up
        case 'w':
            if (x > 0 && grid[x - 1][y] != 0 && !path[x - 1][y])
            {
                x--; // Decrease the row index
            }
            break;
        case 'A': // Move left
        case 'a':
            if (y > 0 && grid[x][y - 1] != 0 && !path[x][y - 1])
            {
                y--; // Decrease the column index
            }
            break;
        case 'S': // Move down
        case 's':
            if (x < grid.size() - 1 && grid[x + 1][y] != 0 && !path[x + 1][y])
            {
                x++; // Increase the row index
            }
            break;
        case 'D': // Move right
        case 'd':
            if (y < grid[0].size() - 1 && grid[x][y + 1] != 0 && !path[x][y + 1])
            {
                y++;
            }
            break;
        case 'Q':
        case 'q':
            return;
        case 'G':
        case 'g':
            cout << "You gave up. Your final position is (" << x << ", " << y << ").\n";
            return;
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
            if (x == grid.size() - 1 && y == grid[0].size() - 1)
            {
                saverec(playername, game_duration, "player_history.csv");
                cout << "YOU WON!\n";
                int choice;
                double subchoice;
                // Ask if the player wants to play again
                char playAgain;
                cout << "Do you want to play again? (Y/N): ";
                cin >> playAgain;

                if (playAgain == 'Y' || playAgain == 'y')
                {
                    // Reset the game state
                    x = 0;
                    y = 0;
                    path = vector<vector<bool>>(grid.size(), vector<bool>(grid[0].size(), false));
                    path[0][0] = true;

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
                            cout << "Leaderboard option selected. (To be implemented)\n";
                            break;
                        case 6:
                            cout << "Exiting the program. Goodbye!\n";
                            return;
                        default:
                            cout << "Invalid choice. Please try again.\n";
                            break;
                        }
                    } while (choice != 6);

                    continue;
                }
                else
                {
                    cout << "GOOD GAME!\n";
                    return;
                }
            }
            else
            {
                cout << "YOU LOSE! Reached maximum allowed moves\n";
                int choice;
                double subchoice;

                // Ask if the player wants to play again
                char playAgain;
                cout << "Do you want to play again? (Y/N): ";
                cin >> playAgain;

                if (playAgain == 'Y' || playAgain == 'y')
                {
                    // Reset the game state
                    x = 0;
                    y = 0;
                    path = vector<vector<bool>>(grid.size(), vector<bool>(grid[0].size(), false));
                    path[0][0] = true;

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
                            cout << "Leaderboard option selected. (To be implemented)\n";
                            break;
                        case 6:
                            cout << "Exiting the program. Goodbye!\n";
                            return;
                        default:
                            cout << "Invalid choice. Please try again.\n";
                            break;
                        }
                    } while (choice != 6);

                    continue;
                }
                else
                {
                    cout << "GOOD GAME!\n";
                }
                return;
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
    string filename;
    cout << "Say your grid name : ";
    cin >> filename;
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

    save_grid(grid, filename, cell_width, path_length);

    vector<vector<bool>> path(x, vector<bool>(y, false));
    path[0][0] = true;

    display_grid(grid, path);
    handle_commands(grid, path, x_pos, y_pos, path_length, playername);
    return;
}