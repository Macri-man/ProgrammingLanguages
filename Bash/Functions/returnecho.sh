add() {
    echo $(( $1 + $2 ))
}

result=$(add 3 5)
echo $result    # 8
