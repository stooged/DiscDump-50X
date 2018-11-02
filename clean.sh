#!/bin/bash
pushd tool
make clean
popd
pushd DiscDump
make clean
popd
rm -f html_payload/DiscDump.html
rm -f bin/DiscDump.bin

