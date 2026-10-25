#!/bin/sh

ffmpeg -i "$1" -vf "v360=eac:equirect" "$2"
