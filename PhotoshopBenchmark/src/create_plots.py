import matplotlib.pyplot as plt
import os
from pprint import pprint


def parse_file(path: str) -> dict:
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

        if not parsed_name in data:
            data[parsed_name] = {"read" : [], "write" : []}

        if "read" in name: 
            data[parsed_name]["read"].append(time)
        elif "write" in name:
            data[parsed_name]["write"].append(time)

    # Average out the list
    for key in data:
        data[key]["read"] = sum(data[key]["read"]) / len(data[key]["read"])
        data[key]["write"] = sum(data[key]["write"]) / len(data[key]["write"])

    return data


def create_plots(data_psapi: dict, data_photoshop: dict, output_dir: str = "plots") -> None:
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
        if key in data_psapi and key in data_photoshop:
            # Combine data for the key
            combined_data = {
                "psapi_read": data_psapi[key]["read"],
                "psapi_write": data_psapi[key]["write"],
                "photoshop_read": data_photoshop[key]["read"],
                "photoshop_write": data_photoshop[key]["write"]
            }
            for data_type, values in combined_data.items():
                color = color_psapi if 'psapi' in data_type else color_photoshop
                alpha = alpha_read if 'read' in data_type else alpha_write
                plt.bar(data_type, values, color=color, alpha=alpha)
            plt.title(f'{key} Benchmark')
            plt.xlabel('Benchmark')
            plt.ylabel('Average Time (s)')
            plt.savefig(os.path.join(output_dir_path, f'{key}_combined_plot.png'))
            plt.close()
        elif key in data_psapi:
            # Plot only PSAPI data
            plt.bar("psapi_read", data_psapi[key]["read"], color=color_psapi, alpha=alpha_read)
            plt.bar("psapi_write", data_psapi[key]["write"], color=color_psapi, alpha=alpha_write)
            plt.title(f'{key} Benchmark')
            plt.xlabel('Benchmark')
            plt.ylabel('Average Time (s)')
            plt.savefig(os.path.join(output_dir_path, f'{key}_psapi_plot.png'))
            plt.close()
        elif key in data_photoshop:
            # Plot only Photoshop data
            plt.bar("photoshop_read", data_photoshop[key]["read"], color=color_photoshop, alpha=alpha_read)
            plt.bar("photoshop_write", data_photoshop[key]["write"], color=color_photoshop, alpha=alpha_write)
            plt.title(f'{key} Benchmark')
            plt.xlabel('Benchmark')
            plt.ylabel('Average Time (s)')
            plt.savefig(os.path.join(output_dir_path, f'{key}_photoshop_plot.png'))
            plt.close()


if __name__ == "__main__":
    base_path = os.path.join(os.path.dirname(__file__), "../")
    parsed_data_psapi = parse_file(os.path.join(base_path, "benchmarkStatisticsPSAPI.txt"))
    parsed_data_photoshop = parse_file(os.path.join(base_path, "benchmarkStatisticsPhotoshop.txt"))
    pprint(parsed_data_psapi)
    pprint(parsed_data_photoshop)
    create_plots(parsed_data_psapi, parsed_data_photoshop)