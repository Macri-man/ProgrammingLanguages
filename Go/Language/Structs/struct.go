package main

import "fmt"

type Pet struct {
	name  string
	age   int
	owner string
}

func newPet(name, owner string) Pet {
	p := Pet{name: name, owner: owner}
	p.age = 20
	return p
}

func (p *Pet) changePet(name string) {
	p.name = name
}

func main() {
	fmt.Println(newPet("Stacy", "John"))
	fmt.Println(Pet{"George", 22, "Fred"})
	fmt.Println(Pet{name: "Jim"})

	stuff := struct {
		something string
		isgood    bool
	}{
		"Hello",
		true,
	}

	fmt.Println(stuff)

	newpets := Pet{"George", 22, "Fred"}
	fmt.Println(newpets)
	newpets.changePet("Hello")
	fmt.Println(newpets)
	newpets.name = "George"
	fmt.Println(newpets)

}
