fn main() {
    let number = 7;

    match number {
        1 | 2 => println!("One or Two"),
        3 | 4 => println!("Three or Four"),
        _ => println!("Other number"),
    }
}
