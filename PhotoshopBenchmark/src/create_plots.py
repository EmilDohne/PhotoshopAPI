import matplotlib.pyplot as plt
import os
from pprint import pprint


def parse_file_timing(path: str) -> dict:
    '''
    Parse a file line by line and return a dictionary containing another dict with read and write times respectively.

    The output dict will look a little like this:
    {
        {"TestFile_8bit"} : {"read": 50ms, "write": 85ms}
    }
    '''
    data = {}

    file = open(path, 'r')
    for line in file:
        name, time = line.split(':')
        name = name.strip()
        time = time.strip()
        time = time.split("ms")[0]
        time = float(time) / 1000    # We want seconds instead of ms

        parsed_name = name.replace("read", "")
        parsed_name = parsed_name.replace("write", "")
        parsed_name = parsed_name.replace("~", "")
        # We want to separate out the star and instead store the star in 
        # the write* key
        parsed_name_no_star = parsed_name.replace("*", "")

        if not parsed_name_no_star in data:
            data[parsed_name_no_star] = {"read" : [], "write" : [], "read*" : [], "write*" : []}

        if "read" in name: 
            if parsed_name_no_star == parsed_name:
                data[parsed_name_no_star]["read"].append(time)
            else:
                data[parsed_name_no_star]["read*"].append(time)
        elif "write" in name:
            if parsed_name_no_star == parsed_name:
                data[parsed_name_no_star]["write"].append(time)
            else:
                data[parsed_name_no_star]["write*"].append(time)

    # Average out the list and delete any empty keys
    for key in data.copy():
        try:
            data[key]["read"] = sum(data[key]["read"]) / len(data[key]["read"])
        except ZeroDivisionError:
            del data[key]["read"] 
        try:
            data[key]["read*"] = sum(data[key]["read*"]) / len(data[key]["read*"])
        except ZeroDivisionError:
            del data[key]["read*"] 
        try:
            data[key]["write"] = sum(data[key]["write"]) / len(data[key]["write"])
        except ZeroDivisionError:
            del data[key]["write"] 
        try:
            data[key]["write*"] = sum(data[key]["write*"]) / len(data[key]["write*"])
        except ZeroDivisionError:
            del data[key]["write*"]

    return data


def parse_file_size(path: str) -> dict:
    '''
    Parse a file line by line and return a dict containing the key and the file size for each of the methods.
    Available methods are: 'ps', 'psapi', 'psapizip' 
    '''
    data = {}

    file = open(path, 'r')
    for line in file:
        name, size = line.split(':')
        name = name.strip()
        size = size.strip()
        size = size.split("KB")[0]

        parsed_name = name.replace("ps", "")
        parsed_name = parsed_name.replace("api", "")
        parsed_name = parsed_name.replace("zip", "")

        if not parsed_name in data:
            data[parsed_name] = {"ps" : [], "psapi" : [], "psapizip" : []}

        if "ps" in name: 
            data[parsed_name]["ps"].append(float(size) / 1024 / 1024)  # The data is in KB but we want GB
        elif "api" in name:
            data[parsed_name]["psapi"].append(float(size) / 1024 / 1024)  # The data is in KB but we want GB
        elif "zip" in name:
            data[parsed_name]["psapizip"].append(float(size) / 1024 / 1024)  # The data is in KB but we want GB

    # Average out the list and delete any empty keys
    for key in data.copy():
        try:
            data[key]["ps"] = sum(data[key]["ps"]) / len(data[key]["ps"])
        except ZeroDivisionError:
            del data[key]["ps"] 
        try:
            data[key]["psapi"] = sum(data[key]["psapi"]) / len(data[key]["psapi"])
        except ZeroDivisionError:
            del data[key]["psapi"] 
        try:
            data[key]["psapizip"] = sum(data[key]["psapizip"]) / len(data[key]["psapizip"])
        except ZeroDivisionError:
            del data[key]["psapizip"]

    return data


def create_timing_plots(data_psapi: dict, data_photoshop: dict, output_dir: str = "plots") -> None:
    output_dir_path = os.path.join(os.path.dirname(__file__), "../", output_dir )
    os.makedirs(output_dir_path, exist_ok=True)
    print(f"Created output dir {output_dir_path}")

    color_psapi = "blue"
    color_photoshop = "red"
    alpha_read = .5
    alpha_write = .7

    # Combine keys from both data sets
    all_keys = set(data_psapi.keys()) | set(data_photoshop.keys())

    for key in all_keys:
        plt.figure()  # Create a new figure for each key
        plt.tight_layout()
        plt.title(key)
        
        # Combine data for the key
        # Get data for the key from both dictionaries
        psapi_data = data_psapi.get(key, {})
        photoshop_data = data_photoshop.get(key, {})

        # Store the data 
        combined_data = {
            "psapi_read": psapi_data.get("read", 0),
            "psapi_read*": psapi_data.get("read*", 0),
            "psapi_write": psapi_data.get("write", 0),
            "psapi_write*": psapi_data.get("write*", 0),
            "photoshop_read": photoshop_data.get("read", 0),
            "photoshop_read*": photoshop_data.get("read*", 0),
            "photoshop_write": photoshop_data.get("write", 0),
            "photoshop_write*": photoshop_data.get("write*", 0),
        }
        # Delete any empty items
        for _key, _value in combined_data.copy().items():
            if _value == 0:
                del combined_data[_key]

        for data_type, values in combined_data.items():
            color = color_psapi if 'psapi' in data_type else color_photoshop
            alpha = alpha_read if 'read' in data_type else alpha_write
            plt.bar(data_type, values, color=color, alpha=alpha)
            plt.text(data_type, values, f"{values:.2f}", ha='center', va='bottom')

        # Change the plot name according to if we have both or just either of the data
        if key in data_psapi and key in data_photoshop:
            plot_name = f'{key.replace(" ", "_")}_combined_plot.png'
        elif key in data_psapi:
            # Plot only PSAPI data
            plot_name = f'{key.replace(" ", "_")}_psapi_plot.png'
        elif key in data_photoshop:
            plot_name = f'{key.replace(" ", "_")}_photoshop_plot.png'


        plt.xlabel('Benchmark')
        plt.ylabel('Average Time (s)')
        plt.savefig(os.path.join(output_dir_path, plot_name))
        plt.close()


def create_timing_plots_bitdepth(data_psapi: dict, data_photoshop: dict, output_dir: str = "plots") -> None:
    '''
    Create plots for all the parsed timings but unlike create_timing_plots it combines it all into
    a single plot
    '''
    output_dir_path = os.path.join(os.path.dirname(__file__), "../", output_dir )
    os.makedirs(output_dir_path, exist_ok=True)
    print(f"Created output dir {output_dir_path}")

    color_psapi = "blue"
    color_photoshop = "red"
    alpha_read = .5
    alpha_write = .7

    # Extracting keys present in both datasets
    keys_psapi = list(data_psapi.keys())
    keys_photoshop = list(data_photoshop.keys())

    for bit_pattern in ["8-bit", "16-bit", "32-bit"]:
        # Filter keys based on the bit pattern
        filtered_keys_psapi = [key for key in keys_psapi if bit_pattern in key]
        filtered_keys_photoshop = [key for key in keys_photoshop if bit_pattern in key]

        # Determine the number of bars
        num_bars = len(filtered_keys_psapi)
        # Set the width of the bars
        bar_width = 0.35
        # Calculate the shift for the Photoshop bars
        bar_shift = bar_width / 2
        # Create plot
        fig, ax = plt.subplots(figsize=(4.5 * len(filtered_keys_photoshop), 6))
        # Plot bars for PSAPI read times
        for i, key in enumerate(filtered_keys_psapi):
            ax.bar(i - bar_shift - bar_shift/2, data_psapi[key]['read'], width=bar_width/2, align='center', label='PSAPI Read' if i == 0 else "", color=color_psapi, alpha=alpha_read)
        # Plot bars for PSAPI write times
        for i, key in enumerate(filtered_keys_psapi):
            ax.bar(i - bar_shift/2, data_psapi[key]['write'], width=bar_width/2, align='center', label='PSAPI Write' if i == 0 else "", color=color_psapi, alpha=alpha_write)
        # Plot bars for Photoshop read times (if available)
        for i, key in enumerate(filtered_keys_psapi):
            if key in data_photoshop:
                ax.bar(i, data_photoshop[key].get('read', data_photoshop[key].get('read*')), width=bar_width/2, align='edge', label='Photoshop Read' if i == 0 else "", color=color_photoshop, alpha=alpha_read)
            else:
                ax.bar(i, 0, width=bar_width/2, align='edge', label='Photoshop Read' if i == 0 else "", color=color_photoshop, alpha=alpha_read)
        # Plot bars for Photoshop write times (if available)
        for i, key in enumerate(filtered_keys_psapi):
            if key in data_photoshop:
                ax.bar(i + bar_shift, data_photoshop[key].get('write', data_photoshop[key].get('write*')), width=bar_width/2, align='edge', label='Photoshop Write' if i == 0 else "", color=color_photoshop, alpha=alpha_write)
            else:
                ax.bar(i, 0, width=bar_width/2, align='edge', label='Photoshop Write' if i == 0 else "", color=color_photoshop, alpha=alpha_read)

        for bar in ax.containers:
            if bar.patches[0]._height > 0:
                ax.bar_label(bar, fmt='%.2f', label_type='edge')

        # Add labels and legend
        ax.set_ylabel('Time (s)')
        ax.set_title(f'Read and Write Times for {bit_pattern}')
        ax.set_xticks(range(num_bars))
        ax.set_xticklabels([key.replace(f"({bit_pattern}) ", "") for key in filtered_keys_psapi], ha='center')
        ax.legend()

        plt.tight_layout()
        plot_name = f"{bit_pattern}_graphs.png"
        plt.savefig(os.path.join(output_dir_path, plot_name))


def create_size_plots(data: dict, output_dir: str = "plots") -> None:
    output_dir_path = os.path.join(os.path.dirname(__file__), "../", output_dir )
    os.makedirs(output_dir_path, exist_ok=True)
    print(f"Created output dir {output_dir_path}")

    color_psapi = "blue"
    color_photoshop = "red"
    alpha = .7

    for key in data:
        plt.figure()  # Create a new figure for each key
        plt.tight_layout()
        plt.title(key)

        for _key, _value in data[key].items():

            color = color_psapi if 'psapi' in _key else color_photoshop
            plt.bar(_key, _value, color=color, alpha=alpha)
            plt.text(_key, _value, f"{_value:.2f}", ha='center', va='bottom')

        plot_name = f'{key.replace(" ", "_")}_combined_plot.png'

        plt.xlabel('Write type')
        plt.ylabel('File Size (GB)')
        plt.savefig(os.path.join(output_dir_path, plot_name))
        plt.close()


if __name__ == "__main__":
    base_path = os.path.join(os.path.dirname(__file__), "../")
    parsed_data_psapi = parse_file_timing(os.path.join(base_path, "benchmarkStatisticsPSAPI.txt"))
    parsed_data_photoshop = parse_file_timing(os.path.join(base_path, "benchmarkStatisticsPhotoshop.txt"))
    create_timing_plots(parsed_data_psapi, parsed_data_photoshop)
    create_timing_plots_bitdepth(parsed_data_psapi, parsed_data_photoshop)
    parsed_data_sizes = parse_file_size(os.path.join(base_path, "FileSizeBench.txt"))
    create_size_plots(parsed_data_sizes)
    parsed_data_single_layer_sizes = parse_file_size(os.path.join(base_path, "FileSizeBenchSingleLayer.txt"))
    create_size_plots(parsed_data_single_layer_sizes)