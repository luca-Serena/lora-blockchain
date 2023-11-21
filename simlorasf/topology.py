#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2019  Tugrul Yatagan <tugrulyatagan@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; If not, see <http://www.gnu.org/licenses/>.

import random
import math
from location import Location
from node import Node
from node import Gateway
from node import TrafficType
from packet import Packet
import os
from math import radians, sin, cos, sqrt, atan2


class Topology:
    def __init__(self):
        self.gateway_list = []
        self.node_list = []
        self.radius = 0

    def get_node(self, id):
        return self.node_list[id - len(self.gateway_list) - 1]

    def get_gateway(self, id):
        return self.gateway_list[id - 1]

    def show(self):
        print('Nodes:')
        for gateway in self.gateway_list:
            print(' {}'.format(gateway))
        for node in self.node_list:
            print(' {}'.format(node))

    def get_get_nearest_gw(self, location):
        nearestGateway = None
        nearestDistance = None
        for gateway in self.gateway_list:
            distance = Location.get_distance(gateway.location, location)
            if nearestGateway is None:
                nearestGateway = gateway
                nearestDistance = distance
            elif distance < nearestDistance:
                nearestGateway = gateway
                nearestDistance = distance
        return nearestGateway, nearestDistance



    def create_topology(number_of_nodes, node_traffic_proportions, radius, number_of_gws):
        assert 1 <= number_of_nodes <= 100000, 'unsupported number of nodes {}'.format(number_of_nodes)
        topology = Topology()
        topology.radius = radius

        with open ("gt.txt", 'r') as f:
            for line in f.readlines():
                topology.gateway_list.append(Gateway(location=Location(float(line.split(", ")[0]), float(line.split(", ")[1]) )))
        
        with open ("nodes.txt", 'r') as f:
            for line in f.readlines():
                args = line.split(", ")
                topology.node_list.append(Node(location=Location(float(args[0]), float(args[1])), name=args[2], id_closest_gateway=args[3], provider=args[4] ))
        
           # Assign traffic generation types
        for type_index, proportion in enumerate(node_traffic_proportions):
            assigned = 0
            while assigned < math.floor(proportion * number_of_nodes):
                randomIndex = random.randint(0, number_of_nodes - 1)
                if topology.node_list[randomIndex].trafficType is None:
                    topology.node_list[randomIndex].trafficType = TrafficType(type_index)
                    assigned = assigned + 1
        for tx_node in topology.node_list:
            if tx_node.trafficType is None:
                tx_node.trafficType = TrafficType.Poisson

        # Find the lowest SF for nodes
        for tx_node in topology.node_list:
            _, nearestDistance = topology.get_get_nearest_gw(tx_node.location)
            tx_node.lowestSf = Packet.get_lowest_sf(distance=nearestDistance)

        return topology