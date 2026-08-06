from pathlib import Path
import subprocess
import sys


def install_dependencies() -> None:
    """Detects and installs missing system and Python dependencies automatically."""
    print("Checking system environment and dependencies...")

    # 1. Handle Linux System dependencies if running on GitHub Actions / Ubuntu
    # CairoSVG requires libcairo2 binaries present on the OS layer.
    if sys.platform.startswith("linux"):
        try:
            # Check if cairo library is missing
            subprocess.run(
                ["ldconfig", "-p"],
                capture_output=True,
                text=True,
                check=True,
            )
            # If not found or in CI, safely attempt installing system binaries
            print("Ensuring Cairo system libraries are present via apt...")
            subprocess.run(
                [
                    "sudo",
                    "apt-get",
                    "update",
                    "-y",
                ],
                capture_output=True,
            )
            subprocess.run(
                [
                    "sudo",
                    "apt-get",
                    "install",
                    "-y",
                    "libcairo2",
                    "libffi-dev",
                ],
                check=True,
            )
        except Exception as e:
            print(
                f"Note: System-level Cairo dependency check skipped or failed: {e}"
            )

    # 2. Check and install Python Packages
    required_packages = {
        "cairosvg": "cairosvg",
        "PIL": "pillow",  # Import name is PIL, pip package name is pillow
    }

    for import_name, pip_name in required_packages.items():
        try:
            __import__(import_name)
        except ImportError:
            print(f"Missing Python dependency: '{pip_name}'. Installing...")
            subprocess.run(
                [
                    sys.executable,
                    "-m",
                    "pip",
                    "install",
                    pip_name,
                ],
                check=True,
            )
            print(f"Successfully installed {pip_name}.")


# Trigger immediate self-installation before importing libraries that might crash
install_dependencies()

# safely import modules now that installation guarantees their existence
import cairosvg
from PIL import Image

# Structural Directory Setup
ROOT_DIR = Path(__file__).resolve().parent.parent
SVG_ICON = ROOT_DIR / "res" / "app-icon.svg"
OUTPUT_DIR = ROOT_DIR / "res" / "generated"

ICON_SIZES =


def generate_pngs() -> None:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    print("Generating PNG files...")

    for size in ICON_SIZES:
        output_file = OUTPUT_DIR / f"app-icon-{size}.png"
        cairosvg.svg2png(
            url=str(SVG_ICON),
            write_to=str(output_file),
            output_width=size,
            output_height=size,
        )
        print(f"Generated {output_file.name}")

    print("PNG generation completed.")


def generate_ico() -> None:
    print("Creating Windows ICO...")
    ico_layers = []
    try:
        for size in ICON_SIZES:
            if size == 512:
                continue
            png_path = OUTPUT_DIR / f"app-icon-{size}.png"
            if not png_path.exists():
                raise FileNotFoundError(
                    f"Missing required layer file: {png_path}"
                )
            ico_layers.append(Image.open(png_path))

        ico_file = OUTPUT_DIR / "app-icon.ico"
        base_image = ico_layers
        base_image.save(ico_file, format="ICO", append_images=ico_layers[1:])
        print(f"Created {ico_file.name}")
    finally:
        for layer in ico_layers:
            layer.close()


def generate_installer_assets() -> None:
    print("Generating installer graphics...")
    source_png = OUTPUT_DIR / "app-icon-512.png"

    if not source_png.exists():
        raise FileNotFoundError(f"Missing source image: {source_png}")

    image = Image.open(source_png)
    banner = image.resize((493, 58), Image.Resampling.LANCZOS)
    banner.save(OUTPUT_DIR / "installer-banner.bmp", format="BMP")

    dialog = image.resize((493, 312), Image.Resampling.LANCZOS)
    dialog.save(OUTPUT_DIR / "installer-dialog.bmp", format="BMP")

    banner.close()
    dialog.close()
    image.close()
    print("Installer graphics generated.")


def verify_output() -> None:
    for size in ICON_SIZES:
        png_file = OUTPUT_DIR / f"app-icon-{size}.png"
        if not png_file.exists():
            raise FileNotFoundError(f"PNG generation failed: {png_file}")

    required_files = [
        OUTPUT_DIR / "app-icon.ico",
        OUTPUT_DIR / "installer-banner.bmp",
        OUTPUT_DIR / "installer-dialog.bmp",
    ]

    for file in required_files:
        if not file.exists():
            raise FileNotFoundError(f"Missing generated file: {file}")

    print("Verification completed successfully.")


def main() -> int:
    try:
        if not SVG_ICON.exists():
            raise FileNotFoundError(f"SVG icon not found: {SVG_ICON}")

        generate_pngs()
        generate_ico()
        generate_installer_assets()
        verify_output()
        print("Icon generation completed successfully.")
        return 0
    except Exception as ex:
        print(f"ERROR: {ex}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
