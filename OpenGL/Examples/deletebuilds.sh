#!/bin/bash

# Recursively find and delete all "build" directories under current directory
find . -type d -name "build" -exec rm -rf {} +

echo "All 'build' folders have been deleted."
