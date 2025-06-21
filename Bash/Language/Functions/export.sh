my_func() {
    echo "Hello from exported function"
}

export -f my_func

bash -c 'my_func'  # runs the function in a child shell
