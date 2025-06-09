package main

import (
	"fmt"
	"strings"
)

type Function interface {
	Call(args ...interface{}) interface{}
}

type FunctionAdapter struct {
	fn interface{}
}

func (f FunctionAdapter) Call(args ...interface{}) interface{} {
	switch fn := f.fn.(type) {
	case func(int, int) int:
		return fn(args[0].(int), args[1].(int))
	case func(string) string:
		return fn(args[0].(string))
	case func(string) int:
		return fn(args[0].(string))
	case func(string) (int, int):
		return fn(args[0].(string))
	default:
		panic("Unsupported function type")
	}
}

func main() {
	functionMap := map[string]Function{
		"add":         FunctionAdapter{func(a, b int) int { return a + b }},
		"greet":       FunctionAdapter{func(name string) string { return "Hello, " + name }},
		"uppercase":   FunctionAdapter{strings.ToUpper},
		"countChars":  FunctionAdapter{func(s string) int { return len(s) }},
		"multireturn": FunctionAdapter{func(s string) (int, int) { return len(s), len(s) + 2 }},
	}

	fmt.Println(functionMap["add"].Call(5, 3))
	fmt.Println(functionMap["greet"].Call("Alice"))
	fmt.Println(functionMap["uppercase"].Call("golang"))
	fmt.Println(functionMap["countChars"].Call("Hello, World!"))
	fmt.Println(functionMap["multireturn"].Call("Hello, World!"))
}
