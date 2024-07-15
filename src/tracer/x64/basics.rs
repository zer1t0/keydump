use super::{register, TraceError};
use errno::Errno;
use std::io::{Read, SeekFrom, Write};
use std::{fs::File, io::Seek};


pub fn read_memory_x64(
    pid: i32,
    start_addr: u64,
    size: usize,
) -> Result<Vec<u8>, TraceError> {
    let filepath = format!("/proc/{}/mem", pid);

    let mut f = File::open(filepath).map_err(|e| {
        TraceError::ReadMemoryError(pid, Errno(e.raw_os_error().unwrap_or(0)))
    })?;

    f.seek(SeekFrom::Start(start_addr)).map_err(|e| {
        TraceError::ReadMemoryError(pid, Errno(e.raw_os_error().unwrap_or(0)))
    })?;

    let mut bytes = vec![0; size];

    f.read_exact(&mut bytes).map_err(|e| {
        TraceError::ReadMemoryError(pid, Errno(e.raw_os_error().unwrap_or(0)))
    })?;

    return Ok(bytes);
}

pub fn write_memory_x64(
    pid: i32,
    start_addr: u64,
    bytes: &[u8],
) -> Result<(), TraceError> {
    let filepath = format!("/proc/{}/mem", pid);

    let mut f = File::options()
        .write(true)
        .truncate(false)
        .open(filepath)
        .map_err(|e| {
            TraceError::WriteMemoryError(
                pid,
                Errno(e.raw_os_error().unwrap_or(0)),
            )
        })?;

    f.seek(SeekFrom::Start(start_addr)).map_err(|e| {
        TraceError::WriteMemoryError(pid, Errno(e.raw_os_error().unwrap_or(0)))
    })?;

    f.write_all(bytes).map_err(|e| {
        TraceError::WriteMemoryError(pid, Errno(e.raw_os_error().unwrap_or(0)))
    })?;

    return Ok(());
}

pub fn stack_push_x64(pid: i32, value: u64) -> Result<(), TraceError> {
    let rsp = register::rsp(pid)? - 8;
    write_memory_x64(pid, rsp, &value.to_le_bytes())?;
    register::set_rsp(pid, rsp)?;
    return Ok(());
}
