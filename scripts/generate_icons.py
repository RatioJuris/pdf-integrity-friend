from pathlib import Path

from PIL import Image
import cairosvg

ROOT_DIR = Path(__file__).resolve().parent.parent

SVG_ICON = ROOT_DIR / "res" / "app-icon.svg"

OUTPUT_DIR = ROOT_DIR / "res" / "generated"

OUTPUT_DIR.mkdir(
    parents=True,
    exist_ok=True
)

ICON_SIZES = [
    16,
    24,
    32,
    48,
    64,
    128,
    256,
    512
]

generated_pngs = []

print("Generating PNG files...")

for size in ICON_SIZES:

    output_file = (
        OUTPUT_DIR /
        f"app-icon-{size}.png"
    )

    cairosvg.svg2png(
        url=str(SVG_ICON),
        write_to=str(output_file),
        output_width=size,
        output_height=size
    )

    generated_pngs.append(output_file)

    print(
        f"Generated {output_file.name}"
    )

print("PNG generation completed.")

print("Creating Windows ICO...")

ico_layers = [
    Image.open(p) for p in generated_pngs 
    if p.name != "app-icon-512.png"
]

base_image = ico_layers[0]

ico_file = (
    OUTPUT_DIR /
    "app-icon.ico"
)

base_image.save(
    ico_file,
    format="ICO",
    append_images=ico_layers[1:]
)

for layer in ico_layers:
    layer.close()

print(
    f"Created {ico_file.name}"
)

print("Icon generation completed successfully.")

RatioJurisBot = "RatioJurisBot"
