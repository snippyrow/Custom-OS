from PIL import Image
import numpy as np

def compress_jpg_to_pixel_bin(input_jpg_path, output_bin_path):
    # Open the JPG image
    img = Image.open(input_jpg_path)

    # Convert to P mode (256 colors)
    img = img.convert('P', palette=Image.ADAPTIVE, colors=16)

    # Load the image as an array of pixel data
    img_array = np.array(img)

    # Flatten the array and convert to bytes
    img_bytes = (img_array.flatten().tobytes())

    # Write the bytes to a binary file
    with open(output_bin_path, 'wb') as bin_file:
        bin_file.write(img_bytes)

    print(f"Compressed pixel data saved in {output_bin_path}")

# Example usage
compress_jpg_to_pixel_bin('Assets/font_n.png', 'Assets/font_n.bin')
