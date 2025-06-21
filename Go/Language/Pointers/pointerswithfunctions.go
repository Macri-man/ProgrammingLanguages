package main

import "fmt"

func increment(x *int) {
    *x++ // Increment the value at the address x points to
}

func main() {
    a := 5
    increment(&a)
    fmt.Println("Value of a after increment:", a) // Output: Value of a after increment: 6
}