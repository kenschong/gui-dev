#!/usr/bin/env python3
"""
Performance Scatter Plot for Mercury Attitude Indicator
Generates beautiful scatter plot from gprof output
"""

import matplotlib.pyplot as plt
import numpy as np
import sys

def parse_gprof_output(filename):
    """Parse gprof flat profile output"""
    functions = []
    times = []
    calls = []
    
    with open(filename, 'r') as f:
        lines = f.readlines()
    
    # Find the flat profile section
    in_flat_profile = False
    for line in lines:
        if 'time   seconds' in line:
            in_flat_profile = True
            continue
        
        if in_flat_profile:
            # Stop at empty line or next section
            if line.strip() == '' or line.startswith('Call graph'):
                break
            
            # Parse the line
            parts = line.split()
            if len(parts) >= 7 and parts[0].replace('.', '').isdigit():
                try:
                    time_percent = float(parts[0])
                    func_name = ' '.join(parts[6:])
                    num_calls = int(parts[3]) if parts[3].isdigit() else 0
                    
                    # Skip very small contributors
                    if time_percent >= 0.5:
                        functions.append(func_name)
                        times.append(time_percent)
                        calls.append(num_calls)
                except (ValueError, IndexError):
                    continue
    
    return functions, times, calls

def create_scatter_plot(functions, times, calls):
    """Create beautiful scatter plot"""
    
    # Filter out functions with 0 calls
    valid_indices = [i for i, c in enumerate(calls) if c > 0]
    if not valid_indices:
        print("No valid call data to plot")
        return
    
    scatter_calls = np.array([calls[i] for i in valid_indices])
    scatter_times = np.array([times[i] for i in valid_indices])
    scatter_names = [functions[i] for i in valid_indices]
    
    # Shorten long function names and remove arguments
    short_names = []
    for func in scatter_names:
        # Remove function arguments (everything after first parenthesis)
        func_name_only = func.split('(')[0]
        
        if len(func_name_only) > 40:
            short_names.append(func_name_only[:37] + '...')
        else:
            short_names.append(func_name_only)
    
    # Create figure
    fig, ax = plt.subplots(figsize=(14, 8))
    fig.patch.set_facecolor('white')
    
    # Calculate marker sizes based on impact (time * log(calls))
    impact = scatter_times * np.log10(scatter_calls + 1)
    marker_sizes = 50 + 400 * (impact / np.max(impact))
    
    # Create scatter plot
    scatter = ax.scatter(scatter_calls, scatter_times, 
                        s=marker_sizes, 
                        c=scatter_times,
                        cmap='YlOrRd',
                        alpha=0.7,
                        edgecolors='black', 
                        linewidth=1.5)
    
    # Log scale for X-axis
    ax.set_xscale('log')
    
    # Labels and title
    ax.set_xlabel('Number of Calls')
    ax.set_ylabel('CPU Time (%)')
    ax.set_title('Application Profile Performance')
    
    # Grid
    ax.grid(True, alpha=0.3, linestyle='-', linewidth=0.8)
    ax.set_axisbelow(True)

    # Add colorbar
    cbar = plt.colorbar(scatter, ax=ax)
    
    # Styling
    ax.tick_params(labelsize=12)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.spines['left'].set_linewidth(1.5)
    ax.spines['bottom'].set_linewidth(1.5)
    
    # Calculate midpoint for smart positioning
    xlims = ax.get_xlim()
    x_mid = 10**(np.mean(np.log10(xlims)))
    
    # Annotate significant functions (>2% time)
    for i, (c, t, name) in enumerate(zip(scatter_calls, scatter_times, short_names)):
        if t > 2.0:
            # Smart positioning to avoid edges - but keep labels close
            # If point is in right half, position label to the left
            if c > x_mid:
                offset_x = c * 0.75  # Closer to point (was 0.5)
                ha = 'right'
            else:
                offset_x = c * 1.25  # Closer to point (was 1.5)
                ha = 'left'
            
            # Keep y position similar to point
            offset_y = t
            
            ax.annotate(name, (c, t), 
                       xytext=(offset_x, offset_y),
                       horizontalalignment=ha,
                       bbox=dict(boxstyle='round,pad=0.5', 
                                facecolor='lightyellow',
                                edgecolor='black', linewidth=1),
                       arrowprops=dict(arrowstyle='->', 
                                      color='black', linewidth=1))
    
    plt.tight_layout()
    
    # Save figure
    output_file = 'scatter_plot.png'
    plt.savefig(output_file, dpi=300, bbox_inches='tight', 
                facecolor='white', edgecolor='none')
    print(f"\n✓ Saved scatter plot to: {output_file} (300 DPI)")
    
    # Also save as PDF (vector)
    output_pdf = 'scatter_plot.pdf'
    plt.savefig(output_pdf, format='pdf', bbox_inches='tight',
                facecolor='white', edgecolor='none')
    print(f"✓ Saved vector version to: {output_pdf}")
    
    
    # Show the plot
    plt.show()
    
    return scatter_calls, scatter_times, short_names

def main():
    if len(sys.argv) < 2:
        print("Usage: ./scatter_plot.py profile_report.txt")
        print("\nGenerates a scatter plot from gprof output.")
        sys.exit(1)
    
    input_file = sys.argv[1]
    
    try:
        print(f"Reading profile data from: {input_file}")
        functions, times, calls = parse_gprof_output(input_file)
        
        if not functions:
            print("ERROR: No profile data found in file")
            print("Make sure you're providing the gprof output file (profile_report.txt)")
            sys.exit(1)
        
        print(f"Found {len(functions)} functions in profile")
        
        # Create scatter plot
        create_scatter_plot(functions, times, calls)
        
    except FileNotFoundError:
        print(f"ERROR: File not found: {input_file}")
        sys.exit(1)
    except Exception as e:
        print(f"ERROR: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main()