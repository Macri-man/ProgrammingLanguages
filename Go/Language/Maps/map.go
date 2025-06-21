package main

import "fmt"

func main(){
	DeclareMap1 := make(map[string]int)

	DeclareMap1["Key1"] = 1
	DeclareMap1["key2"] = 2
	
	fmt.Println("map": DeclareMap1)

	value1 := DeclareMap1["key1"]
	fmt.Println("value1:",value1)

	value3 := DeclareMap1["key3"]
	fmt.Println("value3:",value3)

	fmt.Println("len:",len(DeclareMap1))

	delete(DeclareMap1,"key2")

	key2, value2 := m["key2"]
	fmt.Println("key2:", key2)
  fmt.Println("value2:", value2)

	key1, value1 := m["key1"]
	fmt.Println("key1:", key1)
  fmt.Println("value1:", value1)

	clear(DeclareMap1)
  fmt.Println("map:", DeclareMap1)

	DeclareMap2 := map[string]int{"Value1": 1, "Value2": 2}
  fmt.Println("map:", DeclareMap2)

	DeclareMap3 := map[string]int{"Value1": 1, "Value2": 3}
  if maps.Equal(DeclareMap2, DeclareMap3) {
    fmt.Println("DeclareMap2 equals DeclareMap3")
  }else{
		fmt.Println("DeclareMap2 not equal DeclareMap3")
	}

}