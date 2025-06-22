
## CAN Priority

| Bits      | Field Name      | Size | Description                                     |
| --------- | --------------- | ---- | ----------------------------------------------- |
| 28–26     | `priority`      | 3    | Message priority (1 = highest, 5 = lowest)      |
| 25–21     | `node_index`    | 5    | Node ID (0–31)                                  |
| 20–9      | `message_index` | 12   | Message index within node (0–4095)              |
| 8–0       | `reserved`      | 9    | Reserved (set to 0; may be used in future)      |
| **Total** |                 | 29   | CAN 2.0B extended ID format (29-bit identifier) |

- The lowest CAN ID wins arbitration. Priority 1 is encoded as 000b to make it highest priority.
- This format supports:
    - 5 usable priority levels
    - 32 nodes
    - 4096 messages per node

