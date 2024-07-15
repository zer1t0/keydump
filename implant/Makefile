.PHONY: all

preset = x64-shellcode
build_type = Release

out_dir = out/build/keydump-$(build_type)

all: shc.bin

shc.bin: src/*
	cmake -S . -B $(out_dir) -DCMAKE_BUILD_TYPE=$(build_type) -DFORMAT_SHELLCODE=ON
	cmake --build $(out_dir)
	cp $(out_dir)/shc.bin .

