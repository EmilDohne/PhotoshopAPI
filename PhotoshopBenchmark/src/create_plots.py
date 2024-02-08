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

        pprint(combined_data)

        for data_type, values in combined_data.items():
            color = color_psapi if 'psapi' in data_type else color_photoshop
            alpha = alpha_read if 'read' in data_type else alpha_write
            plt.bar(data_type, values, color=color, alpha=alpha)
            plt.text(data_type, values, f"{values:.2f}", ha='center', va='bottom')

        # Change the plot name according to if we have both or just either of the data
        if key in data_psapi and key in data_photoshop:
            plot_name = f'{key}_combined_plot.png'
        elif key in data_psapi:
            # Plot only PSAPI data
            plot_name = f'{key}_psapi_plot.png'
        elif key in data_photoshop:
            plot_name = f'{key}_photoshop_plot.png'


        plt.xlabel('Benchmark')
        plt.ylabel('Average Time (s)')
        plt.savefig(os.path.join(output_dir_path, plot_name))
        plt.close()


if __name__ == "__main__":
    base_path = os.path.join(os.path.dirname(__file__), "../")
    parsed_data_psapi = parse_file(os.path.join(base_path, "benchmarkStatisticsPSAPI.txt"))
    parsed_data_photoshop = parse_file(os.path.join(base_path, "benchmarkStatisticsPhotoshop.txt"))
    pprint(parsed_data_psapi)
    pprint(parsed_data_photoshop)
    create_plots(parsed_data_psapi, parsed_data_photoshop)