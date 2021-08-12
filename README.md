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

Test Drone Simulation (Single Run Exemple)

```
sudo docker run --rm -it px4-sitl-headless-flex -sysid 11 -speed 1\  
        -gip 192.168.0.6 -gport 14550 -aip 192.168.0.6 -aport 14542\  
        -param "MPC_XY_CRUISE 20" -param "MIS_DIST_1WP 3000"\  
        -param "MPC_XY_VEL_MAX 20" -param "MIS_DIST_WPS 3000"
```
Change the -gip to the IP of your ground station host (for example QGroundControl) to connect to the simulated drone.  
See more options in https://github.com/andrekuros/px4-sitl-headless-flex.   

### Run your first Simulation





