'''
Module for benchmarking Photoshop itself using the python photoshop api. We define a test harness for each of the files and check read/write speeds
restarting in between runs to clear photoshops scratch disk. 

These benchmarks are currently very windows specific and will most likely not run on mac (photoshop doesnt support linux)

NOTE: currently this writes only PSD files which makes minimal differences in terms of write speed for files <2GB but for files >2GB the program will fail at a certain point
      which we account for by still logging the time but appending an asterisk at the end to signal that the run was not complete

Exceptions:
    PsNotInstalledError     : Exception raised when photoshop is not found to be installed on this system
    PsFileNotExistsError    : Exception raised when the given photoshop file does not exist
    PsInvalidFileError      : Exception raised when the given photoshop file does not have a valid extension (.psd or .psb)
    PsIncompleteWriteError  : Exception raised when a file write 
'''
import photoshop.api as ps
import time
import os
import subprocess
import re


class PsNotInstalledError(Exception):
    pass

class PsFileNotExistsError(Exception):
    pass

class PsInvalidFileError(Exception):
    pass

class PsIncompleteWriteError(Exception):
    pass


def _find_photoshop(search_path: str) -> str:
    '''
    Find and return the absolute path of a Photoshop executable given the base path to the installed adobe folder.

    If on windows the search_path for example would be C:/Program Files/Adobe/
    '''
    if not os.path.exists(search_path):
        raise PsNotInstalledError(f"Unable to locate '{search_path}' on the system")

    photoshop_folder_path = None

    # Match the latest installed version if multiple exist
    for entry in os.listdir(search_path):
        if re.match(r"^Adobe Photoshop", entry):
            photoshop_folder_path = os.path.join(search_path, entry)

    if not photoshop_folder_path:
        raise PsNotInstalledError(f"Unable to locate a 'Adobe Photoshop*' folder in the provided path")
    
    print(f"Found Photoshop Folder f'{os.path.basename(photoshop_folder_path)}'")
    for file in os.listdir(photoshop_folder_path):
        if file.lower() == "photoshop.exe":
            return os.path.join(photoshop_folder_path, file)
    
    raise PsNotInstalledError("Was able to locate the Adobe Photoshop folder but couldnt find a 'Photoshop.exe' in it")


# Absolute path to the Photoshop.exe file
PHOTOSHOP_EXE_PATH = _find_photoshop("C:/Program Files/Adobe/")


def restart_photoshop():
    '''
    Close and restart the Photoshop.exe to ensure each run is "clean"
    '''
    # Close Photoshop
    subprocess.call(['taskkill', '/F', '/IM', 'Photoshop.exe'])
    # Wait for some time to ensure Photoshop is completely closed
    time.sleep(5)
    # Open Photoshop
    subprocess.Popen([PHOTOSHOP_EXE_PATH])


class PhotoshopTestHarness():
    '''
    Test harness for measuring Photoshop performance through read and write operations. Each test harness
    restarts photoshop and first loads, then writes the files. The rationale between restarts is that Photoshop
    stores file data on disk meaning the first run would run at normal speed and any run after that would run 
    at ~1.75x the speed. The methodology of writing through the python photoshop api produces identical times
    to opening through photoshop (tested with a simple stopwatch and manually opening)
    '''

    def __init__(self, in_file_path: str, out_file_path: str, bench_name: str):
        if not os.path.isfile(in_file_path):
            raise PsFileNotExistsError(f"Input file '{in_file_path}' does not exist")
        if os.path.splitext(in_file_path)[1] not in (".psd", ".psb"):
            raise PsInvalidFileError(f"Input file '{in_file_path}' does not have a valid photoshop extension")
        
        if not os.path.exists(os.path.dirname(out_file_path)):
            raise RuntimeError(f"Output directory '{os.path.dirname(out_file_path)}' does not exist")
        if os.path.splitext(out_file_path)[1] not in (".psd", ".psb"):
            raise PsInvalidFileError(f"Output file '{out_file_path}' does not have a valid photoshop extension")

        self.in_file_path = in_file_path
        self.out_file_path = out_file_path
        self.bench_name = bench_name


    @staticmethod
    def _timer_decorator_harness(bench_prefix: str, out_file_path: str, exception_to_detect: type[Exception] = None):
        '''
        Wrapper function so that we can time a function by adding this as a decorator. Requires a call from the PhotoshopTestHarness class
        '''
        def decorator(func):
            def wrapper(*args, **kwargs):
                start = time.perf_counter_ns()
                try:
                    result = func(*args, **kwargs)
                    exception_raised = False
                except exception_to_detect as e:
                    result = None
                    exception_raised = True
                end = time.perf_counter_ns() - start

                # args[0] is the class instance
                joined_out_path = os.path.join(os.path.dirname(__file__), "../", out_file_path)    # Create the file relative to us

                # We check if the given exception was raised and if that is the case we attach an asterisk
                with open(joined_out_path, "a") as f:
                    if exception_raised:
                        f.write(f"{bench_prefix}{args[0].bench_name}*: {float(end) / 1000000}ms\n")
                    else:
                        f.write(f"{bench_prefix}{args[0].bench_name}: {float(end) / 1000000}ms\n")
                if exception_raised:
                    print(f"{bench_prefix}{args[0].bench_name}*: {float(end) / 1000000}ms")
                else:
                    print(f"{bench_prefix}{args[0].bench_name}: {float(end) / 1000000}ms")
                return result
            return wrapper
        return decorator


    @_timer_decorator_harness(bench_prefix="read", out_file_path="benchmarkStatisticsPhotoshop.txt")
    def _read_file(self, app: ps.Application):
        '''
        Read the given file path with photoshop, use the blocking behaviour to get an accurate time.
        We initialize the app outside of this scope to avoid counting startup times
        '''
        app.load(self.in_file_path)


    @_timer_decorator_harness(bench_prefix="write", out_file_path="benchmarkStatisticsPhotoshop.txt", exception_to_detect=PsIncompleteWriteError)
    def _write_file(self, app: ps.Application):
        '''
        Write the given file path with photoshop, use the blocking behaviour to get an accurate time.
        We initialize the app outside of this scope to avoid counting startup times
        '''
        curr_doc = app.documents.getByName(os.path.basename(self.in_file_path))
        options = ps.PhotoshopSaveOptions()

        # This is unfortunately a bit of a hack since we cannot get Photoshop to write .psb
        # files using the python photoshop api so at some point when writing the photoshop files
        # it will crash if it exceeds 2gb. From looking into it, it appears that it writes
        # the files until the ImageData section after which it will check the file size and abort 
        # (presumably to write the LayerAndMaskInformation marker). The files do exceed 2gb 
        # and it should be fairly close to what it actually is but just keep in mind that these results
        # may be skewed and that one has to manually click away the Error that pops up for it to actually
        # fail
        try:
            curr_doc.saveAs(self.out_file_path, options, True)
        except:
            # We catch this exception in the wrapper and write accordingly
            print(f"Unable to complete write for file '{self.in_file_path}', results will have an asterisk added")
            raise PsIncompleteWriteError(f"Unable to complete write for file '{self.in_file_path}', results will have an asterisk added")


    def run(self):
        # Continuously try querying the photoshop application until we are able to initialize the application after which we run the test suite
        restart_photoshop()
        ps_app = None
        total_time_slept = 0
        while True:
            try:
                ps_app = ps.Application()
                break
            except:
                # We dont want to infinitely check for photoshop
                if (total_time_slept == 30):
                    raise RuntimeError(f"Photoshop took longer than 30 seconds to start up, aborting test suite {self.bench_name}")
                time.sleep(1)
                total_time_slept += 1
        self._read_file(ps_app)
        self._write_file(ps_app)
