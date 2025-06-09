package main

import "fmt"

func main() {
	// Declare and initialize an array
	var arr [4]int
	arr[0] = 10
	arr[1] = 20
	arr[2] = 30
	arr[3] = 40
	fmt.Println("Array:", arr) // Output: Array: [10 20 30 40]

	// Initialize an array with values
	arr2 := [3]string{"apple", "banana", "cherry"}
	fmt.Println("Initialized Array:", arr2) // Output: Initialized Array: [apple banana cherry]

	// Multi-dimensional array
	matrix := [2][3]int{
		{1, 2, 3},
		{4, 5, 6},
	}
	fmt.Println("Multi-dimensional Array:", matrix)
	// Output: Multi-dimensional Array: [[1 2 3] [4 5 6]]

	// Iterate over an array
	for i, v := range arr2 {
		fmt.Printf("Index: %d, Value: %s\n", i, v)
	}
	// Output:
	// Index: 0, Value: apple
	// Index: 1, Value: banana
	// Index: 2, Value: cherry
}
