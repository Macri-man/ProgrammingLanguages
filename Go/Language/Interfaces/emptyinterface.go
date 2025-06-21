package main

import "fmt"

func printAnything(i interface{}) {
    fmt.Println(i)
}

func main() {
    printAnything(42)
    printAnything("hello")
    printAnything([]int{1, 2, 3})
}