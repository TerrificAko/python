import os
import re
from pathlib import Path


class GenIlaXdc:
    def __init__(self, list_name):
        self.list_name = list_name
        current_dir = Path.cwd()
        self.file_path = os.path.join(current_dir, list_name)
        self.data_dict = {}
        self.clk_data_expanded_dict = {}
        self.clk_data_unexpanded_dict = {}

    def load_content(self):
        with open(self.file_path, 'r', encoding='utf-8') as f:
            content = f.readlines()
            f.close()
        return content

    def extract_content(self):
        content = self.load_content()
        self.match_cfg()
        signal_type = ['wire', 'reg', 'io', 'other']
        current_clk = ''

        for index, line in enumerate(content):
            # if line.startswith('clk'):
            get_file = self.get_file_searcher.match(line)
            get_clk = self.get_clk_searcher.match(line)
            get_bus = self.get_bus_searcher.match(line)
            get_signal = self.get_signal_searcher.match(line)

            if get_file:
                file_name = os.path.splitext(os.path.basename(content[index + 1]))[0]
                self.data_dict.setdefault(file_name, {})
                clk_index = 0
            if get_clk:
                current_clk = content[index + 1].strip()
                self.data_dict.setdefault(file_name, {}).setdefault(content[index + 1].strip(), {})

            if get_bus:
                bus_list = []
                bus_index = 0
                while not self.get_signal_searcher.match(content[index + 1 + bus_index]):
                    if content[index + 1 + bus_index].strip() not in signal_type:
                        if content[index + 1 + bus_index].strip() == "":
                            pass
                        else:
                            bus_list.append(content[index + 1 + bus_index].strip())
                    bus_index += 1
                self.data_dict[file_name][current_clk].setdefault('bus', bus_list)
            if get_signal:
                signal_list = []
                single_index = 0
                while index + 1 + single_index < len(content):
                    if self.get_file_searcher.match(content[index + 1 + single_index]):
                        break
                    if content[index + 1 + single_index].strip() not in signal_type:
                        if content[index + 1 + single_index].strip() == "":
                            pass
                        else:
                            signal_list.append(content[index + 1 + single_index].strip())
                    single_index += 1
                self.data_dict[file_name][current_clk].setdefault('signal', signal_list)

        for file_name, file_data in self.data_dict.items():
            for clk_name, clk_data in file_data.items():
                self.clk_data_expanded_dict.setdefault(clk_name, {})
                self.clk_data_unexpanded_dict.setdefault(clk_name, {})
                for signal_type, signal_list in clk_data.items():
                    if signal_type == 'bus':
                        for bus_unexpanded in signal_list:
                            replaced_bus = bus_unexpanded.replace('.', '/')
                            self.clk_data_unexpanded_dict[clk_name].setdefault('bus', []).append(replaced_bus)
                            range_search = re.search(r"\[(\d+):(\d+)\]", replaced_bus)
                            if range_search:
                                start = int(range_search.group(2))
                                end = int(range_search.group(1))
                                for i in range(start, end + 1):
                                    expanded_bus = f"{replaced_bus[:replaced_bus.index('[')]}[{i}]"
                                    self.clk_data_expanded_dict[clk_name].setdefault('bus', []).append(expanded_bus)
                    if signal_type == 'signal':
                        self.clk_data_expanded_dict[clk_name].setdefault('signal', []).extend(signal_list)
                        self.clk_data_unexpanded_dict[clk_name].setdefault('signal', []).extend(signal_list)

    def gen_ila_xdc(self):
        mark_debug = "set_property MARK_DEBUG true"
        ila_content_mark = []
        ila_content_probe = []
        ila_content_dbg_hub = []
        clk_index = 0
        dbg_hub_clk = 'axi_clk'
        for clk_name, clk_data in self.clk_data_expanded_dict.items():
            for signal_type, signal_list in clk_data.items():
                for signal in signal_list:
                    wrapped_signal = "{" + signal + "}"
                    ila_content_mark.append(f"{mark_debug} [get_nets {wrapped_signal}]")

        for clk_name, clk_data in self.clk_data_unexpanded_dict.items():
            probe_index = 0
            #probe_str = f"probe{probe_index}"
            ila_data_depth = 1024
            ila_index = f"u_ila_{clk_index}"
            get_debug_core = f"[get_debug_cores {ila_index}]"
            ila_content_probe.append(f"create_debug_core {ila_index} ila")
            ila_content_probe.append(f"set_property ALL_PROBE_SAME_MU true {get_debug_core}")
            ila_content_probe.append(f"set_property ALL_PROBE_SAME_MU_CNT 4 {get_debug_core}")
            ila_content_probe.append(f"set_property C_ADV_TRIGGER true {get_debug_core}")
            ila_content_probe.append(f"set_property C_DATA_DEPTH {ila_data_depth} {get_debug_core}")
            ila_content_probe.append(f"set_property C_EN_STRG_QUAL true {get_debug_core}")
            ila_content_probe.append(f"set_property C_INPUT_PIPE_STAGES 0 {get_debug_core}")
            ila_content_probe.append(f"set_property C_TRIGIN_EN false {get_debug_core}")
            ila_content_probe.append(f"set_property C_TRIGOUT_EN false {get_debug_core}")
            ila_content_probe.append(f"set_property C_ADV_TRIGGER true {get_debug_core}")
            ila_content_probe.append(f"set_property port_width 1 [get_debug_ports {ila_index}/clk]")
            ila_content_probe.append(f"connect_debug_port {ila_index}/clk [get_nets [list {clk_name}]]")
            for signal_type, signal_list in clk_data.items():
                if signal_type == 'bus':
                    for signal in signal_list:
                        probe_str = f"probe{probe_index}"
                        start = None
                        end = None
                        range_search = re.search(r"\[(\d+):(\d+)\]", signal)
                        if range_search:
                            start = int(range_search.group(2))
                            end = int(range_search.group(1))
                        if probe_index != 0:
                            ila_content_probe.append(f"create_debug_port {ila_index} probe")
                        ila_content_probe.append(f"set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports {ila_index}/{probe_str}]")
                        ila_content_probe.append(f"set_property port_width {end - start + 1} [get_debug_ports {ila_index}/{probe_str}]")
                        signal_expanded_long_list = ""
                        for i in range(start, end + 1):
                            signal_expanded_list_unwrapped = f"{signal[:signal.index('[')]}[{i}]"
                            signal_expanded_list_wrapped = "{" + signal_expanded_list_unwrapped + "}"
                            signal_expanded_long_list += f"{signal_expanded_list_wrapped} "
                        ila_content_probe.append(f"connect_debug_port {ila_index}/{probe_str} [get_nets [list {signal_expanded_long_list}]]")
                        probe_index += 1
                if signal_type == 'signal':
                    for signal in signal_list:
                        probe_str = f"probe{probe_index}"
                        if probe_index != 0:
                            ila_content_probe.append(f"create_debug_port {ila_index} probe")
                        ila_content_probe.append(f"set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports {ila_index}/{probe_str}]")
                        ila_content_probe.append(f"set_property port_width 1 [get_debug_ports {ila_index}/{probe_str}]")
                        ila_content_probe.append(f"connect_debug_port {ila_index}/{probe_str} [get_nets [list {signal}]]")
                        probe_index += 1
            clk_index = clk_index + 1

        ila_content_dbg_hub.append("set_property C_CLK_INPUT_FREQ_HZ 300000000 [get_debug_cores dbg_hub]")
        ila_content_dbg_hub.append("set_property C_ENABLE_CLK_DIVIDER false [get_debug_cores dbg_hub]")
        ila_content_dbg_hub.append("set_property C_USER_SCAN_CHAIN 1 [get_debug_cores dbg_hub]")
        ila_content_dbg_hub.append(f"connect_debug_port dbg_hub/clk [get_nets {dbg_hub_clk}]")
        ila_content = ila_content_mark + ila_content_probe + ila_content_dbg_hub

        with open("./ila_xdc.xdc", "w") as f:
            for line in ila_content:
                f.write(line + "\n")

    def match_cfg(self):
        # self.get_macro_ifdef = re.compile("\s*`ifdef\s*(\w+)")
        self.get_file_searcher = re.compile("\s*FILE_PATH\s*")
        self.get_clk_searcher = re.compile("\s*CLK\s*")
        self.get_bus_searcher = re.compile("\s*BUS\s*")
        self.get_signal_searcher = re.compile("\s*SINGLE\s*")


def main():
    gen_ila = GenIlaXdc(list_name='./signal_list.txt')
    gen_ila.extract_content()
    gen_ila.gen_ila_xdc()


if __name__ == '__main__':
    main()
