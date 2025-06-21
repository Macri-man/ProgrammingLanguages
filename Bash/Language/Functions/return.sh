is_even() {
    if (( $1 % 2 == 0 )); then
        return 0    # success: even
    else
        return 1    # failure: odd
    fi
}

is_even 4
echo $?    # 0 (true)

is_even 5
echo $?    # 1 (false)
