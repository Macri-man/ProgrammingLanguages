fn main() {
    let mut number = 1;

    let result = loop {
        number *= 2;

        if number % 2 == 0 {
            break number * 3;
        }
    };

    println!("{}", result);
}
