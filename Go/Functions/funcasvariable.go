package main

import "fmt"

// Function type alias
type Operation func(int, int) int

func add(a, b int) int {
	return a + b
}

func subtract(a, b int) int {
	return a - b
}

func main() {
	var op Operation

	op = add
	fmt.Println("Add:", op(5, 3)) // Output: Add: 8

	op = subtract
	fmt.Println("Subtract:", op(5, 3)) // Output: Subtract: 2
}
