#!/usr/bin/zsh

rm -rf ~/.local/include/veer
rm -rf ~/.local/lib/veer
rm ~/.local/lib/libcore.a
rm ~/.local/lib/librender.a

cd $(dirname "$0")
cmake --install build --prefix ~/.local
