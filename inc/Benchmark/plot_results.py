#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

def plot_benchmarks():
    base_path = Path(__file__).parent / "results"
    graphs_path = Path(__file__).parent / "graphs"
    graphs_path.mkdir(exist_ok=True)
    
    operations = ["insert", "lookup", "remove"]
    trees = ["btree", "bplustree"]
    
    # Individual plots for each tree
    for tree in trees:
        fig, axes = plt.subplots(1, 3, figsize=(15, 4))
        fig.suptitle(f'{tree.upper()} Performance', fontsize=16)
        
        for idx, op in enumerate(operations):
            csv_file = base_path / tree / f"{op}.csv"
            if csv_file.exists():
                df = pd.read_csv(csv_file)
                axes[idx].plot(df['count'], df['time_us'], marker='o')
                axes[idx].set_xlabel('Elements')
                axes[idx].set_ylabel('Time (μs)')
                axes[idx].set_title(f'{op.capitalize()}')
                axes[idx].grid(True)
        
        plt.tight_layout()
        plt.savefig(graphs_path / f"{tree}_performance.png", dpi=150)
        plt.close()
    
    # Comparison plots
    fig, axes = plt.subplots(1, 3, figsize=(15, 4))
    fig.suptitle('BTree vs B+Tree Comparison', fontsize=16)
    
    for idx, op in enumerate(operations):
        for tree in trees:
            csv_file = base_path / tree / f"{op}.csv"
            if csv_file.exists():
                df = pd.read_csv(csv_file)
                axes[idx].plot(df['count'], df['time_us'], marker='o', label=tree.upper())
        
        axes[idx].set_xlabel('Elements')
        axes[idx].set_ylabel('Time (μs)')
        axes[idx].set_title(f'{op.capitalize()}')
        axes[idx].legend()
        axes[idx].grid(True)
    
    plt.tight_layout()
    plt.savefig(graphs_path / "comparison.png", dpi=150)
    plt.close()

if __name__ == "__main__":
    plot_benchmarks()
