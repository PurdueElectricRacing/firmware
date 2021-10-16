# DAQ Code Generation
Automatic code generation for CAN mesage parsing based on a global CAN JSON configuration

## Adding a Node
In order to configure a new CAN node, add can_parse.c and can_parse.h using the templates to a folder within the node source directory named "can." Change the node name in the header file to match that of the node name in the can_config.json.
