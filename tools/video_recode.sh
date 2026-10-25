#!/bin/sh

ffmpeg -i $1 -vf "fps=1, scale=192:96" -c:v libx264 -profile:v high -crf 20 -pix_fmt yuv420p -an $2