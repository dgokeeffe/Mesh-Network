# Mesh-Network
A novel implementation in C of a simulation for a wireless sensory network

The basic idea is that 4 nodes (represented by indivudal processes) must all simultaneously generate the same random number in order to register a hit. Each time a number is generated, a series of messages is sent in parallel between the processes, and if they all match that number is sent to the BaseStation process.
