import platform


class Processor: 
    """! Processor python class."""

    def __init__(self, instance_name, class_name):
        self.instance_name = instance_name
        self.class_name = class_name
        self.parameters = {}

    def toString(self):
        print("\tProcessor( %s of instance %s )" % (self.instance_name, self.class_name))
        if len(self.parameters) > 0:
            print("\t\tParameters: ")
            for key, value in self.parameters.items():
                print("\t\t\t [ %s ]: %s" % (key, value))


class Process:
    """! Process python class."""
    lastProcess = None

    def __init__(self):
        self.max_events = -1
        self.input_files = []
        self.output_files = []
        self.sequence = []
        self.libraries = []
        Process.lastProcess = self

    def add_library(self, lib):
        """! Add a libraries to the list of libraries to load, searching the appropriate paths to find it,
        and adding the correct file extension"""
        systemName = platform.system()
        if (systemName == "Linux"):
            libParts = lib.split(".")
            add_lib = libParts[0]+".so"
            pass
        if (systemName == "Darwin"):
            libParts = lib.split(".")
            add_lib = libParts[0]+".dylib"
            pass
        self.libraries.append(add_lib)

    def printProcess(self):
        """! Print process."""
        if (self.max_events > 0): print(" Maximum events to process: %d" % (self.max_events))
        else: print(" No limit on maximum events to process")

        print("Processor sequence:")
        for proc in self.sequence:
            proc.toString()
        if len(self.input_files) > 0:
            if len(self.output_files) == len(self.input_files):
                print("Files:")
                for i in range(0, len(self.input_files)):
                    print("   '%s' -> '%s'"%(self.input_files[i], self.output_files[i]))
            else:
                print("Input files:")
                for afile in self.input_files:
                    print("   %s"%(afile))
                if len(self.output_files) > 0:
                    print("Output file:", self.output_files[0])
        elif len(self.output_files) > 0:
            print("Output file:", self.output_files[0])
        if len(self.libraries) > 0:
            print("Shared libraries to load:")
            for afile in self.libraries:
                print("   %s"%(afile))
