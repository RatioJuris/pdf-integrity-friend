from pathlib import Path

from PIL import Image
import cairosvg

ROOT = Path(__file__).resolve().parent.parent

SVG_FILE = ROOT / "res" / "app-icon.svg"
OUTPUT_DIR = ROOT / "res" / "generated"

OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

sizes = [
    16,
    24,
    32,
    48,
    64,
    128,
    256,
    512
]

png_files = []

for size in sizes:
    png_path = OUTPUT_DIR / f"app-icon-{size}.png"

    cairosvg.svg2png(
        url=str(SVG_FILE),
        write_to=str(png_path),
        output_width=size,
        output_height=size
    )

    png_files.append(png_path)

print("PNG icon generation completed.")

ico_sizes = []

for size in sizes:
    img = Image.open(
        OUTPUT_DIR / f"app-icon-{size}.png"
    )

    ico_sizes.append(img)

ico_path = OUTPUT_DIR / "app-icon.ico"

ico_sizes[0].save(
    ico_path,
    format="ICO",
    sizes=[(s, s) for s in sizes]
)

print("ICO generation completed.")
print(f"Output: {ico_path}")
