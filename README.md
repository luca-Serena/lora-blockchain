## Components

- SUMO
- LUNES
- simlorasf

## USAGE
It is necessary to download ARTIS-GAIA framework, available at https://pads.cs.unibo.it/doku.php?id=pads:download. Specifically the software must be inserted inside the 'MODELS' folder or, alternatively, variable ROOT in LUNES makefile has to be changes in order to point to ARTÌS/GAIA library.
Also, SUMO simulator and python libraries *traci* and *sumolib* are required.

Furthermore, the following files must to be created:
- *gt.txt*, which contains the locations of the gateways in the format"xvalue, yvalue" , where each line of the file contains the location of one gateway. The number of lines of this file, thus, should be the same of the parameter GATEWAYS, contained in lunes/run
- the various files containing location and traffic configuration for SUMO. Those can be obtained by executing osmWebWizard. ('python3 /usr/share/sumo/tools/osmWebWizard.py')

To execute the software, tun the following command
'python3 launcher.py'

## Parameters
There are multiple parameters of the multilevel simulator that can be set by the user.

Some are contained in launcher.py, such as:
- *loraVehiclePercentage*, which must be between 0 and 1, it sets the percentage of vehicles carrying LoRa sensors
- *warmup_steps*, which are the steps necessary for the vehicles to fill the map, nothing else other than standard sumo execution happens
- *actual_steps*, which are the full steps of the simulation. The total SUMO timesteps are given by warmup_steps + actual_steps
- *loraRange*, which is communication range (in meters) of LoRa physical medium
- *interaction_step*, which represent the number of steps at which LUNES and simorasf are called. We also assume that one packet is sent by every node in this gap . One step in SUMO is one second
- *num_providers*, which is the number of LoRaWAN providers (and consequently it corresponds to the number of full nodes in the permissioned blockchain)


Other parameters are containes in lunes/run, such as:
- CLIENTS, which is the number of customers
- SENSORS, which is the number of sensor owners. This does not necessarily corresponds to the number of vehicles
- SUBS, which is the number of subscription between one client and a sensor
