package main

import "fmt"

func main() {
	// Declare and initialize a slice
	s := []int{1, 2, 3, 4, 5}
	fmt.Println("Original slice:", s)

	// Access elements
	fmt.Println("First element:", s[0])
	fmt.Println("Third element:", s[2])

	// Slice a slice
	subSlice := s[1:4]
	fmt.Println("Sub-slice:", subSlice) // Output: [2, 3, 4]

	// Append to a slice
	s = append(s, 6, 7)
	fmt.Println("After appending:", s)

	// Append another slice
	s2 := []int{8, 9}
	s = append(s, s2...)
	fmt.Println("After appending another slice:", s)

	// Copy a slice
	copySlice := make([]int, len(s))
	copy(copySlice, s)
	fmt.Println("Copied slice:", copySlice)

	// Length and capacity
	fmt.Println("Length of s:", len(s))
	fmt.Println("Capacity of s:", cap(s))
}
