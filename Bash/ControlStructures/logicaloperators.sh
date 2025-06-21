if [ -f "file.txt" ] && [ -r "file.txt" ]; then
    echo "File exists and is readable."
fi

if [[ -f "file.txt" && -r "file.txt" ]]; then
    echo "File exists and is readable."
fi
