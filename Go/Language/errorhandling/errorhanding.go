package main

import (
    "fmt"
    "errors"
)

func doSomething(flag bool) error {
    if !flag {
        return errors.New("something went wrong")
    }
    return nil
}

func main() {
    if err := doSomething(false); err != nil {
        fmt.Println("Error:", err)
    } else {
        fmt.Println("Operation successful")
    }
}