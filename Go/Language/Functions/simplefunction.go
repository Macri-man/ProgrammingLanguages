package main

import "fmt"

// Function that adds two integers
func add(a int, b int) int {
    return a + b
}

func main() {
    result := add(3, 4)
    fmt.Println("Sum:", result) // Output: Sum: 7
}