fn main() {
    let mut number = 1;

    while number < 100 {
        if number % 15 == 0 {
            number += 13;
            println!("{}", number);
        } else if number % 8 == 0 {
            number += 7;
            println!("{}", number);
        } else if number % 5 == 0 {
            number += 6;
            println!("{}", number);
        } else {
            println!("{}", number);
        }
        number += 1;
    }
}
