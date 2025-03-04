import sqlite3
import matplotlib.pyplot as plt
import numpy as np
import math
import argparse

parser = argparse.ArgumentParser("plot-android-benchmark-results")
parser.add_argument("db", help="Path to SQLite database with results", type=str)
args = parser.parse_args()

# Connect to the SQLite database
conn = sqlite3.connect(args.db)  # Replace with your database file path
cursor = conn.cursor()

# Query to get the average FPS and variance for each style, renderer, and model where syncRendering=0
query = """
SELECT deviceManufacturer, model, styleName, renderer,
       AVG(fps) AS avg_fps,
       (
           AVG(fps * fps) - AVG(fps) * AVG(fps)
       ) AS variance_fps
FROM benchmark_result
WHERE syncRendering = 0
GROUP BY deviceManufacturer, model, styleName, renderer
ORDER BY deviceManufacturer, model, styleName, renderer
"""

# Execute the query
cursor.execute(query)
rows = cursor.fetchall()

# Close the connection to the database
conn.close()

# Organize the data into a dictionary for plotting
data = {}
for row in rows:
    manufacturer, model, style, renderer, avg_fps, variance_fps = row
    device = f"{manufacturer} {model}"
    if device not in data:
        data[device] = {}
    if style not in data[device]:
        data[device][style] = {'fps': [], 'stddev': []}
    stddev_fps = math.sqrt(max(variance_fps, 0))  # Calculate the standard deviation and ensure it's not negative
    data[device][style]['fps'].append((renderer, avg_fps))
    data[device][style]['stddev'].append((renderer, stddev_fps))

# Define the colors for each renderer
colors = {'legacy': 'gray', 'drawable': 'blue', 'vulkan': 'red'}
renderer_order = ['legacy', 'drawable', 'vulkan']

legend_names = {'legacy': 'Legacy', 'drawable': 'OpenGL', 'vulkan': 'Vulkan'}

# Create subplots for each device
num_devices = len(data)
fig, axes = plt.subplots(num_devices, 1, figsize=(10, 6 * num_devices))

if num_devices == 1:
    axes = [axes]

# Plot data for each device
for ax, (device, styles) in zip(axes, data.items()):
    # Get a sorted list of styles
    style_names = sorted(styles.keys())

    # Number of styles
    n_styles = len(style_names)

    # Create an array with the position of each group of bars
    bar_width = 0.25
    index = np.arange(n_styles)

    # Plot each renderer's bar with error bars in the defined order
    for i, renderer in enumerate(renderer_order):
        avg_fps_values = []
        stddev_values = []
        for style in style_names:
            fps_values = dict(styles[style]['fps'])
            stddev_values_dict = dict(styles[style]['stddev'])
            # Use get to return a default of 0 if the renderer is not present
            avg_fps_values.append(fps_values.get(renderer, 0))
            stddev_values.append(stddev_values_dict.get(renderer, 0))
        ax.bar(index + i * bar_width, avg_fps_values, bar_width, yerr=stddev_values,
               label=legend_names[renderer], color=colors[renderer], capsize=5, error_kw={'elinewidth': 1, 'ecolor': 'black'})

    # Set the title, labels, ticks, etc.
    ax.set_title(device)
    ax.set_xlabel('Style')
    ax.set_ylabel('Average FPS')
    ax.set_xticks(index + bar_width)
    ax.set_xticklabels(style_names)
    ax.legend(title='Renderer')

# Adjust layout
plt.tight_layout()

# Show the plot
plt.savefig('plot.png')
