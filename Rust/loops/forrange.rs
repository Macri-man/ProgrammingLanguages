fn main() {
    for number in 1..10 {
        println!("{}", number);
    }

    for number in 1..=10 {
        println!("{}", number);
    }

    let words = vec!["Apples", "Trees", "Birds", "Chairs"];

    for word in words.iter() {
        println!("{}", word);
    }

    println!("words: {:?}", words);

    let mut words = vec!["Oranges", "Jazz", "Donuts", "Doorbell"];

    for word in words.iter_mut() {
        println!("{}", word);
    }

    println!("words: {:?}", words);

    let words = vec!["Jump", "Horse", "Mountain", "Peaches"];

    for word in words.clone().into_iter() {
        println!("{}", word);
    }

    println!("words: {:?}", words);

    let words = vec!["Hill", "House", "Giant", "Snake"];

    for word in words.into_iter() {
        println!("{}", word);
    }

    //println!("words: {:?}", words);
}
