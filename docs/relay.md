# Relay

Each LAN only has 2 IP addresses, one for all the apps to multicast on, and another for the services. And they each listen to the other IP for multicasts and respond in turn.

We've designed a relay to bridge 2 LANs (2 IPs each). As apps on LAN A will not be actively listening to messages multicast on LAN B's services, the relay will do so and then forward the messages to the correct IP and ports.
