#Simple Test 01 - 11/08/2021
#Andre Kuros

from mininet.log import lg, info, setLogLevel
from mininet.net import Mininet, Containernet
from mininet.topolib import TreeNet, TreeContainerNet
from mininet.node import Controller
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.nodelib import NAT


sitlImage="andrekuros/px4-sitl-headless-flex"
simSpeed= "1"
gcsIp= "10.0.0.99"
mavsdkIp="10.0.0.99"
params  = " --param \"MPC_XY_CRUISE 20\" --param \"MIS_DIST_1WP 3000\" --param \"MPC_XY_VEL_MAX 20\" --param \"MIS_DIST_WPS 3000\""#--param \"IMU_INTEG_RATE 10\""

numCompanies = 3
dronesInCompanies = [1,1,1]
delaysCompaniesUAVs = ['0ms', '0ms', '0ms']
delaysCompaniesSwitch = ['0ms', '0ms', '0ms']
bwCompanies = [1000,1000,1000]

lg.setLogLevel('info')

net = Containernet( topo=None)
net.addController('c0')
s1 = net.addSwitch("s1")

#h_gcs = net.addDocker("h_gcs", ip="10.0.0.99", dimage="mavsdk-kuros-gcs" , network_mode='none',  dcmd="/bin/bash", volumes=["shareAll:/shareAll"])
#net.addLink(h_gcs,s1,cls=TCLink, delay='0ms', bw=1000)

#Create drones as hosts
for c in range(1, numCompanies + 1):
    sc = net.addSwitch("sc" + str(c))    
    net.addLink(sc,s1,cls=TCLink, delay=delaysCompaniesSwitch[c-1], bw=1000)
    for h in range(dronesInCompanies[c-1]):
        h_new = net.addDocker("c" + str(c) + "-n" + str(h) ,dimage=sitlImage, network_mode='none', dcmd="/bin/bash /root/entrypoint.sh --sysid " + str(10*c + h) +  " --gip " + gcsIp + " --gport 1455" + str(c-1) + " --aip " + mavsdkIp + " --aport 1454" + str(c-1) + " --speed " + simSpeed + params )
        net.addLink(h_new,sc,cls=TCLink, delay=delaysCompaniesUAVs[c-1], bw=bwCompanies[c-1])

net.build()

# Add NAT connectivity if necessary
#net.addNAT().configDefault()

#start Network
net.start()
CLI( net )
net.stop()

