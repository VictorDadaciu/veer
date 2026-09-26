#!/usr/bin/zsh

rm -rf ~/.local/include/veer
rm -rf ~/.local/lib/veer

cd $(dirname "$0")
cmake --install build --prefix ~/.local >/dev/null
