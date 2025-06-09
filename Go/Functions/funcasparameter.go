package main

import "fmt"

func applyFunction(a, b int, function func(int, int) int) int {
	return function(a, b)
}

func add(a, b int) int {
	return a + b
}

func subtract(a, b int) int {
	return a - b
}

func main() {

	resultadd := applyFunction(5, 3, add)
	fmt.Println("Result of addition:", resultadd) // Output: Result of addition: 8

	resultsubtract := applyFunction(5, 3, subtract)
	fmt.Println("Result of addition:", resultsubtract) // Output: Result of addition: 8

}
