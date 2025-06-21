package main

import "fmt"

type Animal interface {
	Speak() string
}
type Dog struct{}

func (d Dog) Speak() string {
	return "Woof!"
}

type Cat struct{}

func (c Cat) Speak() string {
	return "Meow!"
}
func makeItSpeak(a Animal) {
	fmt.Println(a.Speak())
}

func main() {
	d := Dog{}
	c := Cat{}

	makeItSpeak(d) // Outputs: Woof!
	makeItSpeak(c) // Outputs: Meow!
}
