#include <iostream>
#include <vector>
#include <string>

// Utility function to check if a move is valid
bool isValidMove(int x, int y, const std::vector<std::vector<int>> &maze, const std::vector<std::vector<bool>> &visited)
{
    // Check boundaries and if the cell is not a block and not visited
    return x >= 0 && x < maze.size() && y >= 0 && y < maze[0].size() && maze[x][y] != 0 && !visited[x][y];
}

// DFS function to find the path
bool dfs(std::vector<std::vector<int>> &maze, std::vector<std::vector<bool>> &visited, int x, int y, int sum, int target, int maxPathLength, int pathLength)
{
    // Base case: If we reached the end
    if (x == maze.size() - 1 && y == maze[0].size() - 1)
    {
        return sum == target;
    }

    // Mark the current cell as visited
    visited[x][y] = true;

    // Path length exceeded the limit
    if (pathLength > maxPathLength)
    {
        visited[x][y] = false;
        return false;
    }

    // Directions: right, down, left, up
    std::vector<std::pair<int, int>> directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    for (const auto &dir : directions)
    {
        int newX = x + dir.first;
        int newY = y + dir.second;
        if (isValidMove(newX, newY, maze, visited))
        {
            if (dfs(maze, visited, newX, newY, sum + maze[x][y], target, maxPathLength, pathLength + 1))
            {
                return true;
            }
        }
    }

    // If path is not found, backtrack
    visited[x][y] = false;
    return false;
}

void solveMaze(std::vector<std::vector<int>> &maze, int maxPathLength)
{
    // Assume the maze's bottom right corner holds the target sum
    int targetSum = maze.back().back();

    // Create a visited 2D vector initialized to false
    std::vector<std::vector<bool>> visited(maze.size(), std::vector<bool>(maze[0].size(), false));

    // Start DFS from the top left corner
    if (dfs(maze, visited, 0, 0, 0, targetSum, maxPathLength, 0))
    {
        // Print the maze with the correct path marked in a different color
        for (int i = 0; i < maze.size(); i++)
        {
            for (int j = 0; j < maze[i].size(); j++)
            {
                if (visited[i][j])
                {
                    // Red color for path
                    std::cout << "\x1B[31m" << maze[i][j] << "\x1B[0m ";
                }
                else
                {
                    std::cout << maze[i][j] << " ";
                }
            }
            std::cout << std::endl;
        }
    }
    else
    {
        std::cout << "No path found that meets the conditions." << std::endl;
    }
}

int main()
{
    // Example usage:
    std::vector<std::vector<int>> maze = {
        {3, 1, 4, 1},
        {0, 4, 7, 6},
        {0, 0, 0, 5},
        {8, 4, 0, 31},
    };
    solveMaze(maze, 7);

    return 0;
}
