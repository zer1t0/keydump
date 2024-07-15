use super::basics;
use super::ptrace;
use super::register;
use super::TraceError;

const __NR_CLOSE: u64 = 3;
const __NR_CLONE: u64 = 56;
const __NR_FORK: u64 = 57;
const __NR_MMAP: u64 = 9;
const __NR_MUNMAP: u64 = 11;
const __NR_EXECVE: u64 = 59;

pub fn exec_mmap_x64(
    pid: i32,
    sc_addr: u64,
    addr: u64,
    length: u64,
    prot: i32,
    flags: i32,
    fd: i32,
    offset: i64,
) -> Result<u64, TraceError> {
    return execute_syscall_x64(
        pid,
        sc_addr,
        __NR_MMAP,
        addr,
        length,
        prot as u64,
        flags as u64,
        fd as u64,
        offset as u64,
    );
}

pub fn exec_munmap_x64(
    pid: i32,
    sc_addr: u64,
    addr: u64,
    length: u64,
) -> Result<u64, TraceError> {
    return execute_syscall_x64(
        pid,
        sc_addr,
        __NR_MUNMAP,
        addr,
        length,
        0,
        0,
        0,
        0,
    );
}

pub fn execute_syscall_x64(
    pid: i32,
    sc_addr: u64,
    sc_number: u64,
    rdi: u64,
    rsi: u64,
    rdx: u64,
    r10: u64,
    r8: u64,
    r9: u64,
) -> Result<u64, TraceError> {
    let backup_regs = ptrace::getregs_x64(pid)?;

    set_syscall_regs(pid, sc_addr, sc_number, rdi, rsi, rdx, r10, r8, r9)?;

    if let Err(e) = super::super::basics::singlestep(pid) {
        ptrace::setregs_x64(pid, &backup_regs)?;
        return Err(e);
    }

    let result = register::rax(pid)?;

    ptrace::setregs_x64(pid, &backup_regs)?;

    return Ok(result);
}

fn set_syscall_regs(
    pid: i32,
    sc_addr: u64,
    sc_number: u64,
    rdi: u64,
    rsi: u64,
    rdx: u64,
    r10: u64,
    r8: u64,
    r9: u64,
) -> Result<(), TraceError> {
    let mut regs = ptrace::getregs_x64(pid)?;

    regs.rip = sc_addr;
    regs.rax = sc_number;
    regs.rdi = rdi;
    regs.rsi = rsi;
    regs.rdx = rdx;
    regs.r10 = r10;
    regs.r8 = r8;
    regs.r9 = r9;

    ptrace::setregs_x64(pid, &regs)?;

    return Ok(());
}

const X64_SYSCALL_SIZE: usize = 2;
const X64_SYSCALL_OPCODES: &[u8] = &[0x0f, 0x05];

pub fn search_syscall_inst_nearby(pid: i32) -> Result<u64, TraceError> {
    let pc = register::rip(pid)?;

    if is_syscall_at_addr(pid, pc - X64_SYSCALL_SIZE as u64)? {
        return Ok(pc - X64_SYSCALL_SIZE as u64);
    } else if is_syscall_at_addr(pid, pc)? {
        return Ok(pc);
    }

    return Err(TraceError::SyscallNotFoundError(pid));
}

fn is_syscall_at_addr(pid: i32, addr: u64) -> Result<bool, TraceError> {
    let syscall_bytes =
        basics::read_memory_x64(pid, addr, X64_SYSCALL_SIZE)?;
    return Ok(syscall_bytes == X64_SYSCALL_OPCODES);
}
