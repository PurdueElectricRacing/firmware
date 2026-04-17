Irving Wang (irvingw@purdue.edu)

## DAQ SPMC Buffer
Specialized data structure designed specifically for DAQ.
- Lockless
- Priority aware
- High throughput
- DMA friendly

DAQ26 setup:
- Producer(s): CAN1 and CAN2 ISRs
- Master Consumer: SD thread
- Follower Consumer: Ethernet thread

Notes:
- Even though there are actually two producing ISRs (CAN1 and CAN2), they have the same priority and cannot preempt each other, so we can treat them as a single producer for the purposes of this data structure.
- Data is returned to the consumers in contiguous chunks to optimize for DMA transfers.
    - The total capacity of the buffer is sized to be a multiple of chunk size to prevent fragmentation.
- If the SD thread falls behind, data will be dropped until it catches up. These "overflows" are tracked in a counter.
    - Several buffer parameters can be tuned to reduce the likelihood of overflows.
- If the ETH thread falls far behind the SD thread, it will fast-forward it's tail to the location of the SD thread. The "dropped frames" are also tracked in a counter.

> [!NOTE]
> todo more detailed docs

![sd fsm](SD_thread_FSM.drawio.png)

![timing diagram](SD_timing_diagram.drawio.png)