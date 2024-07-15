use super::error::TraceError;
use errno::errno;
use libc;
use std::mem;

pub fn attach(pid: i32) -> Result<(), TraceError> {
    let res = unsafe { libc::ptrace(libc::PTRACE_ATTACH, pid, 0, 0) };
    if res == -1 {
        return Err(TraceError::AttachError(pid, errno()));
    }
    return Ok(());
}

pub fn detach(pid: i32) -> Result<(), TraceError> {
    let res = unsafe { libc::ptrace(libc::PTRACE_DETACH, pid, 0, 0) };
    if res == -1 {
        return Err(TraceError::DetachError(pid, errno()));
    }
    return Ok(());
}

pub fn singlestep(pid: i32) -> Result<(), TraceError> {
    let res = unsafe { libc::ptrace(libc::PTRACE_SINGLESTEP, pid, 0, 0) };
    if res == -1 {
        return Err(TraceError::SingleStepError(pid, errno()));
    }
    return Ok(());
}

pub fn get_syscall_info(
    pid: i32,
) -> Result<(libc::ptrace_syscall_info, usize), TraceError> {
    let mut info = libc::ptrace_syscall_info {
        op: 0,
        pad: [0, 0, 0],
        arch: 0,
        instruction_pointer: 0,
        stack_pointer: 0,
        u: libc::__c_anonymous_ptrace_syscall_info_data {
            entry: libc::__c_anonymous_ptrace_syscall_info_entry {
                nr: 0,
                args: [0, 0, 0, 0, 0, 0],
            },
        },
    };
    let size = mem::size_of::<libc::ptrace_syscall_info>();

    let res = unsafe {
        libc::ptrace(libc::PTRACE_GET_SYSCALL_INFO, pid, size, &mut info)
    };
    if res == -1 {
        return Err(TraceError::GetSyscallInfoError(pid, errno()));
    }

    return Ok((info, res as usize));
}
