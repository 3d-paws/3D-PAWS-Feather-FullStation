# LoRaWAN Private Network Direction
[←Top](../README.md)<BR>

Setting up a LoRaWAN private network involves configuring LoRaWAN Gateways to forward LoRaWAN messages to a private LoRaWAN network server (also known as a LoRaWAN Network Server or LNS). This allows you to bypass public platforms like The Things Network. By self-hosting, you achieve total control over your device data, zero subscription fees, and the ability to operate entirely offline or on an isolated local network

### Top Private Server Software Options

[ChirpStack](https://www.chirpstack.io/): Popular open-source stack that combines the network server and application server in a web interface. It is modular and can be deployed with Docker on Linux or a Raspberry Pi.

[The Things Stack Sandbox / Enterprise](https://www.thethingsindustries.com/docs/concepts/): Enterprise-grade LoRaWAN server software with both network server and application server functions. The Sandbox is a free, limited version for testing, while the open-source core is available for DIY use.

[Gotthardp LoRaWAN Server](https://gotthardp.github.io/lorawan-server/): Lightweight all-in-one LoRaWAN server written in Erlang, aimed at small private LoRa networks and resource-constrained edge deployments.

### Deployment Architecture Options

Depending on your hardware availability and scale, you can deploy a private server using three major methods:

On-Gateway Execution: Installing software like ChirpStack directly onto an open Linux-based gateway. Manufacturers like DFRobot offer built-in local servers with Node-RED out of the box. This is best for single-gateway setups but does not scale well to multi-gateway deployments.

Local Server / On-Premise VM: Running a dedicated Ubuntu machine or a local server instance. Gateways forward raw radio packets over the LAN to a central machine, keeping the deployment isolated from the public internet.

Private Cloud Instance: Deploying the LNS stack inside an isolated cloud environment like AWS EC2. You can also use AWS IoT Core for LoRaWAN, which is a fully managed LNS and can be connected through VPC interface endpoints for private connectivity.

### Basic Setup Requirements

To successfully launch your local network, you will need to prepare four core items:

- The Server Environment: A Linux environment running Docker and MQTT to orchestrate the software. ChirpStack deployments commonly include MQTT, PostgreSQL, and related services in a Docker-based stack.
- LoRaWAN Gateway: A physical gateway configured with a packet forwarder pointing to your private server's IP address. Packet forwarders send received RF packets over an IP/UDP link to the server.
- End Devices: Sensors using identical regional frequency parameters, such as US915 or EU868, matching the gateway and network settings.
- Data Integrations: Databases or visualization tools like PostgreSQL, InfluxDB, or Grafana to read and display payload outputs.