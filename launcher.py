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


interaction_step=10
total_steps = 1000
loraVehiclePercentage= 0.8
loraRange = 5000
gatewayFile="gt.txt"
nodesFile = "nodes.txt"
gateways = list()


# we need to import some python modules from the $SUMO_HOME/tools directory
if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")


def cartesian_distance (x1, y1, x2, y2):
    # Calcola la differenza delle coordinate x e y
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
def gateways_in_range (veh_id):
    res = -1
    min_distance = loraRange 
    x, y = traci.vehicle.getPosition(veh_id)
    for g in gateways:
        distance = cartesian_distance(int(x), int(y), g.x, g.y) 
        if (distance < min_distance):
            res = g.id
    return res
            


# contains TraCI control loop
def run():
    gateway_counter=0
    with open (gatewayFile, "r") as gfile:
        for line in gfile:          # read gateway positions and fill up the list of gateways
            x, y = map(int, line.strip().split(',')) 
            gateways.append(Gateway(x, y, gateway_counter))
            gateway_counter +=1

    step = 0
    conn1 = traci.getConnection("sim1")

    while step < total_steps:
        counter=0       #lora vehicles counter
        for veh_id in traci.vehicle.getIDList():
            if (traci.vehicle.getColor(veh_id) == (255, 255, 0, 255)):    #yellow, it's a new vehicle
                #print ("new vehicle ", traci.vehicle.getTypeID(veh_id),  "  ", step, "  ", traci.vehicle.getPosition(veh_id))
                if (traci.vehicle.getTypeID(veh_id) == "veh_passenger" and random.random() < loraVehiclePercentage): 
                    traci.vehicle.setColor(veh_id, (255, 0, 0, 255))      #red, vehicle with Lora sensors
                else:
                    traci.vehicle.setColor(veh_id, (255, 0, 255, 255))    #purple, vehicle without Lora sensors
            
            # write the positions of the vehicles/sensors in the nodes files, the file will be used by simlorasf
            if (step % interaction_step == 0):
                with open (nodesFile, 'a') as nf,  open ("lunes/step" + str(step) + ".txt", "w") as file:
                    if (traci.vehicle.getTypeID(veh_id) == "veh_passenger" and traci.vehicle.getColor(veh_id) == (255, 0, 0, 255)):
                        nf.write(str(traci.vehicle.getPosition(veh_id)).replace('(', '').replace(')', '') + ",  " + str(step) +"\n")
                        gateway_id = gateways_in_range(veh_id)
                        if (gateway_id >= 0):
                            file.write (str(veh_id) + " " + str(gateway_id) + " " + str(step))
                        counter+=1

        #execute lorasimsf
        if (step % interaction_step == 0 and counter > 0 ):
            print (counter, " lora vehicles at ", step)
            os.system("python3 simlorasf/main.py -r 5000 -g " + str(len(gateways)) + " -n " + str(counter) + " -s SF_Lowest -d  20 -p 0.2 -z 60 -o 1 0")    
            with open (nodesFile, 'a') as f:            #after the file has been used by simlorasf to kwow the location of the sensors, then it is emptied
                f.truncate(0)           

        conn1.simulationStep()
        step += 1
        time.sleep(0.2)

    traci.close()
    sys.stdout.flush()


# main entry point
if __name__ == "__main__":
    options = get_options()

    #initialize lunes
    os.chdir ("lunes")
    os.system ("./run &")
    os.chdir ("..")

    # check binary
    if options.nogui:
        sumoBinary = checkBinary('sumo')
    else:
        sumoBinary = checkBinary('sumo-gui')

    # traci starts sumo as a subprocess and then this script connects and runs
    traci.start([sumoBinary, "-c", "osm.sumocfg", "--tripinfo-output", "tripinfo.xml"], label = "sim1")
    run()
