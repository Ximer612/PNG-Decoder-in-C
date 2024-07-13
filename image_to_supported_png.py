from PIL import Image
from os.path import exists


img_name = "image.png"
img_path_input = "resources/"+img_name
img_path_output = "resources/converted_"+img_name

# Open image
file_exists = exists(img_path_input)

if not file_exists:
    raise Exception("INPUT FILE DOESN'T EXIST!")

img = Image.open(img_path_input)

# Converts image in RGBA (Truecolor with alfa channel)
img = img.convert('RGBA')

# Saves converted image
img.save(img_path_output)
