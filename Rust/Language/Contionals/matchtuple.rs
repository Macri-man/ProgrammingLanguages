fn main() {
    let tuple = (1, 2, 3);

    match tuple {
        (1, x, y) => println!("x: {}, y: {}", x, y),
        (2, ..) => println!("Starts with 2"),
        _ => println!("Some other tuple"),
    }
}