# shellnova

A base project to create GNU/Linux shellcodes from c code in an "easy" way.

The project compiles a c binary that includes all the machine code and data
into the .text section. Then the .text section its extracted to create a
shellcode.

Additionally, shellnova can use the libc functions since it looks for them
dynamically. It is possible to expand this functionality to search for
other libraries symbols if its required.

Whatever, this project should be considered a template that requires to be
adapted for every situation.

A similar process is done in the
[Stardust](https://github.com/Cracked5pider/Stardust) project for generating
Windows shellcode programs, from which was inspirated. You can check the
following post of [C5pider](https://twitter.com/C5pider) to learn how it works:
 [Modern implant design: position independent malware development](https://5pider.net/blog/2024/01/27/modern-shellcode-implant-design/).

## Example

You can test the shellcode with the `utils/exec-shc` program:
```
cd shellnova/
make
utils/exec-shc shc.bin
```

And it should print something like:
```
[exec-shc] Shellcode start addr: 0x7a7250ad9000
[exec-shc] Shellcode end addr: 0x7a7250ada06a
PID: 16118
Start addr: 0x7a7250ad9000
Data addr: 0x7a7250ada000
End addr: 0x7a7250ada06a
Data size: 6a
Hello world
```

## Relevant Parts

- `src/main.c` is were your code goes.

- `src/linker.ld`: The linker script, which indicates where the data and code of
the final binary is going to be stored in the .text section.

- `src/lib_d.c` file includes the code for searching a library symbols.

- `src/libc_d.c` file includes the code for declaring libc symbols, it can
be expanded to include new symbols.

- `src/start.asm` and `src/start.c`: Execute the shellcode preamble before the
main. You may need to modify these files in order to adapt the shellcode to
your specific situation.

## Dependencies

In order to use the project you require the following dependencies:
```
sudo apt update
sudo apt install -y gcc cmake make nasm python3-pip
pip install -r scripts/requirements.txt
```


## Credits

-  [C5pider](https://twitter.com/C5pider)
