#!/bin/bash

# 🧪 BloomDB Test Runner
# Ejecuta todos los tests: lógicos, Valgrind (memoria), y ASan (sanitizer)

set -e  # Exit on any error

echo "╔═══════════════════════════════════════════╗"
echo "║   🧪 BloomDB Complete Test Suite         ║"
echo "╚═══════════════════════════════════════════╝"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

function run_test_suite() {
    local suite_name=$1
    local make_target=$2
    
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${YELLOW}  Running: $suite_name${NC}"
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    
    if make $make_target; then
        echo -e "${GREEN}✅ $suite_name: PASSED${NC}"
        return 0
    else
        echo -e "${RED}❌ $suite_name: FAILED${NC}"
        return 1
    fi
}

# Track failures
failed=0

# 1. Normal tests (correctness)
run_test_suite "Logical Tests" "test" || failed=1
echo ""

# 2. Valgrind tests (memory leaks)
run_test_suite "Valgrind Memory Tests" "valgrind" || failed=1
echo ""

# 3. ASan tests (address sanitizer)
run_test_suite "ASan Tests" "asan" || failed=1
echo ""

# Final summary
echo "╔═══════════════════════════════════════════╗"
if [ $failed -eq 0 ]; then
    echo -e "║   ${GREEN}🎉 ALL TESTS PASSED!${NC}                   ║"
    echo "╚═══════════════════════════════════════════╝"
    exit 0
else
    echo -e "║   ${RED}❌ SOME TESTS FAILED${NC}                   ║"
    echo "╚═══════════════════════════════════════════╝"
    exit 1
fi
