#!/bin/sh
PYTHONDONTWRITEBYTECODE=1
Blender="$1"
InputFile="$2"
OutputFile="$3"
BlenderOptions='--background -noaudio -noglsl -nojoystick'
Script="$(dirname "$0")/ExportJSON.py"
"$Blender" "$InputFile" $BlenderOptions --python "$Script" -- "$OutputFile" >/dev/null
