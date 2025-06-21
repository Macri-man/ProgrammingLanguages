package main

import "fmt"

func main() {

	first := []int{1, 2, 3}
	fmt.Println(first)
	second := [4]int{1, 2, 3}
	fmt.Println(second)

	stuff := [5]string{"table", "chair", "cloth", "toy", "joy"}

	fmt.Println(stuff)

	a := stuff[0:3]
	b := stuff[1:5]
	fmt.Println(a, b)

	b[0] = "Noone"
	fmt.Println(a, b)
	fmt.Println(stuff)

	s := []int{2, 5, 6}
	fmt.Printf("len=%d cap=%d %v\n", len(s), cap(s), s)

	s = s[1:]
	fmt.Printf("len=%d cap=%d %v\n", len(s), cap(s), s)

	s = s[:2]
	fmt.Printf("len=%d cap=%d %v\n", len(s), cap(s), s)

	c := make([]int, 3)
	fmt.Printf("len=%d cap=%d %v\n", len(c), cap(c), c)

	copy(c, s)
	fmt.Printf("len=%d cap=%d %v\n", len(c), cap(c), c)

	c = append(c, 3, 4, 5)
	fmt.Printf("len=%d cap=%d %v\n", len(c), cap(c), c)

}
