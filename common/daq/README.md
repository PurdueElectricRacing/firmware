Add the following to the tasks.json within the .vscode directory:
}
    "label": "can_gen",
    "type": "shell",
    "command": "python ./common/daq/can_gen.py",
    "problemMatcher": []
}

The automatic code generation will be ran each time the code is compiled. In order to configure a new CAN node, add can_parse.c and can_parse.h using the templates to a folder within the node named "CAN." Change the node name in the header file to match that of the node name in the can_config.json.