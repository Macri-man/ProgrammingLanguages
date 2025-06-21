package main

import (
	"fmt"
)

func main() {

	functionMap := map[string]func(int) int{
		"double": func(x int) int { return x * 2 },
		"square": func(x int) int { return x * x },
	}

	// Using the functions
	result := functionMap["double"](4)
	fmt.Println(result) // Outputs: 8
	result = functionMap["square"](4)
	fmt.Println(result) // Outputs: 16

	functionMap2 := map[string]func(string) string{
		"first":  func(x string) string { return "hello " + x },
		"second": func(x string) string { return "greet " + x },
	}

	// Using the functions
	result2 := functionMap2["first"]("john")
	fmt.Println(result2) // Outputs: 8
	result2 = functionMap2["second"]("david")
	fmt.Println(result2) // Outputs: 16

}
