import numpy as np
from PIL import Image, ImageDraw, ImageFont
import random
import math
import os
import time
import datetime
import requests # To download the font
from skimage import filters # For edge detection

def download_font(url, path="pixel_font.ttf"):
    """Downloads a font file if it doesn't exist."""
    if not os.path.exists(path):
        print(f"Downloading font from {url}...")
        try:
            response = requests.get(url, stream=True)
            response.raise_for_status()
            with open(path, 'wb') as f:
                for chunk in response.iter_content(chunk_size=8192):
                    f.write(chunk)
            print("Font downloaded successfully.")
            return path
        except requests.exceptions.RequestException as e:
            print(f"Error downloading font: {e}")
            return None
    else:
        # print("Font already exists.")
        return path

def create_time_image(text, font_path, width=256, height=256, path="time_image.png"):
    """Generates an image with the given text."""
    img = Image.new('L', (width, height), color=255) # Pure white background
    draw = ImageDraw.Draw(img)
    
    try:
        font = ImageFont.truetype(font_path, size=120)
    except IOError:
        print(f"Error: Font file not found at '{font_path}'. Using default font.")
        font = ImageFont.load_default()

    text_bbox = draw.textbbox((0, 0), text, font=font)
    text_width = text_bbox[2] - text_bbox[0]
    text_height = text_bbox[3] - text_bbox[1]
    position = ((width - text_width) / 2, (height - text_height) / 2)
    
    draw.text(position, text, font=font, fill=0)
    img.save(path)
    return path

def distribute_points_on_image(image_path, num_points, min_distance, edge_bias=0.4):
    """
    Distributes points on an image using an edge-biased, two-phase approach.

    Args:
        image_path (str): The path to the input image.
        num_points (int): The total number of points to distribute.
        min_distance (float): The minimum required distance between any two points.
        edge_bias (float): The percentage of points to place along edges (0.0 to 1.0).
    """
    try:
        img = Image.open(image_path).convert('L')
        pixels = np.array(img)
        width, height = img.size
    except FileNotFoundError:
        print(f"Error: Image file not found at {image_path}")
        return [], None

    # --- Create Probability Maps ---
    # 1. Density Map (for filling)
    density_weights = (255.0 - pixels.astype(np.float64)) + 1e-6

    # 2. Edge Map (for outlines)
    edge_pixels = filters.sobel(pixels)
    edge_weights = (edge_pixels / np.max(edge_pixels)) * 255.0 + 1e-6
    
    def sample_from_map(weights, num_samples, existing_points):
        """Helper function to run rejection sampling on a given weight map."""
        flat_weights = weights.flatten()
        probabilities = flat_weights / np.sum(flat_weights)
        pixel_indices = np.arange(width * height)
        
        points = []
        min_dist_sq = min_distance**2
        max_attempts = num_samples * 200
        attempts = 0

        while len(points) < num_samples and attempts < max_attempts:
            attempts += 1
            random_index = np.random.choice(pixel_indices, p=probabilities)
            y, x = divmod(random_index, width)
            candidate_point = (x, y)
            
            is_valid = True
            # Check against points from this phase AND previous phases
            for point in existing_points + points:
                dist_sq = (point[0] - candidate_point[0])**2 + (point[1] - candidate_point[1])**2
                if dist_sq < min_dist_sq:
                    is_valid = False
                    break
            
            if is_valid:
                points.append(candidate_point)
        
        if attempts >= max_attempts and len(points) < num_samples:
             print(f"  Warning: Reached max attempts in phase. Got {len(points)}/{num_samples} points.")
        
        return points

    # --- Two-Phase Sampling ---
    num_edge_points = int(num_points * edge_bias)
    num_density_points = num_points - num_edge_points

    print(f"Phase 1: Placing {num_edge_points} edge points...")
    edge_points = sample_from_map(edge_weights, num_edge_points, [])
    
    print(f"Phase 2: Placing {num_density_points} density points...")
    density_points = sample_from_map(density_weights, num_density_points, edge_points)

    accepted_points = edge_points + density_points
    return accepted_points, img.size

def visualize_result(img_size, points, output_path):
    """Draws the points on a new blank image and saves it."""
    img = Image.new('RGB', img_size, 'white')
    draw = ImageDraw.Draw(img)
    
    for x, y in points:
        draw.ellipse((x-1, y-1, x+1, y+1), fill='black')
        
    img.save(output_path)
    print(f"Visualization saved to '{output_path}'")

# --- Main Execution ---
if __name__ == "__main__":
    FONT_URL = "https://github.com/google/fonts/raw/main/ofl/pressstart2p/PressStart2P-Regular.ttf"
    FONT_PATH = download_font(FONT_URL)

    if FONT_PATH:
        current_time_str = datetime.datetime.now().strftime("%S")
        print(f"\n--- Generating for time: {current_time_str} ---")
        
        time_image_path = create_time_image(current_time_str, FONT_PATH)
        
        # --- Configuration ---
        # With edge biasing, even a low number of points can be recognizable.
        # Try running with 200, 500, and then 4000 to see the difference.
        NUM_POINTS = 100
        MIN_SPACING = 7
        # Use 40% of points to define the edges, 60% to fill density.
        EDGE_BIAS = 0.4
        
        distributed_points, image_size = distribute_points_on_image(
            image_path=time_image_path,
            num_points=NUM_POINTS,
            min_distance=MIN_SPACING,
            edge_bias=EDGE_BIAS
        )

        if distributed_points:
            output_filename = f"time_dots_edge_{current_time_str.replace(':', '-')}_{NUM_POINTS}pts.png"
            visualize_result(
                img_size=image_size,
                points=distributed_points,
                output_path=output_filename
            )
        
        # if os.path.exists(time_image_path):
        #     os.remove(time_image_path)
