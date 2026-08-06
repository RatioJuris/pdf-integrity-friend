from pathlib import Path
import subprocess
import sys


def install_dependencies() -> None:
    """Detects and installs missing Python dependencies automatically."""
    print("Validating build dependencies...")

    required_packages = {
        "PIL": "pillow",
        "PySide6": "PySide6",  # Replaced svglib/reportlab with Qt's robust SVG engine
    }

    for import_name, pip_name in required_packages.items():
        try:
            __import__(import_name)
        except ImportError:
            print(f"Package '{pip_name}' is missing. Launching pip installer...")
            subprocess.run(
                [sys.executable, "-m", "pip", "install", pip_name], check=True
            )
            print(f"Successfully configured {pip_name}.")


# Initialize package validations
install_dependencies()

from PIL import Image
from PySide6.QtCore import QSize, Qt
from PySide6.QtGui import QColor, QPainter, QPixmap
from PySide6.QtSvg import QSvgRenderer

# Directory Mappings
ROOT_DIR = Path(__file__).resolve().parent.parent
SVG_ICON = ROOT_DIR / "res" / "app-icon.svg"
OUTPUT_DIR = ROOT_DIR / "res" / "generated"

ICON_SIZES = [16, 24, 32, 48, 64, 128, 256, 512]


def generate_pngs() -> None:
    """Translates SVG to structural multi-tier PNG dimensions using Qt engine."""
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    print("Compiling asset vectors to PNG formats using Qt SVG engine...")

    # Load SVG file via Qt's renderer
    renderer = QSvgRenderer(str(SVG_ICON))
    if not renderer.isValid():
        raise ValueError(f"Could not parse or validate SVG schema: {SVG_ICON}")

    for size in ICON_SIZES:
        output_file = OUTPUT_DIR / f"app-icon-{size}.png"

        # Create transparent canvas backing
        pixmap = QPixmap(QSize(size, size))
        pixmap.fill(QColor(Qt.GlobalColor.transparent))

        # Paint SVG uniformly on top of the target size canvas
        painter = QPainter(pixmap)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        painter.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform)
        renderer.render(painter)
        painter.end()

        # Save output image
        if not pixmap.save(str(output_file), "PNG"):
            raise IOError(f"Failed to write image file: {output_file}")

        print(f"Generated {output_file.name}")

    print("PNG generation finished.")


def generate_ico() -> None:
    """Wraps target resolution layers into a Windows executable .ico file."""
    print("Compiling Windows ICO container...")
    ico_layers = []
    try:
        for size in ICON_SIZES:
            if size == 512:
                continue  # Windows ICO specifications max out at 256x256
            png_path = OUTPUT_DIR / f"app-icon-{size}.png"
            if not png_path.exists():
                raise FileNotFoundError(f"Missing resource layer: {png_path}")
            ico_layers.append(Image.open(png_path))

        ico_file = OUTPUT_DIR / "app-icon.ico"
        base_image = ico_layers[0]
        base_image.save(ico_file, format="ICO", append_images=ico_layers[1:])
        print(f"Created {ico_file.name}")
    finally:
        for layer in ico_layers:
            layer.close()


def generate_installer_assets() -> None:
    """Prepares standard structural windows setup installer bitmaps."""
    print("Processing installer setup screens...")
    source_png = OUTPUT_DIR / "app-icon-512.png"

    if not source_png.exists():
        raise FileNotFoundError(f"Missing baseline graphic source: {source_png}")

    image = Image.open(source_png)

    banner = image.resize((493, 58), Image.Resampling.LANCZOS)
    banner.save(OUTPUT_DIR / "installer-banner.bmp", format="BMP")

    dialog = image.resize((493, 312), Image.Resampling.LANCZOS)
    dialog.save(OUTPUT_DIR / "installer-dialog.bmp", format="BMP")

    banner.close()
    dialog.close()
    image.close()
    print("Setup assets created successfully.")


def verify_output() -> None:
    """Confirms all structural build assets passed processing checks."""
    for size in ICON_SIZES:
        png_file = OUTPUT_DIR / f"app-icon-{size}.png"
        if not png_file.exists():
            raise FileNotFoundError(f"Output verification failed: {png_file}")

    required_files = [
        OUTPUT_DIR / "app-icon.ico",
        OUTPUT_DIR / "installer-banner.bmp",
        OUTPUT_DIR / "installer-dialog.bmp",
    ]

    for file in required_files:
        if not file.exists():
            raise FileNotFoundError(f"Verification tracking missing: {file}")

    print("Pipeline compilation verify check passed.")


def main() -> int:
    try:
        if not SVG_ICON.exists():
            raise FileNotFoundError(f"Source file asset target missing: {SVG_ICON}")

        generate_pngs()
        generate_ico()
        generate_installer_assets()
        verify_output()

        print("Asset generation operations completed successfully.")
        return 0
    except Exception as ex:
        print(f"CRITICAL ERROR: {ex}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
