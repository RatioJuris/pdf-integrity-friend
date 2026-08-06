const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

// 1. Auto-install dependencies internally
try {
    require('sharp');
} catch (e) {
    console.log("Installing 'sharp' engine via npm...");
    execSync('npm install sharp --no-save', { stdio: 'inherit' });
}

const sharp = require('sharp');

const ROOT_DIR = path.resolve(__dirname, '..');
const SVG_ICON = path.join(ROOT_DIR, 'res', 'app-icon.svg');
const OUTPUT_DIR = path.join(ROOT_DIR, 'res', 'generated');
const ICON_SIZES = [16, 24, 32, 48, 64, 128, 256, 512];

async function main() {
    if (!fs.existsSync(OUTPUT_DIR)) {
        fs.mkdirSync(OUTPUT_DIR, { recursive: true });
    }

    console.log("Compiling SVG to asset targets via Node.js Sharp...");

    // Generate all target PNG dimensions
    for (const size of ICON_SIZES) {
        const outPath = path.join(OUTPUT_DIR, `app-icon-${size}.png`);
        await sharp(SVG_ICON)
            .resize(size, size)
            .png()
            .toFile(outPath);
        console.log(`Generated app-icon-${size}.png`);
    }

    // Generate Installer Graphics (.bmp format)
    await sharp(path.join(OUTPUT_DIR, 'app-icon-512.png'))
        .resize(493, 58)
        .toFormat('bmp')
        .toFile(path.join(OUTPUT_DIR, 'installer-banner.bmp'));

    await sharp(path.join(OUTPUT_DIR, 'app-icon-512.png'))
        .resize(493, 312)
        .toFormat('bmp')
        .toFile(path.join(OUTPUT_DIR, 'installer-dialog.bmp'));

    // Generate Windows Multi-Res ICO Container 
    // (Sharp shortcuts this by converting a layered PNG array into an ICO structure)
    const icoLayers = ICON_SIZES.filter(s => s !== 512).map(s => path.join(OUTPUT_DIR, `app-icon-${s}.png`));
    await sharp(icoLayers[0])
        .toFormat('ico', { sizes: ICON_SIZES.filter(s => s !== 512) })
        .toFile(path.join(OUTPUT_DIR, 'app-icon.ico'));

    console.log("Asset compilation completed successfully.");
}

main().catch(err => {
    console.error("CRITICAL ERROR:", err);
    process.exit(1);
});
