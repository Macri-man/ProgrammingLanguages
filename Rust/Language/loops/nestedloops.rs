#![allow(unreachable_code, unused_labels)]
fn main() {
    let mut numouter = 0;
    let mut numinner = 1;

    'outer: loop {
        println!("Entered the outer loop");
        numouter += 2;
        'inner: loop {
            println!("Entered the inter loop");
            numinner += 1;

            if numinner == numouter {
                break;
            }

            if numinner > 2 {
                break 'outer;
            }
        }

        println!(
            "This statement will only be reached when {} = {}",
            numouter, numinner
        );
    }

    println!("Exited the outer loop");
}
