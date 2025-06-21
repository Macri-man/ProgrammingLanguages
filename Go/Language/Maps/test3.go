package main

import (
	"fmt"
	"reflect"
	"strings"
)

type Function interface {
	Call(args ...interface{}) interface{}
}

type FunctionAdapter struct {
	fn interface{}
}

func (f FunctionAdapter) Call(args ...interface{}) interface{} {
	fnValue := reflect.ValueOf(f.fn)
	numIn := fnValue.Type().NumIn()
	numOut := fnValue.Type().NumOut()

	// Prepare the input arguments
	in := make([]reflect.Value, numIn)
	for i := 0; i < numIn; i++ {
		in[i] = reflect.ValueOf(args[i])
	}

	// Call the function
	out := fnValue.Call(in)

	//if there is only one return value
	if numOut == 1 {
		return out[0].Interface()
	}

	// Convert the output to []interface{}
	result := make([]interface{}, numOut)
	for i, v := range out {
		result[i] = v.Interface()
	}
	return result
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

	multiReturnResult := functionMap["multireturn"].Call("Hello, World!").([]interface{})
	fmt.Printf("Multi-return result: %v, %v\n", multiReturnResult[0], multiReturnResult[1])
}
