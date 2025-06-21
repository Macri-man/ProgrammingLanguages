fn main() {
    let mut number = 0;
    let end = 5;

    loop {
        number += 1;

        if number % 2 == 0 {
            println!("Even");
            continue;
        }

        println!("{}", number);

        if number == end {
            println!("End");
            break;
        }
    }
}
