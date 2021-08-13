# UTM Multi-Features Docker Based TestBed

This is an academic project that aims to implement a docker based simulation infraesctruture that allow to represent multiple features of an Unmanned Aircraft System Traffic Management (UTM), from the low level drone control system up to the airspace management algorithm.    

The proposed TestBed is under construction and the current version was built througt the integration of some OpenSource projects, as presented below:
   
![image](https://user-images.githubusercontent.com/18732953/129098862-49636de7-294f-438e-a099-0c933ce92538.png)

# Reproducing the TestBed

All tests were done on Ubuntu 20.04 LTS for the infraestruture and Windows 10 for the visualization tool.

## Building the Infraestrucuture

The base for the resourses for the TestBed are:   
   
DOCKER CONTAINERS: https://www.docker.com/  
CONTAINERNET (MININET): https://containernet.github.io/ 
MAVSDK: https://github.com/mavlink/MAVSDK   
PX4 AUTOPILOT: https://github.com/PX4/PX4-Autopilot   
imGUI: https://github.com/ocornut/imgui   
   
### STEP 1: Install DOCKER

REF: https://docs.docker.com/engine/install/ubuntu/

```
sudo apt-get update
sudo apt-get install docker-ce docker-ce-cli containerd.io
```
   
### STEP 2: Install CONTAINERNET

REF: https://containernet.github.io/#installation

```
sudo apt-get install ansible git aptitude
git clone https://github.com/containernet/containernet.git
cd containernet/ansible
sudo ansible-playbook -i "localhost," -c local install.yml
cd ..
sudo make develop
```

Test CONTAINERNET
```
# run interactive container and directly start containernet example
docker run --name containernet -it --rm --privileged --pid='host' -v /var/run/docker.sock:/var/run/docker.sock containernet/containernet
```

### STEP 3: Get PX4 SITL HEADLESS FLEX

The Drone simulation solution selected for this first version of this TestBed was the SITL from the PX4 Autopilot (https://px4.io/) using Gazebo (http://gazebosim.org/) simulation. To make it possible, we start from the image px4-gazebo-headless (https://github.com/JonasVautherin/px4-gazebo-headless), but we need to make some modifications to have enough flexibility. The new container project created aims to add more flexibility to change the SITL params when running docker image with the lastest version of PX4 and to allow inicializations in different conditions and configurations, becoming more suitable for a wider range of applications (https://hub.docker.com/r/andrekuros/px4-sitl-headless-flex).

Getting px4-headless-flex

```docker pull andrekuros/px4-sitl-headless-flex```

Test Drone Simulation (Single Run Example)

```
sudo docker run --rm -it px4-sitl-headless-flex -sysid 11 -speed 1\  
        -gip 192.168.0.6 -gport 14550 -aip 192.168.0.6 -aport 14542\  
        -param "MPC_XY_CRUISE 20" -param "MIS_DIST_1WP 3000"\  
        -param "MPC_XY_VEL_MAX 20" -param "MIS_DIST_WPS 3000"
```
Change the -gip to the IP of your ground station host (for example QGroundControl) to connect to the simulated drone.  
See more options in https://github.com/andrekuros/px4-sitl-headless-flex.   

## Run your first Simulation

Using the ContainerNet you can start a simulated Mininet Network with multiple drones throught an Python. To do this first run the Containernet Docker Image an then run the python script.

```
sudo docker run --name containernet -it --rm --privileged --pid='host' -v /home/user/share:/containernet/examples -v /var/run/docker.sock:/var/run/docker.sock containernet/containernet  /bin/bash   

containernet> python3 test1.py
```

```
#Case: Initial Test - 03/07/2021
#test1.py
#Andre Kuros

from mininet.log import lg, info
from mininet.net import Mininet, Containernet
from mininet.topolib import TreeNet, TreeContainerNet
from mininet.node import Controller
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.log import info, setLogLevel
from mininet.nodelib import NAT


sitlImage="andrekuros/px4-sitl-headless-flex"
simSpeed= "1"
gcsIp= "10.0.0.99"
mavsdkIp="10.0.0.99"
params  = " --param \"MPC_XY_CRUISE 20\" --param \"MIS_DIST_1WP 3000\" --param \"MPC_XY_VEL_MAX 20\" --param \"MIS_DIST_WPS 3000\""#--param \"IMU_INTEG_RATE 10\""

numCompanies = 3
dronesInCompanies = [3,3,3]
delaysCompaniesUAVs = ['0ms', '0ms', '0ms']
delaysCompaniesSwitch = ['0ms', '0ms', '0ms']
bwCompanies = [1000,1000,1000]

lg.setLogLevel('info')

net = Containernet( topo=None)
net.addController('c0')
s1 = net.addSwitch("s1")

h_gcs = net.addDocker("h_gcs", ip="10.0.0.99", dimage="mavsdk-kuros-gcs" , network_mode='none',  dcmd="/bin/bash", volumes=["shareAll:/shareAll"])
net.addLink(h_gcs,s1,cls=TCLink, delay='0ms', bw=1000)

#Create drones as hosts
for c in range(1, numCompanies + 1):
    sc = net.addSwitch("sc" + str(c))    
    net.addLink(sc,s1,cls=TCLink, delay=delaysCompaniesSwitch[c-1], bw=1000)
    for h in range(dronesInCompanies[c-1]):
        h_new = net.addDocker("c" + str(c) + "-n" + str(h) ,dimage=sitlImage, network_mode='none', dcmd="/bin/bash /root/entrypoint.sh --sysid " + str(10*c + h) +  " --gip " + gcsIp + " --gport 1455" + str(c-1) + " --aip " + mavsdkIp + " --aport 1454" + str(c-1) + " --speed " + simSpeed + params )
        net.addLink(h_new,sc,cls=TCLink, delay=delaysCompaniesUAVs[c-1], bw=bwCompanies[c-1])

net.build()

# Add NAT connectivity
#net.addNAT().configDefault()

#start Network
net.start()
CLI( net )

# Shut down network
net.stop()
```
<hr>

# The UTM Controller

The UTM Controller is the solution created to allow to test algorithms to control the Airspace, receiving informations from all drones in the simulation and sending commands and authorizations. The solution is divided in two main modules, the GCS_UTM and the GridView. The GCS_UTM has the background to communicate with the drones and the interfaces to interact with them. The GridView is an solution to allow the visualization of the Airspace and add the possibility to interact in realtime. 

## Using the GCS_UTM

The current version of the GCS_UTM was tested Windows or Linux and can be started inside or outside of the Mininet, allowing to monitore and control the drones from an external host. 

### GCS_UTM on Windows



