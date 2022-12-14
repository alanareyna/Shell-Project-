OUT=tests/output/`basename ${0%.sh}`.out
EOUT=tests/output/`basename ${0%.sh}`.eout
rm -f $OUT $EOUT tests/output/shouldnt-exist

./lobo_shell.x >$OUT << 'EOF'
echo "hello \"hi\" hello"
EOF

echo "hello \"hi\" hello" > $EOUT

if [ -f tests/output/shouldnt-exist* ]; then 
    echo "TEST FAILED $0"
    rm -f tests/output/shouldnt-exist*
    exit
fi

diff $OUT $EOUT && echo "PASSED $0" || echo "TEST FAILED $0"