from PIL import Image, ImageDraw

# Create 128x96 image (4x3 grid of 32x32 sprites)
img = Image.new('RGBA', (128, 96), (0, 0, 0, 0))
draw = ImageDraw.Draw(img)

colors = [
    (255, 0, 0),    # Red
    (0, 255, 0),    # Green
    (0, 0, 255),    # Blue
    (255, 255, 0),  # Yellow
    (255, 0, 255),  # Magenta
    (0, 255, 255),  # Cyan
    (255, 128, 0),  # Orange
    (128, 0, 255),  # Purple
    (255, 255, 255),# White
    (128, 128, 128),# Gray
    (64, 64, 64),   # Dark gray
    (192, 192, 192) # Light gray
]

# Draw 32x32 colored squares
for i in range(12):
    x = (i % 4) * 32
    y = (i // 4) * 32
    draw.rectangle([x, y, x+31, y+31], fill=colors[i], outline=(0,0,0))
    # Draw frame number
    draw.text((x+12, y+12), str(i), fill=(0,0,0))

img.save('assets/test_data/simple_atlas.png')
print("Created simple_atlas.png")