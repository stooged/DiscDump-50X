#!/bin/bash
set -e
pushd tool
make
popd
pushd DiscDump
make
popd
mkdir -p bin
rm -f bin/DiscDump.bin
cp DiscDump/DiscDump.bin bin/DiscDump.bin
mkdir -p html_payload
tool/bin2js bin/DiscDump.bin > html_payload/payload.js
FILESIZE=$(stat -c%s "bin/DiscDump.bin")
PNAME=$"DiscDump"
cp exploit.template html_payload/DiscDump.html
sed -i -f - html_payload/DiscDump.html << EOF
s/#NAME#/$PNAME/g
s/#BUF#/$FILESIZE/g
s/#PAY#/$(cat html_payload/payload.js)/g
EOF
rm -f DiscDump/DiscDump.bin
rm -f html_payload/payload.js
