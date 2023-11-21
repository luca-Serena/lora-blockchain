## Components

- SUMO
- LUNES
- simlorasf

## USAGE
It is necessary to download ARTIS-GAIA framework, available at https://pads.cs.unibo.it/doku.php?id=pads:download. Specifically the software must be inserted inside the 'MODELS' folder.
Also, python library traci is required

Furthermore, the following files must to be created:
- gt.txt which contains the locations of the gateways in the format"xvalue, yvalue" , where each line of the file contains the location of one gateway. The number of lines of this file, thus, should be the same of the parameter GATEWAYS, contained in lunes/run
- the various files containing location and traffic configuration for SUMO. Those can be obtained by executing osmWebWizard. ('python3 /usr/share/sumo/tools/osmWebWizard.py')

To execute the software, tun the following command
'python3 launcher.py'

## Parameters
There are many parameters of the system that can be set.

Some are contained in launcher.py, such as:
- *loraVehiclePercentage*, which must be between 0 and 1, it sets the percentage of vehicles carrying LoRa sensors
- *warmup_steps*, which are the steps necessary for the vehicles to fill the map, nothing else other than standard sumo execution happens
- *actual_steps*, which are the full steps of the simulation. The total SUMO timesteps are given by warmup_steps + actual_steps
- *loraRange*, which is communication range (in meters) of LoRa physical medium
- *interaction_step*, which represent the number of steps at which LUNES and simorasf are called. We also assume that one packet is sent by every node in this gap . One step in SUMO is one second
- *num_providers*, which is the number of LoRaWAN providers


Other parameters are containes in lunes/run, such as:
- FULL_NODES, which is the number of full nodes of the permissioned blockchain
- GATEWAYS, which is the number of gateways placed in the considered area. It must corresponds to the number of lines in gt.txt
- CLIENTS, which is the number of clients
- SENSORS, which is the number of sensor owners. This does not necessarily corresponds to the number of vehicles
- SUBS, which is the number of subscription between one client and a sensor
