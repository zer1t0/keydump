use super::TraceError;
use super::ptrace;

pub fn rax(pid: i32) -> Result<u64, TraceError> {
    let regs = ptrace::getregs_x64(pid)?;
    return Ok(regs.rax);
}

pub fn rip(pid: i32) -> Result<u64, TraceError> {
    let regs = ptrace::getregs_x64(pid)?;
    return Ok(regs.rip);
}

pub fn rsp(pid: i32) -> Result<u64, TraceError> {
    let regs = ptrace::getregs_x64(pid)?;
    return Ok(regs.rsp);
}

pub fn set_rip(pid: i32, rip: u64) -> Result<(), TraceError> {
    let mut regs = ptrace::getregs_x64(pid)?;
    regs.rip = rip;
    ptrace::setregs_x64(pid, &regs)?;
    return Ok(());
}


pub fn set_rsp(pid: i32, rsp: u64) -> Result<(), TraceError> {
    let mut regs = ptrace::getregs_x64(pid)?;
    regs.rsp = rsp;
    ptrace::setregs_x64(pid, &regs)?;
    return Ok(());
}
