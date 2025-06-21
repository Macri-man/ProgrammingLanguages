package main

import "fmt"

// Function with named return values
func swap(a, b int) (x, y int) {
	x = b
	y = a
	return // Named return values are returned automatically
}

func main() {
	a, b := 1, 2
	x, y := swap(a, b)
	fmt.Println("x:", x, "y:", y) // Output: x: 2 y: 1
}
