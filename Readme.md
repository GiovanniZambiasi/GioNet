# GioNet
A low level socket library for multiplayer games based on UDP and Winsock!

## Features
- Simulates connections like TCP
- Uses UDP for speed
- Implements a [reliability layer](#reliability-layer), allowing specific packets to be guaranteed to arrive, and ordered

# Reliability Layer
GioNet packets can be sent reliably, despite using the UDP protocol behind the scenes. To ensure the packets you send
are reliable, make sure to set the ``reliable`` parameter of the ``GioNet::Message`` struct to ``true``.

The reliability layer implemented in this library is based on the concept of redundancy. You can read more about it in
[this article by Glenn Fiedler](https://gafferongames.com/post/reliability_ordering_and_congestion_avoidance_over_udp/).
Essentially, every packet sent between two hosts includes an ``ackId``, and an ``ackBitset``. 

## Ack ID
The ``ackId`` will always be
set to the id of the latest incoming _reliable_ packet. Note that, because packet ids will eventually overflow, the 
_latest id_ will sometimes be smaller than a previous one. For example, if the current ``ackId`` is 
``GioNet::Packet::MaxPossibleId``, the next expected ``ackId`` will be ``1`` or ``2``.

## Ack Bitset
The ``ackBitset`` is used to encode a series of _acks_ into a single data structure, using the ``ackId`` as a reference point.
In essence, each "on" bit of the ``ackBitset`` represents an ack for a packet id. This packet id will be the current ``ackId``, 
minus N where N is the position of the respective bit. Here's an example:

> Suppose the current ``ackId`` is **5**, to ack the packet ids:
**1** and **3**, the ``ackBitset`` should be ``0b1010``. Note that the first bit (from right to left) is _off_, meaning packet id 4
has not yet been received. Meanwhile, the second bit (packet id 3) is _on_, since we've already received that packet.

<br><br>
Remember that every packet sent (whether they are reliable or not) will include both an ``ackId`` and an ``ackBitset``.
In practice, multiple acks will be sent for a given packet id. This means that, even if some packets don't make it through
to the server, there will likely be dozens of others that will, which will redundantly include an ack for a given packet.
This redundancy is what provides reliability with a certain degree of safety.

## TODO
- [ ] UDP "connections"
- [ ] UDP reliability layer
    - [X] Abstract socket from connections class (only host classes should handle sockets to facilitate adding congestion control later)
    - [X] Maybe replace Peer struct with Connection class?
    - [X] ack bitset should piggy back off of every packet
    - [ ] If an ack has not been received and the bitset has moved beyond it, hosts should be able to send an ack request
    - [ ] Discard old unnecessary packets
    - [ ] Reschedule missing packets to send to clients after missing acks received
- [ ] Packet fragmentation
- [ ] Socket thread safety
- [ ] Make API work on unix (kinda easy apparently 🤷‍♂️)
