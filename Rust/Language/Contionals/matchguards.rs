fn main() {
    let number = 7;

    match number {
        n if n % 2 == 0 => println!("Even number: {}", n),
        n if n % 2 != 0 => println!("Odd number: {}", n),
        _ => println!("Not a number"),
    }
}