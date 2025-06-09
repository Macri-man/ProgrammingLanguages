package main

import "fmt"

type person interface {
	name() string
	age() int
}

type friend struct {
	friendName string
	ageNumber  int
}

type enemy struct {
	enemyName string
	ageNumber int
}

func (f friend) name() string {
	return "friendly " + f.friendName
}

func (f friend) age() int {
	return 15 + f.ageNumber
}

func (e enemy) name() string {
	return "Antagonist " + e.enemyName
}

func (e enemy) age() int {
	return 20 + e.ageNumber
}

func getStuff(p person) {
	fmt.Println(p)
	fmt.Println(p.name())
	fmt.Println(p.age())
}

func main() {

	f := friend{"Goerge", 15}
	e := enemy{"John", 5}

	getStuff(f)
	getStuff(e)
}
