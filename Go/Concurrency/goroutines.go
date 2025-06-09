package main

import (
    "fmt"
    "time"
)

func say(message string) {
    for i := 0; i < 5; i++ {
        time.Sleep(time.Millisecond * 500)
        fmt.Println(message)
    }
}

func main() {
    go say("Hello")
    go say("World")

    // Wait for goroutines to finish
    time.Sleep(time.Second * 3)
}