import numpy as np
from PIL import Image, ImageDraw, ImageFont
import random
import math
import os
import time
import datetime
import requests # To download the font

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
        print("Font already exists.")
        return path

def create_time_image(text, font_path, width=256, height=256, path="time_image.png"):
    """Generates an image with the given text."""
    img = Image.new('L', (width, height), color=255) # Pure white background
    draw = ImageDraw.Draw(img)
    
    try:
        # Load a large font size, then resize down for pixelated effect
        font = ImageFont.truetype(font_path, size=120)
    except IOError:
        print(f"Error: Font file not found at '{font_path}'. Using default font.")
        font = ImageFont.load_default()

    # Calculate text size and position to center it
    text_bbox = draw.textbbox((0, 0), text, font=font)
    text_width = text_bbox[2] - text_bbox[0]
    text_height = text_bbox[3] - text_bbox[1]
    
    position = ((width - text_width) / 2, (height - text_height) / 2)
    
    # Draw the text in black
    draw.text(position, text, font=font, fill=0)
        
    img.save(path)
    return path

def distribute_points_on_image(image_path, num_points, min_distance):
    """
    Distributes points on an image based on pixel darkness with a minimum spacing.
    (This function remains the same as your original version)
    """
    try:
        # 1. Load and Prepare Image
        img = Image.open(image_path).convert('L') # Convert to grayscale
        pixels = np.array(img)
        width, height = img.size
    except FileNotFoundError:
        print(f"Error: Image file not found at {image_path}")
        return []

    # 2. Create Probability Distribution
    weights = (255.0 - pixels.astype(np.float64)) + 1e-6 
    flat_weights = weights.flatten()
    probabilities = flat_weights / np.sum(flat_weights)
    pixel_indices = np.arange(width * height)

    # 3. Iteratively Sample and Reject
    accepted_points = []
    min_dist_sq = min_distance**2
    max_attempts = num_points * 200 # Increased multiplier for dense text
    attempts = 0

    while len(accepted_points) < num_points and attempts < max_attempts:
        attempts += 1
        random_index = np.random.choice(pixel_indices, p=probabilities)
        y = random_index // width
        x = random_index % width
        candidate_point = (x, y)
        
        is_valid = True
        for point in accepted_points:
            dist_sq = (point[0] - candidate_point[0])**2 + (point[1] - candidate_point[1])**2
            if dist_sq < min_dist_sq:
                is_valid = False
                break
        
        if is_valid:
            accepted_points.append(candidate_point)
            
    if attempts >= max_attempts and len(accepted_points) < num_points:
         print(f"Warning: Reached max attempts. Generated {len(accepted_points)}/{num_points} points.")

    return accepted_points, img.size

def visualize_result(img_size, points, output_path):
    """Draws the points on a new blank image and saves it."""
    # Create a new blank (white) image for the output
    img = Image.new('RGB', img_size, 'white')
    draw = ImageDraw.Draw(img)
    
    for x, y in points:
        # Draw a small black circle for each point
        draw.ellipse((x-1, y-1, x+1, y+1), fill='black')
        
    img.save(output_path)
    print(f"Visualization saved to '{output_path}'")

# --- Main Execution ---
if __name__ == "__main__":
    # --- Configuration ---
    # Font URL for a nice pixelated look
    FONT_URL = "https://github.com/google/fonts/raw/main/ofl/pressstart2p/PressStart2P-Regular.ttf"
    FONT_PATH = download_font(FONT_URL)

    if FONT_PATH:
        # Loop to update the clock every second
        # For this example, we'll just run it once. 
        # You could wrap this in a `while True:` loop.
        
        current_time_str = datetime.datetime.now().strftime("%S")
        print(f"\n--- Generating for time: {current_time_str} ---")
        
        # 1. Generate the source image of the time
        time_image_path = create_time_image(current_time_str, FONT_PATH)
        
        # 2. Distribute points based on the new image
        # These numbers can be tweaked for different visual effects
        NUM_POINTS = 100
        MIN_SPACING = 7
        
        distributed_points, image_size = distribute_points_on_image(
            image_path=time_image_path,
            num_points=NUM_POINTS,
            min_distance=MIN_SPACING
        )

        # 3. Visualize the output dots
        if distributed_points:
            output_filename = f"time_dots_{current_time_str.replace(':', '-')}.png"
            visualize_result(
                img_size=image_size,
                points=distributed_points,
                output_path=output_filename
            )
        
        # 4. Clean up intermediate files
        # if os.path.exists(time_image_path):
        #     os.remove(time_image_path)


