use std::process::Command;
use std::env;
use std::fs;


fn main() {
    println!("cargo::rerun-if-changed=implant/");

    env::set_current_dir("./implant/").unwrap();

    Command::new("make")
        .output()
        .expect("failed to execute process");

    env::set_current_dir("..").unwrap();

    let shc: Vec<u8> = fs::read("implant/shc.bin").unwrap();

    let rust_code = format!("pub const KEYDUMP_SHC: &[u8] = &{:?};", shc);

    fs::write("src/shc.rs", rust_code).expect("Error writing shellcode");
}

