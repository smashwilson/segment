extern crate pest;
#[macro_use]
extern crate pest_derive;

use std::env;
use std::fs;

mod parser;

fn main() {
    for source_path in env::args().skip(1) {
        let content = fs::read_to_string(&source_path).expect("Unable to read file");
        parser::parse(&content);
    }
}
