#!/bin/sh
PYTHONDONTWRITEBYTECODE=1
cd "$(dirname "$0")"
./GeneratePlanetSurface.py $@
