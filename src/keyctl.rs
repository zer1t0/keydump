
use errno::Errno;
use libc;

const __NR_KEYCTL: i64 = 250;
const KEYCTL_READ: u64 = 11;

pub fn read_key(key_id: i32) -> Result<Vec<u8>, Errno> {
    let size = unsafe { libc::syscall(__NR_KEYCTL, KEYCTL_READ, key_id, 0, 0) };
    if size < 0 {
        return Err(Errno(-size as i32));
    }

    let mut data = vec![0; size as usize];
    let ptr = data.as_mut_ptr();

    let size =
        unsafe { libc::syscall(__NR_KEYCTL, KEYCTL_READ, key_id, ptr, size) };
    if size < 0 {
        return Err(Errno(-size as i32));
    }

    return Ok(data);
}

#[derive(Debug)]
pub struct KeyDescription {
    pub type_: String,
    pub uid: u32,
    pub gid: u32,
    pub permissions: u32,
    pub name: String,
}
