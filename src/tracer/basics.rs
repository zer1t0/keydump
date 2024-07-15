use super::ptrace;
use super::TraceError;
use libc;
use log;

pub fn detach_process(pid: i32) -> Result<(), TraceError> {
    log::debug!("Detaching from process {}", pid);
    if let Err(e) = ptrace::detach(pid) {
        log::debug!("Ptrace detach error: {}", e);
        return Err(e);
    }
    return Ok(());
}

pub fn wait_for_stop(pid: i32) -> Result<(), TraceError> {
    let mut process_status = 0;
    let res = unsafe { libc::waitpid(pid, &mut process_status, 0) };
    if res != pid {
        return Err(TraceError::WaitError);
    }

    if !libc::WIFSTOPPED(process_status) {
        return Err(TraceError::WaitError);
    }

    return Ok(());
}

pub fn wait_for_singlestep(pid: i32) -> Result<(), TraceError> {
    let mut process_status = 0;
    let res =
        unsafe { libc::waitpid(pid, &mut process_status, libc::WUNTRACED) };
    if res != pid {
        return Err(TraceError::WaitError);
    }

    if !was_stopped_by_sigtrap(process_status) {
        return Err(TraceError::WaitError);
    }

    return Ok(());
}

fn was_stopped_by_sigtrap(status: i32) -> bool {
    return libc::WIFSTOPPED(status) && libc::WSTOPSIG(status) == libc::SIGTRAP;
}

// TODO: Check target architecture, can be done with PTRACE_GET_SYSCALL
// but sometimes it fails.
pub fn attach_process(pid: i32) -> Result<(), TraceError> {
    ptrace::attach(pid)?;
    wait_for_stop(pid)?;
    return Ok(());
}

pub fn singlestep(pid: i32) -> Result<(), TraceError> {
    ptrace::singlestep(pid)?;
    wait_for_singlestep(pid)?;
    return Ok(());
}
