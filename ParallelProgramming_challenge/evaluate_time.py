import subprocess
import statistics

max_processors = 6  # max number of processors
num_runs = 3    # number of runs to average result
print(f"Computing average time based on {num_runs} executions...")

serial_time = 0
for i in range(max_processors):
    mpi_command = "./scripts/launch.sh ./build/main ./data/molecules2.smi output.csv " + str(i+1)
    real_times = []

    for _ in range(num_runs):
        result = subprocess.run(mpi_command, shell=True, capture_output=True, text=True)
        output_lines = result.stderr.strip().split("\n")

        # Extract from stderr the time and convert it
        real_time_str = output_lines[-3].split()[1]

        minutes_str, seconds_str = real_time_str.split('m')
        minutes = int(minutes_str)
        seconds = float(seconds_str.rstrip('s'))

        total_seconds = minutes * 60 + seconds
        real_times.append(total_seconds)

    average_real_time = statistics.mean(real_times)

    minutes_result = int(average_real_time / 60)
    seconds_result = round(average_real_time % 60, 3)
    if i == 0:
        serial_time = average_real_time

    scale_factor = serial_time / average_real_time
    print(f"n_proc={i+1}, time: {minutes_result} min, {seconds_result} s, scale factor: {scale_factor:.3f}")
