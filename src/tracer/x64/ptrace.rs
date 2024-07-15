use errno::errno;
use libc::{self, user_regs_struct};
use super::TraceError;


pub fn getregs_x64(pid: i32) -> Result<user_regs_struct, TraceError> {
    let mut regs = empty_user_regs_struct();
    let res = unsafe { libc::ptrace(libc::PTRACE_GETREGS, pid, 0, &mut regs) };
    if res == -1 {
        return Err(TraceError::GetRegsError(pid, errno()));
    }
    return Ok(regs);
}

pub fn setregs_x64(pid: i32, regs: &user_regs_struct) -> Result<(), TraceError> {
    let res = unsafe { libc::ptrace(libc::PTRACE_SETREGS, pid, 0, regs) };
    if res == -1 {
        return Err(TraceError::SetRegsError(pid, errno()));
    }
    return Ok(());
}


#[cfg(target_arch = "x86_64")]
fn empty_user_regs_struct() -> user_regs_struct {
    user_regs_struct {
        r15: 0,
        r14: 0,
        r13: 0,
        r12: 0,
        rbp: 0,
        rbx: 0,
        r11: 0,
        r10: 0,
        r9: 0,
        r8: 0,
        rax: 0,
        rcx: 0,
        rdx: 0,
        rsi: 0,
        rdi: 0,
        orig_rax: 0,
        rip: 0,
        cs: 0,
        eflags: 0,
        rsp: 0,
        ss: 0,
        fs_base: 0,
        gs_base: 0,
        ds: 0,
        es: 0,
        fs: 0,
        gs: 0,
    }
}
