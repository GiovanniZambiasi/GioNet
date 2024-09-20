# GioNet
A barebones networking experiment using WinSock!

## TODO
- [ ] UDP "connections"
- [ ] UDP reliability layer
    - [ ] Abstract socket from connections class (only host classes should handle sockets to facilitate adding congestion control later)
    - [ ] Maybe replace Peer struct with Connection class?
    - [ ] ack bitset should piggy back off of every packet
    - [ ] If an ack has not been received and the bitset has moved beyond it, hosts should be able to send an ack request
    - [ ] Implement pings if no packets are sent within given time
- [ ] Packet fragmentation
- [ ] Socket thread safety
- [ ] Make API work on unix (kinda easy apparently 🤷‍♂️)
