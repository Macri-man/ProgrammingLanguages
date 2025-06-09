package main

import (
	"fmt"
	"strings"
)

// Define a type for our variadic function
type VariadicFunc func(...interface{}) []interface{}

func main() {
	// Create a map of variadic functions
	functionMap := map[string]VariadicFunc{
		"add": func(args ...interface{}) []interface{} {
			sum := 0
			for _, arg := range args {
				if num, ok := arg.(int); ok {
					sum += num
				}
			}
			return []interface{}{sum}
		},
		"greet": func(args ...interface{}) []interface{} {
			if len(args) > 0 {
				if name, ok := args[0].(string); ok {
					return []interface{}{"Hello, " + name}
				}
			}
			return []interface{}{"Hello, World"}
		},
		"uppercase": func(args ...interface{}) []interface{} {
			results := make([]interface{}, len(args))
			for i, arg := range args {
				if str, ok := arg.(string); ok {
					results[i] = strings.ToUpper(str)
				} else {
					results[i] = arg
				}
			}
			return results
		},
		"countChars": func(args ...interface{}) []interface{} {
			results := make([]interface{}, len(args))
			for i, arg := range args {
				if str, ok := arg.(string); ok {
					results[i] = len(str)
				} else {
					results[i] = 0
				}
			}
			return results
		},
	}

	// Using the functions
	fmt.Println(functionMap["add"](5, 3, 2))
	fmt.Println(functionMap["greet"]("Alice"))
	fmt.Println(functionMap["uppercase"]("golang", "python"))
	fmt.Println(functionMap["countChars"]("Hello, World!", "OpenAI"))

	// Using multiple arguments
	fmt.Println(functionMap["add"](1, 2, 3, 4, 5))
	fmt.Println(functionMap["uppercase"]("go", "rust", "java"))
}
