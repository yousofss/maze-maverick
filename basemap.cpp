#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <string>
#include <iomanip>
#include <algorithm>

using namespace std;

const int START_COLOR = 32;
const int END_COLOR = 34;
const int PATH_COLOR = 33;

vector<vector<int>> create_grid(int x, int y, int a_l, int a_u, int b_l, int b_u, int path_length)
{
  vector<vector<int>> grid(x, vector<int>(y, 0));
  vector<vector<bool>> path(x, vector<bool>(y, false));
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> dis_a(a_l, a_u);
  uniform_int_distribution<> dis_b(b_l, b_u);

  int current_x = 0;
  int current_y = 0;
  int path_sum = 0;

  for (int i = 0; i < path_length - 1; i++) // Ensure the last step reaches [x][y]
  {
    int direction = gen() % 4; // 0: up, 1: left, 2: down, 3: right

    switch (direction)
    {
    case 0: // Move up
      if (current_x > 0)
        current_x--;
      break;
    case 1: // Move left
      if (current_y > 0)
        current_y--;
      break;
    case 2: // Move down
      if (current_x < x - 1)
        current_x++;
      break;
    case 3: // Move right
      if (current_y < y - 1)
        current_y++;
      break;
    }
    int num;
    do
    {
      num = dis_a(gen);
    } while (num == 0);

    path_sum += num;

    grid[current_x][current_y] = num;
    path[current_x][current_y] = true;
  }

  // last step must leads to [x][y]
  int remaining_x_steps = x - 1 - current_x;
  int remaining_y_steps = y - 1 - current_y;

  for (int i = 0; i < remaining_x_steps; i++)
  {
    current_x++;
    int num = dis_a(gen);
    path_sum += num;
    grid[current_x][current_y] = num;
    path[current_x][current_y] = true;
  }

  for (int i = 0; i < remaining_y_steps; i++)
  {
    current_y++;
    int num = dis_a(gen);
    path_sum += num;
    grid[current_x][current_y] = num;
    path[current_x][current_y] = true;
  }

  for (int i = 0; i < x; i++)
  {
    for (int j = 0; j < y; j++)
    {
      if (!path[i][j])
      {
        grid[i][j] = dis_b(gen);
      }
    }
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

void handle_commands(vector<vector<int>> &grid, vector<vector<bool>> &path, int &x, int &y)
{
  // vector<pair<int, int>> path;
  char command;
  int x_pos, y_pos;
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
    cout << "-------------------------------------------" << endl;
    ;
  }
}

int main()
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
    cout << "Invalid path length. Please enter a value between (" << min_plen << " , " << max_plen << ") : ";
    cin >> path_length;
  }
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
  vector<vector<bool>> path(x, vector<bool>(y, false));
  path[0][0] = true; // Mark the start position as part of the path

  display_grid(grid, path);
  handle_commands(grid, path, x_pos, y_pos);

  return 0;
}