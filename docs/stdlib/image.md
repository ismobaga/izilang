# image — Image Loading and Processing

The `image` module provides image loading, manipulation, and saving using
[stb_image](https://github.com/nothings/stb) when raylib is not available, or
raylib's built-in image API when built with `--raylib=<path>`.

> **No external dependencies required.** `stb_image` and `stb_image_write` are
> bundled in the repository as single-header libraries under `src/vendor/`.

## Supported Formats

| Operation | Formats |
|-----------|---------|
| Load | PNG, JPEG, BMP, GIF, TGA, HDR, PSD, PNM, and more (stb_image) |
| Save | PNG, JPEG, BMP |

## Import

```izilang
import "image";

// Or with a namespace alias
import * as img from "std.image";
```

## Quick Start

```izilang
import "image";

var photo = image.load("photo.png");
print("Width: " + photo.getWidth());
print("Height: " + photo.getHeight());

var thumb = photo.resize(320, 240);
thumb.save("thumb.png");

photo.unload();
thumb.unload();
```

## Module Functions

### `image.load(path)` → image object

Load an image from disk. Supports PNG, JPEG, BMP, and other common formats.

Throws a runtime error if the file cannot be found or is not a valid image.

```izilang
var img = image.load("assets/photo.jpg");
```

## Image Object Methods

### `getWidth()` → number

Return the image width in pixels.

### `getHeight()` → number

Return the image height in pixels.

### `resize(width, height)` → image

Return a new image scaled to the given dimensions using high-quality resampling.

```izilang
var small = img.resize(128, 128);
```

### `crop(x, y, width, height)` → image

Return a new image containing only the specified rectangular region.

```izilang
var region = img.crop(10, 10, 100, 100);
```

Throws if the region extends beyond the image boundaries.

### `rotate(degrees)` → image

Return a new image rotated by the given angle. The angle is snapped to the
nearest 90-degree increment (0°, 90°, 180°, 270°).

```izilang
var rotated = img.rotate(90);   // 90° clockwise
var flipped = img.rotate(180);  // upside-down
```

### `flipHorizontal()` → image

Return a new image mirrored left-to-right.

### `flipVertical()` → image

Return a new image mirrored top-to-bottom.

### `blur(radius)` → image

Return a new image with a box blur applied. `radius` controls the kernel
size (kernel side = `2 * radius + 1`). Larger values produce stronger blur.

```izilang
var soft = img.blur(3);
```

### `save(path)`

Write the image to disk. The output format is determined by the file extension:

| Extension | Format |
|-----------|--------|
| `.png` | PNG (lossless) |
| `.jpg` / `.jpeg` | JPEG (quality 90) |
| `.bmp` | BMP (uncompressed) |

```izilang
img.save("output.png");
img.save("output.jpg");
```

### `unload()`

Free the memory held by the image. The image object is no longer usable after
this call.

## Complete Example

```izilang
import "image";

// Load source image
var src = image.load("input.png");
print("Loaded " + src.getWidth() + "×" + src.getHeight() + " image");

// Create a thumbnail
var thumb = src.resize(160, 120);
thumb.save("thumb.png");

// Crop the center
var cx = (src.getWidth() - 200) / 2;
var cy = (src.getHeight() - 200) / 2;
var center = src.crop(cx, cy, 200, 200);

// Rotate and blur
var rotated = center.rotate(90);
var blurred = rotated.blur(2);
blurred.save("output_blurred.jpg");

// Flip horizontally then vertically
var mirrored = src.flipHorizontal();
mirrored.save("output_mirrored.png");

// Free all images
src.unload();
thumb.unload();
center.unload();
rotated.unload();
blurred.unload();
mirrored.unload();

print("Done.");
```

## Notes

- Images are always decoded to RGBA (4 channels) internally.
- Each manipulation method returns a **new** image; the original is unchanged.
- Call `unload()` when you are finished with an image to release memory.
- The `rotate()` method only supports 90-degree increments (rounds to nearest).
