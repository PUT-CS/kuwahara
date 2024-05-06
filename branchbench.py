import os
import shutil
import subprocess
import time
import argparse
import numpy as np
import matplotlib.pyplot as plt
import decimal
from colorama import init, Fore, Style

# Initialize colorama
init()

# Function to compile C++ code using cmake and make
def compile_code(folder):
    subprocess.run(["cmake", "."], cwd=folder)
    subprocess.run(["make"], cwd=folder)

# Function to clone the repository if not already cloned
def clone_repository(repo_url, clone_dir):
    if not os.path.exists(clone_dir):
        subprocess.run(["git", "clone", repo_url, clone_dir])
    elif os.listdir(clone_dir) == []:
        print(Fore.YELLOW + f"Directory {clone_dir} exists but is empty. Cloning repository..." + Style.RESET_ALL)
        subprocess.run(["git", "clone", repo_url, clone_dir])
    else:
        print(Fore.GREEN + f"Repository already exists in {clone_dir}" + Style.RESET_ALL)

# Function to switch to a specific branch
def switch_branch(branch_name, clone_dir):
    print(Fore.MAGENTA + f"Switching to {branch_name} branch..." + Style.RESET_ALL)
    subprocess.run(["git", "checkout", branch_name], cwd=clone_dir)

# Function to time the execution of a C++ program
def time_execution(program, arguments, iterations, branch_name, window_size):
    execution_times = []
    for _ in range(iterations):
        start_time = time.time()
        process = subprocess.Popen([program] + arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, cwd=os.path.dirname(program))
        stdout, stderr = process.communicate()
        end_time = time.time()

        # Print the executed command
        print(Fore.BLUE + f"Executed command: {' '.join([program] + arguments)}" + Style.RESET_ALL)

        # Print program output
        if stdout:
            print(Fore.GREEN + "Program output:")
            print(stdout)
        if stderr:
            print(Fore.RED + "Error messages:")
            print(stderr)

        # Convert stdout to decimal (if possible)
        try:
            execution_time = float(stdout)
            execution_times.append(execution_time)
            print(Fore.GREEN + f"Execution time: {execution_time:.6f} seconds" + Style.RESET_ALL)
        except decimal.InvalidOperation:
            print(Fore.RED+ "Failed to convert stdout to decimal" + Style.RESET_ALL)

    return np.mean(execution_times)

# Function to remove the cloned repository if --keep option was not provided
def remove_repository(clone_dir, keep):
    if not keep:
        print(Fore.YELLOW + f"Removing cloned repository from {clone_dir}" + Style.RESET_ALL)
        shutil.rmtree(clone_dir)
    else:
        print(Fore.GREEN + f"Cloned repository kept in {clone_dir}" + Style.RESET_ALL)

# Function to save the plot if --save option was provided
def save_plot(save_path):
    if save_path:
        plt.savefig(save_path)
        print(Fore.GREEN + f"Plot saved as {save_path}" + Style.RESET_ALL)
    else:
        plt.show()

# Main function to compile and time different implementations of the Kuwahara filter
def main(repo_url, clone_dir, branch_names, window_sizes, executable_arguments, iterations, save_path, keep):
    # Clone the repository
    clone_repository(repo_url, clone_dir)

    # Dictionary to store execution times for each branch and window size
    execution_times = {branch_name: {size: [] for size in window_sizes} for branch_name in branch_names}

    # Compile and time each branch with different arguments
    for branch_name in branch_names:
        switch_branch(branch_name, clone_dir)
        compile_code(clone_dir)
        program_path = os.path.join(clone_dir, "kuwahara")
        for j, arguments in enumerate(executable_arguments, 1):
            print(Fore.CYAN + f"Testing {branch_name} branch with window size {window_sizes[j-1]}..." + Style.RESET_ALL)
            execution_time = time_execution(program_path, arguments.split(), iterations, branch_name, window_sizes[j-1])
            execution_times[branch_name][window_sizes[j-1]].append(execution_time)

    # Remove the cloned repository if --keep option was not provided
    remove_repository(clone_dir, keep)

    # Calculate average execution times
    average_execution_times = {branch_name: {size: np.mean(times) for size, times in times_dict.items()} for branch_name, times_dict in execution_times.items()}

    # Plotting the execution times
    plt.figure(figsize=(10, 6))
    ax = plt.gca()
    ax.set_xlim(window_sizes[0], window_sizes[-1] + 1)
    ax.set_ylim(0, max([max(times.values()) for times in average_execution_times.values()]) + 0.1)
    for branch_name, times in average_execution_times.items():
        plt.plot(times.keys(), times.values(), label=branch_name, linewidth=3.0)
    plt.title('Average Execution Time by Implementation and Kernel Size')
    plt.xlabel('Kernel Size')
    plt.ylabel('Average Execution Time (seconds)')
    plt.xticks(window_sizes)
    plt.legend()
    plt.grid(True)

    # Save the plot if --save option was provided
    save_plot(save_path)

if __name__ == "__main__":
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='Compile and time different implementations of the Kuwahara filter.')
    parser.add_argument('--save', type=str, help='Save the plot to the specified file.')
    parser.add_argument('--keep', action='store_true', help='Keep the cloned repository in /tmp.')
    parser.add_argument('--iterations', type=int, default=1, help='Number of iterations for each window size and each path.')
    args = parser.parse_args()

    # Define constants
    repo_url = "https://github.com/PUT-CS/kuwahara.git"
    clone_dir = "/tmp/kuwahara"
    branch_names = ["sequential", "openmp", "cuda"]
    window_sizes = [5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45]
    executable_arguments = [f"{os.path.join(clone_dir, 'img/jez.jpg')} {os.path.join(clone_dir, 'img/test.jpg')} --window {size}" for size in window_sizes]

    # Run the main function
    main(repo_url, clone_dir, branch_names, window_sizes, executable_arguments, args.iterations, args.save, args.keep)
