#!/usr/bin/env python

import os
import sys
import optparse
import time
import random
import math
from sumolib import checkBinary  # Checks for the binary in environ vars
import traci


class Gateway:
    def __init__(self, x, y, id):
        self.x = x
        self.y = y
        self.id = id
        
    def __str__(self):
        return f"Gateway: (x={self.x}, y={self.y})"


num_providers=5
interaction_step=10                         #frequency at which LUNES and simlorasf are called
warmup_steps=100                            #steps necessary for the vehicles to fill the map
actual_steps=3000                           
total_steps = warmup_steps + actual_steps   #total number of SUMO time-steps
loraVehiclePercentage= 0.2                  #percentage of vehicles in the map that are equipped with LoRa
loraRange = 2100                            #Lora Range in meters. Consider https://www.researchgate.net/publication/338071047_An_Evaluation_of_LoRa_Communication_Range_in_Urban_and_Forest_Areas_A_Case_Study_in_Brazil_and_Portugal
scenario = 1                                # 0 = normal scenario. 1 = detecting other vehicles scenario
pause_within_timesteps=0.0                  #pause between SUMO timestep in order to better visualize the movements of vehicles. It can set to 0 also
gatewayFile="gt.txt"                        #file with the locations of the gateways
transmissionsFile="lunes/transmissions.txt" #temporary file where successful transmission of LoRa packages are stored
nodesFile = "nodes.txt"                     #temporary file where temporary locations of the nodes are stored
energyResFile = "LoRa-data.txt"             #file with LoRa results
#osmLocation = "locations/bologna-dintorni-20k"
osmLocation = "locations/bologna-5000"          
gateways = list()                           #list of the gateways



# we need to import some python modules from the $SUMO_HOME/tools directory
if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")


def cartesian_distance (x1, y1, x2, y2):
    diff_x = x2 - x1
    diff_y = y2 - y1
    distance = math.sqrt(diff_x**2 + diff_y**2)
    return distance

def get_options():
    opt_parser = optparse.OptionParser()
    opt_parser.add_option("--nogui", action="store_true",
                         default=False, help="run the commandline version of sumo")
    options, args = opt_parser.parse_args()
    return options


#returns the id of the closest gateway, -1 if no gateway in the range
def closest_gateway(veh_id):
    res = -1
    min_distance = float("inf") 
    x, y = traci.vehicle.getPosition(veh_id)
    for g in gateways:
        distance = cartesian_distance(int(x), int(y), g.x, g.y) 
        if (distance < min_distance):
            res = g.id
    return res
            

# contains TraCI control loop
def run():
    step = 0
    conn1 = traci.getConnection("sim1")

    while step < total_steps:
        counter=0       #lora vehicles counter
        detected_vehicles_counter =0
        transactionList = list() #list of tuples with the info about the transactions
        if (step % interaction_step == 0 and step > warmup_steps):
            # write the positions of the vehicles/sensors in the nodes files, the file will be used by simlorasf
            with open (nodesFile, 'a') as nf:
                for veh_id in traci.vehicle.getIDList():
                    if (traci.vehicle.getColor(veh_id) == (255, 255, 0, 255)):    #yellow, it's a new vehicle
                        if (traci.vehicle.getTypeID(veh_id) == "veh_passenger" and random.random() < loraVehiclePercentage): 
                            traci.vehicle.setColor(veh_id, (255, 0, 0, 255))      #red, vehicle with Lora sensors
                        else:
                            traci.vehicle.setColor(veh_id, (255, 0, 255, 255))    #purple, vehicle without Lora sensors
            
                    if (traci.vehicle.getTypeID(veh_id) == "veh_passenger" and traci.vehicle.getColor(veh_id) == (255, 0, 0, 255)):
                        counter+=1
                        gateway_id = closest_gateway(veh_id)
                        provider = random.randint (0, num_providers -1)    #temporarily, the connection between data and provider is random
                        nf.write(str(traci.vehicle.getPosition(veh_id)).replace('(', '').replace(')', '') + ", " + str(veh_id) +  ", " + str(gateway_id) + ", " + str(provider) + ", \n")
                        if (scenario == 1):
                            neighbors = traci.vehicle.getNeighbors(veh_id, 0), traci.vehicle.getNeighbors(veh_id, 1), traci.vehicle.getNeighbors(veh_id, 2), traci.vehicle.getNeighbors(veh_id, 3)
                            num_neighbors = len([t for t in neighbors if t])
                            detected_vehicles_counter += num_neighbors

                #for elem in transactionList:            #write the trasnmitted data in the transmission file
                #    tf.write(elem)

            #execute lorasimsf
            if scenario == 1:
                print ("\nDetected ", detected_vehicles_counter, " out of ", len (traci.vehicle.getIDList()), " vehicles at ", (step - warmup_steps))
            print (counter, " lora vehicles at ", step)
            os.system("python3 simlorasf/main.py -r " +  str(loraRange) + " -g " + str(len(gateways)) + " -n " + str(counter) + " -s SF_Lowest -d  20 -p 0.2 -z 60 -o 1 0")    
            with open (nodesFile, 'a') as f:            #after the file has been used by simlorasf to kwow the location of the sensors, then it is emptied
                f.truncate(0)       

            #execute lunes steps
            with open ("lunes/step" + str(step - warmup_steps) + ".txt", "w") as flagFile:             #data ready to be read
                flagFile.write ("OK")    

        conn1.simulationStep()
        step += 1
        time.sleep(pause_within_timesteps)

    traci.close()
    sys.stdout.flush()




# main entry point
if __name__ == "__main__":
    options = get_options()

    #eliminate all temporary files from previous executions
    for filename in os.listdir('lunes'):
        if filename.startswith('step'):
            file_path = os.path.join('lunes', filename)
            try:
                os.remove(file_path)
            except OSError as e:
                print(f"Error when deleting file {file_path}: {e}")

    with open (energyResFile, 'w') as f:
        f.write ("#energy c. (J)    #pdr    #deliveries   #packets   #throughput \n")

    if os.path.exists (transmissionsFile):
        os.remove(transmissionsFile)

    #load and count the gateways
    gateway_counter=0
    with open (gatewayFile, "r") as gfile:
        for line in gfile:          # read gateway positions and fill up the list of gateways
            x, y = map(int, line.strip().split(',')) 
            gateways.append(Gateway(x, y, gateway_counter))
            gateway_counter +=1

    #initialize lunes
    os.chdir ("lunes")
    os.system ("./run -p " + str(num_providers) + " -g " + str(gateway_counter) + " -s " + str(actual_steps) + " &")
    os.chdir ("..")

    # check binary
    if options.nogui:
        sumoBinary = checkBinary('sumo')
    else:
        sumoBinary = checkBinary('sumo-gui')

    # traci starts sumo as a subprocess and then this script connects and runs
    traci.start([sumoBinary, "-c", osmLocation + "/osm.sumocfg", "--tripinfo-output", osmLocation + "/tripinfo.xml"], label = "sim1")
    run()
