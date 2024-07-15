#!/usr/bin/env python3
import argparse
from elftools.elf.elffile import ELFFile

def parse_args():
    parser = argparse.ArgumentParser(
        description='Extracts shellcode from a ELF.'
    );
    parser.add_argument('elf',help="ELF file" );
    parser.add_argument('out',help='Output shellcode file');
    return parser.parse_args();


CANARY = b"ENDSHC"

def main():
    args = parse_args()

    with open(args.elf, "rb") as fi:
        elf = ELFFile(fi)

        for section in elf.iter_sections():
            if section.name == ".text":
                text_data = section.data()

    canary_index = text_data.index(b"ENDSHC")

    final_data = text_data[:canary_index]

    with open(args.out, "wb") as fo:
        fo.write(final_data)

    print("Shellcode write in", args.out)

if __name__ == '__main__':
    exit(main())
