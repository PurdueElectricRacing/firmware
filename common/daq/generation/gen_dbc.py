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

                    # signed signals cannot have bits less than the required size
                    # unsigned can be whatever 
                    # floats mUsT be 32

                    msg_signals.append(db.Signal(name=sig['sig_name'],
                                          start=curr_sig_pos,
                                          length=sig['length'],
                                          byte_order="little_endian",
                                          is_signed=not ('uint' in sig['type']),
                                          initial=None,
                                          scale=1 if 'scale' not in sig else sig['scale'],
                                          offset=0 if 'offset' not in sig else sig['offset'],
                                          minimum=None if 'minimum' not in sig else sig['minimum'],
                                          maximum=None if 'maximum' not in sig else sig['maximum'],
                                          unit="" if 'unit' not in sig else sig['unit'],
                                          comment="" if 'sig_desc' not in sig else sig['sig_desc'],
                                          choices=None if 'choices' not in sig else {i:choice for i, choice in enumerate(sig['choices'])},
                                          is_multiplexer=False,
                                          is_float=('float' in sig['type']),
                                          decimal=None))
                    curr_sig_pos += sig['length']
                messages[msg['msg_name']] = db.Message(frame_id=msg['id'],
                                        name=msg['msg_name'],
                                        length=msg['dlc'],
                                        signals=msg_signals,
                                        comment=msg['msg_desc'],
                                        is_extended_frame=True if 'is_normal' not in msg else not msg['is_normal'],
                                        senders=[node['node_name']],
                                        bus_name=bus['bus_name'])

    can_db = db.Database()
    can_db.nodes.clear()
    can_db.nodes.extend(nodes)
    can_db.messages.clear()
    can_db.messages.extend(messages.values())

    can_db.refresh()

    with open(dbc_path,'w',newline='\n') as fout:
        fout.write(can_db.as_dbc_string())
    
    generator.log_success("DBC Generated at " + str(dbc_path))


if __name__ == "__main__":
    gen_config = json.load(open(generator.GENERATOR_CONFIG_JSON_PATH))
    config = generator.load_message_config(gen_config['can_json_config_path'], gen_config['can_json_schema_path'])
    gen_dbc(config, gen_config['dbc_output_path'])