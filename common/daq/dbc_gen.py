from cantools import db
import can_gen
import json
from jsonschema import validate
from jsonschema.exceptions import ValidationError
import pickle

#can_db = cantools.db.Database()
DBC_PATH = './common/daq/per_dbc.dbc'
PICKLE_PATH = './common/daq/per_msg_objs.pkl'

def gen_dbc(can_config, dbc_path=DBC_PATH):
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
    
    can_gen.log_success("DBC Generated at " + dbc_path)

def main():
    can_config = json.load(open(can_gen.can_json_config_path))
    can_schema = json.load(open(can_gen.can_json_schema_path))
    can_gen.define_config(can_config)

    # compare with schema
    try:
        validate(can_config, can_schema)
    except ValidationError as e:
        can_gen.log_fail("Invalid JSON!")
        print(e)
        quit()

    can_gen.generate_ids()
    can_gen.generate_dlcs()

    gen_dbc(can_config)

if __name__ == "__main__":
    main()