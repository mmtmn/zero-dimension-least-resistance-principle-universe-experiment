import numpy as np
import matplotlib.pyplot as plt
import random

# Initialize the grid
grid_size = 100
grid = np.zeros((grid_size, grid_size))

# Starting point
start_point = (grid_size // 2, grid_size // 2)
grid[start_point] = 1

# Function to find least resistance direction
def find_least_resistance(grid, x, y):
    directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]
    random.shuffle(directions)  # Randomize to avoid biased expansion
    
    min_resistance = float('inf')
    best_direction = None
    
    for dx, dy in directions:
        nx, ny = x + dx, y + dy
        if 0 <= nx < grid_size and 0 <= ny < grid_size:
            if grid[nx, ny] < min_resistance:
                min_resistance = grid[nx, ny]
                best_direction = (nx, ny)
    
    return best_direction

# Simulation parameters
iterations = 5000

# Run the simulation
for _ in range(iterations):
    new_x, new_y = find_least_resistance(grid, start_point[0], start_point[1])
    grid[new_x, new_y] += 1
    start_point = (new_x, new_y)

# Plot the grid
plt.imshow(grid, cmap='hot', interpolation='nearest')
plt.colorbar()
plt.title("Universe Simulation (Expansion from Zero-Dimensional Point)")
plt.show()
