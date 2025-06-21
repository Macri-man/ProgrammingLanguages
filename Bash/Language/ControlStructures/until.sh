count=1
until [ $count -gt 5 ]; do
    echo "Count is $count"
    ((count++))
done
