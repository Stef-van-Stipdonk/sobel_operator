#!/bin/bash

IMAGE_DIR="./grayscaled_images"
OUTPUT_DIR="./sobel_image"

mkdir -p "$OUTPUT_DIR"

for img in "$IMAGE_DIR"/*.pgm
do
    if [ -f "$img" ]; then
        filename=$(basename "$img")
        base="sobel_${filename%.pgm}"
        echo "Processing $img"
        ./a.out "$img" > "$OUTPUT_DIR/${base}.pgm"
    else
        echo "No .pgm files found in $IMAGE_DIR"
    fi
done
