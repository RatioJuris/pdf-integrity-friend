const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

// 1. Auto-install dependencies internally if missing
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
const ICON_SIZES =;

async function main() {
    if (!fs.existsSync(SVG_ICON)) {
        throw new Error(`Source SVG icon file not found at: ${SVG_ICON}`);
    }

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
    const sourcePng512 = path.join(OUTPUT_DIR, 'app-icon-512.png');

    await sharp(sourcePng512)
        .resize(493, 58)
        .toFormat('bmp')
        .toFile(path.join(OUTPUT_DIR, 'installer-banner.bmp'));
    console.log("Generated installer-banner.bmp");

    await sharp(sourcePng512)
        .resize(493, 312)
        .toFormat('bmp')
        .toFile(path.join(OUTPUT_DIR, 'installer-dialog.bmp'));
    console.log("Generated installer-dialog.bmp");

    // 🌟 FIXED: Map entire paths array into an array of Sharp input objects
    console.log("Creating Windows multi-resolution ICO...");
    const filteredSizes = ICON_SIZES.filter(s => s !== 512);
    const icoInputs = filteredSizes.map(s => ({
        input: path.join(OUTPUT_DIR, `app-icon-${s}.png`)
    }));

    // Pass the entire inputs definition block to generate a proper layered ICO
    await sharp(icoInputs)
        .toFormat('ico', { sizes: filteredSizes })
        .toFile(path.join(OUTPUT_DIR, 'app-icon.ico'));
    console.log("Created app-icon.ico");

    // Post-generation verification check
    const requiredFiles = [
        path.join(OUTPUT_DIR, 'app-icon.ico'),
        path.join(OUTPUT_DIR, 'installer-banner.bmp'),
        path.join(OUTPUT_DIR, 'installer-dialog.bmp')
    ];
    ICON_SIZES.forEach(s => requiredFiles.push(path.join(OUTPUT_DIR, `app-icon-${s}.png`)));

    for (const file of requiredFiles) {
        if (!fs.existsSync(file)) {
            throw new Error(`Verification asset missing: ${file}`);
        }
    }

    console.log("Asset compilation completed successfully.");
}

main().catch(err => {
    console.error("CRITICAL ERROR:", err.message || err);
    process.exit(1);
});
