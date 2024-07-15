
use std::fmt;
use errno::Errno;

#[derive(Debug)]
pub enum TraceError {
    AttachError(i32, Errno),
    DetachError(i32, Errno),
    GetRegsError(i32, Errno),
    GetSyscallInfoError(i32, Errno),
    PeekTextError(i32, Errno),
    ReadMemoryError(i32, Errno),
    SetRegsError(i32, Errno),
    SignalError(i32, Errno),
    SingleStepError(i32, Errno),
    SyscallError(i32, Errno),
    SyscallNotFoundError(i32),
    WaitError,
    WriteMemoryError(i32, Errno),
}

impl fmt::Display for TraceError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::AttachError(pid, errno) => {
                write!(f, "Attach Error on pid {}: {}", pid, errno)
            }
            Self::DetachError(pid, errno) => {
                write!(f, "Detach Error on pid {}: {}", pid, errno)
            }
            Self::GetRegsError(pid, errno) => {
                write!(f, "Get registers Error on pid {}: {}", pid, errno)
            }
            Self::GetSyscallInfoError(pid, errno) => {
                write!(f, "Get syscall info Error on pid {}: {}", pid, errno)
            }
            Self::PeekTextError(pid, errno) => {
                write!(f, "Peek Text Error on pid {}: {}", pid, errno)
            }
            Self::ReadMemoryError(pid, errno) => {
                write!(f, "Read memory Error on pid {}: {}", pid, errno)
            }
            Self::SetRegsError(pid, errno) => {
                write!(f, "Set registers Error on pid {}: {}", pid, errno)
            }
            Self::SignalError(pid, errno) => {
                write!(f, "Error sending signal to pid {}: {}", pid, errno)
            }
            Self::SingleStepError(pid, errno) => {
                write!(f, "Single step Error on pid {}: {}", pid, errno)
            }
            Self::SyscallError(pid, errno) => {
                write!(f, "Ptrace syscall Error on pid {}: {}", pid, errno)
            }
            Self::SyscallNotFoundError(pid) => {
                write!(f, "Unable to found syscall in pid {}", pid)
            }
            Self::WaitError => write!(f, "Wait Error"),
            Self::WriteMemoryError(pid, errno) => {
                write!(f, "Write memory Error on pid {}: {}", pid, errno)
            }
        }
    }
}

impl From<TraceError> for String {
    fn from(item: TraceError) -> Self {
        item.to_string()
    }
}
