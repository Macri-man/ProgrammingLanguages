for i in {1..10}; do
    if [ $i -eq 5 ]; then
        break   # exit loop when i == 5
    fi
    if [ $((i % 2)) -eq 0 ]; then
        continue   # skip even numbers
    fi
    echo $i
done
