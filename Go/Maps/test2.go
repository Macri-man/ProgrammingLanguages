package main

import (
	"fmt"
	"strings"
)

func main() {
	functionMap := map[string]interface{}{
		"add":        func(a, b int) int { return a + b },
		"greet":      func(name string) string { return "Hello, " + name },
		"uppercase":  strings.ToUpper,
		"countChars": func(s string) int { return len(s) },
	}

	// Using the functions
	if addFunc, ok := functionMap["add"].(func(int, int) int); ok {
		fmt.Println(addFunc(5, 3))
	}

	if greetFunc, ok := functionMap["greet"].(func(string) string); ok {
		fmt.Println(greetFunc("Alice"))
	}

	if uppercaseFunc, ok := functionMap["uppercase"].(func(string) string); ok {
		fmt.Println(uppercaseFunc("golang"))
	}

	if countCharsFunc, ok := functionMap["countChars"].(func(string) int); ok {
		fmt.Println(countCharsFunc("Hello, World!"))
	}

	fmt.Println(functionMap["add"].(func(int, int) int)(5, 3))
}
