macro_rules! calculate {
    ($x:expr + $y:expr) => {
        $x + $y
    };
    ($x:expr - $y:expr) => {
        $x - $y
    };
}

fn main() {
    let sum = calculate!(5 + 3);
    let difference = calculate!(10 - 4);
    
    println!("Sum: {}", sum); // Prints: Sum: 8
    println!("Difference: {}", difference); // Prints: Difference: 6
}