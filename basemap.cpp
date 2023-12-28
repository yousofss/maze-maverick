#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <string>
#include <iomanip>
#include <algorithm>

using namespace std;

int x_pos, y_pos;

vector<vector<int>> create_grid(int x, int y, int a_l, int a_u, int b_l, int b_u, int path_length)
{
  vector<vector<int>> grid(x, vector<int>(y, 0));
  vector<vector<bool>> path(x, vector<bool>(y, false));
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> dis_a(a_l, a_u);
  uniform_int_distribution<> dis_b(b_l, b_u);

  // Generate the path
  for (int i = 0; i < path_length; i++)
  {
    int x_pos = i / y;
    int y_pos = i % y;
    int num;
    do
    {
      num = dis_a(gen);
    } while (num == 0);
    grid[x_pos][y_pos] = num;
    path[x_pos][y_pos] = true;
  }

  // Fill the rest of the grid
  for (int i = path_length; i < x * y; i++)
  {
    int x_pos = i / y;
    int y_pos = i % y;
    grid[x_pos][y_pos] = dis_b(gen);
  }

  return grid;
}

void save_grid(const vector<vector<int>> &grid, const string &filename, int cell_width)
{
  ofstream file(filename);
  if (file.is_open())
  {
    for (const auto &row : grid)
    {
      for (const auto &val : row)
      {
        file << setw(cell_width) << val;
      }
      file << "\n";
    }
    file.close();
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
        cout << "\x1B[32m" << setw(cell_width) << grid[i][j] << "\x1B[0m"; // Print the start position in green
      }
      else if (i == grid.size() - 1 && j == grid[0].size() - 1)
      {
        cout << "\x1B[34m" << setw(cell_width) << grid[i][j] << "\x1B[0m"; // Print the end position in blue
      }
      else if (path[i][j])
      {
        cout << "\x1B[33m" << setw(cell_width) << grid[i][j] << "\x1B[0m"; // Print the cells in the path in yellow
      }
      else
      {
        cout << setw(cell_width) << grid[i][j];
      }
    }
    cout << "\n";
  }
}

void handle_commands(vector<vector<int>> &grid, vector<vector<bool>> &path, int &x, int &y)
{
  //vector<pair<int, int>> path;
  char command;
  cout << "Enter command (W:up, A:left, S:down, D:right) : ";
  while (true)
  {
    cin >> command;
    switch (command)
    {
    case 'W':                           // Move up
      if (x > 0 && grid[x - 1][y] != 0) // Check if the upper cell is valid and not zero
      {
        x--; // Decrease the row index
      }
      break;
    case 'A':                           // Move left
      if (y > 0 && grid[x][y - 1] != 0) // Check if the left cell is valid and not zero
      {
        y--; // Decrease the column index
      }
      break;
    case 'S':                                         // Move down
      if (x < grid.size() - 1 && grid[x + 1][y] != 0) // Check if the lower cell is valid and not zero
      {
        x++; // Increase the row index
      }
      break;
    case 'D':                                            // Move right
      if (y < grid[0].size() - 1 && grid[x][y + 1] != 0) // Check if the right cell is valid and not zero
      {
        y++;
      }
      break;
    case 'Q':
      return;
    case 'G':
      cout << "You gave up. Your final position is (" << x << ", " << y << ").\n";
      return;
    default: 
      cout << "Invalid command. Please try again.\n";
      continue; 
    }
    x_pos = x;
    y_pos = y;
    path[x][y] = true;
    display_grid(grid, path);
    cout << "-------------------------------------------"  << endl;;
  }
}

int main()
{
  int x, y, a_l, a_u, b_l, b_u, path_length;
  cout << "Enter the number of rows: ";
  cin >> x;
  cout << "Enter the number of columns: ";
  cin >> y;
  cout << "Enter the path length: ";
  cin >> path_length;
  cout << "Enter the lower and upper bounds for the path period [a_l, a_u]: ";
  cin >> a_l >> a_u;
  cout << "Enter the lower and upper bounds for the rest of the grid [b_l, b_u]: ";
  cin >> b_l >> b_u;

  vector<vector<int>> grid = create_grid(x, y, a_l, a_u, b_l, b_u, path_length);
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

  save_grid(grid, filename, cell_width);
  cout << "Grid saved to " << filename << "\n";
  //vector<pair<int, int>> path;
  vector<vector<bool>> path(x, vector<bool>(y, false));
  path[0][0] = true; // Mark the start position as part of the path
  //path.push_back(make_pair(0, 0));

  display_grid(grid, path);
  handle_commands(grid, path, x_pos, y_pos);

  return 0;
}