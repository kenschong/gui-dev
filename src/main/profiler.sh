#!/bin/bash

# Simple Profiler for Mercury Attitude Indicator (Makefile version)
# Usage: ./simple_profiler.sh [build|run|analyze|all|clean]

PROFILE_DATA="profile_data"
EXECUTABLE="gui_app"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_header() {
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}$1${NC}"
    echo -e "${GREEN}========================================${NC}"
}

print_error() {
    echo -e "${RED}ERROR: $1${NC}"
}

print_info() {
    echo -e "${YELLOW}INFO: $1${NC}"
}

# Build with profiling enabled
build_profile() {
    print_header "Building with Profiling Enabled"
    
    # Clean previous build
    make clean 2>/dev/null
    
    # Build with profiling flags
    print_info "Compiling with -pg flag for profiling..."
    
    # Add profiling flags to your compilation
    CXXFLAGS="-pg -g -O2" make
    
    if [ $? -ne 0 ]; then
        print_error "Build failed"
        print_info "You may need to modify your Makefile to accept CXXFLAGS"
        exit 1
    fi
    
    print_info "Build complete!"
}

# Run the program to generate profiling data
run_profile() {
    print_header "Running Program to Generate Profile Data"
    
    if [ ! -f "$EXECUTABLE" ]; then
        print_error "Executable '$EXECUTABLE' not found. Run './simple_profiler.sh build' first"
        exit 1
    fi
    
    mkdir -p "$PROFILE_DATA"
    cd "$PROFILE_DATA"
    
    print_info "Starting program..."
    print_info "USE THE GUI FOR 20-30 SECONDS, THEN CLOSE THE WINDOW"
    print_info "Try different scenarios and control modes to get representative data"
    echo ""
    
    ../"$EXECUTABLE"
    
    if [ ! -f "gmon.out" ]; then
        print_error "No profile data generated (gmon.out not found)"
        print_info "Make sure the program was compiled with -pg flag"
        exit 1
    fi
    
    cd ..
    print_info "Profile data saved to $PROFILE_DATA/gmon.out"
}

# Analyze profiling data
analyze_profile() {
    print_header "Analyzing Profile Data"
    
    if [ ! -f "$PROFILE_DATA/gmon.out" ]; then
        print_error "No profile data found. Run './simple_profiler.sh run' first"
        exit 1
    fi
    
    print_info "Generating flat profile (functions sorted by time)..."
    echo ""
    echo "====================================================================="
    echo "TOP FUNCTIONS BY CPU TIME"
    echo "====================================================================="
    
    gprof "$EXECUTABLE" "$PROFILE_DATA/gmon.out" -b -p | head -40
    
    echo ""
    echo "====================================================================="
    print_info "Full report saved to profile_report.txt"
    
    gprof "$EXECUTABLE" "$PROFILE_DATA/gmon.out" > profile_report.txt
    
    print_info "Call graph saved to profile_callgraph.txt"
    gprof "$EXECUTABLE" "$PROFILE_DATA/gmon.out" -q > profile_callgraph.txt
    
    echo ""
    print_info "Analysis complete! Key files:"
    echo "  - profile_report.txt      (full flat profile)"
    echo "  - profile_callgraph.txt   (call graph)"
    
    # Quick summary
    echo ""
    print_header "QUICK SUMMARY"
    echo "Look for these functions taking high % time:"
    echo "  - drawAttitudeGauge, drawRateIndicator (rendering)"
    echo "  - sin, cos (trigonometry)"
    echo "  - ImGui::Render (UI framework)"
    echo "  - updateSpacecraft (physics)"
    echo ""
    echo "Read profile_report.txt for details!"
}

# Clean profiling data
clean_profile() {
    print_header "Cleaning Profile Data"
    
    rm -rf "$PROFILE_DATA"
    rm -f profile_report.txt profile_callgraph.txt gmon.out
    
    print_info "Cleaned profile data"
}

# Show usage
usage() {
    echo "Mercury Attitude Indicator - Simple Profiler"
    echo ""
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  build    - Rebuild with profiling enabled"
    echo "  run      - Run and generate profile data"
    echo "  analyze  - Analyze profile data and show results"
    echo "  all      - Do all steps (build, run, analyze)"
    echo "  clean    - Clean profile data"
    echo ""
    echo "Quick start:"
    echo "  ./simple_profiler.sh all"
    echo ""
    echo "Then read profile_report.txt to see where time is spent!"
}

# Main script logic
case "$1" in
    build)
        build_profile
        ;;
    run)
        run_profile
        ;;
    analyze)
        analyze_profile
        ;;
    all)
        build_profile
        echo ""
        run_profile
        echo ""
        analyze_profile
        ;;
    clean)
        clean_profile
        ;;
    *)
        usage
        exit 1
        ;;
esac

exit 0