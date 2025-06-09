fn main() {
    let some_value: Option<i32> = Some(10);

    match some_value {
        Some(val) => println!("Value is: {}", val),
        None => println!("No value"),
    }
}