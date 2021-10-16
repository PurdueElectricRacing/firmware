""" gen_dbc.py: Converts CAN Message JSON configuration to DBC file """

from cantools import db
import json
import generator

def gen_dbc(can_config, dbc_path):
    """ Converts from json to dbc format """

    nodes = []
    messages = {}

    for bus in can_config['busses']:
        for node in bus['nodes']:
            nodes.append(db.Node(name=node['node_name'],comment=""))
            for msg in node['tx']:
                curr_sig_pos = 0
                msg_signals = []
                for sig in msg['signals']:
                    msg_signals.append(db.Signal(name=sig['sig_name'],
                                          start=curr_sig_pos,
                                          length=sig['length'],
                                          byte_order="little_endian",
                                          is_signed=False,
                                          initial=None,
                                          scale=1,
                                          offset=0,
                                          minimum=None,
                                          maximum=None,
                                          unit="",
                                          comment="",
                                          is_multiplexer=False,
                                          is_float=False,
                                          decimal=None))
                    curr_sig_pos += sig['length']

                messages[msg['msg_name']] = db.Message(frame_id=msg['id'],
                                        name=msg['msg_name'],
                                        length=msg['dlc'],
                                        signals=msg_signals,
                                        comment=msg['msg_desc'],
                                        is_extended_frame=True,
                                        senders=[node['node_name']])

    can_db = db.load_file(dbc_path)
    can_db.nodes.clear()
    can_db.nodes.extend(nodes)
    can_db.messages.clear()
    can_db.messages.extend(messages.values())

    can_db.refresh()

    with open(dbc_path,'w',newline='\n') as fout:
        fout.write(can_db.as_dbc_string())
    
    generator.log_success("DBC Generated at " + dbc_path)


if __name__ == "__main__":
    gen_config = json.load(open(generator.GENERATOR_CONFIG_JSON_PATH))
    config = generator.load_message_config(gen_config['can_json_config_path'], gen_config['can_json_schema_path'])
    gen_dbc(config, gen_config['dbc_output_path'])