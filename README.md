# ESP8266 433/315Mhz MQTT over WiFi bridge

This MQTT bridge allows you to control cheap home-automation RF power outlets using a common MQTT interface.
With e.g. [Home-assistant](https://github.com/home-assistant/home-assistant), these power outlets can then be switched on/off remotely over the internet and integrate seamlessly with IFTT.

## MQTT auto-discovery
The MQTT broker is automatically discovered through mDNS. When you do not want to use this, simply put the static IP or hostname of your MQTT broker in the `hostString` variable.

To enable MQTT discovery on the broker, simply install `avahi-daemon`. For a Raspberry Pi, use the following command:
```bash
sudo apt-get install avahi-daemon
```
Next, create the following service declaration in `/etc/avahi/services/mqtt.service`:
```xml
<service-group>
  <name replace-wildcards="yes">MQTT on %h</name>
  <service>
    <type>_mqtt._tcp</type>
    <port>1883</port>
  </service>
</service-group>
```
Restart your avahi-daemon service: `sudo service avahi-daemon restart`.

_Please note that discovery over mDNS only works within the same multicast/broadcast domain and cannot cross network interfaces, unless a mDNS-repeater or igmp-proxy is used._

## MQTT protocol
The following MQTT payload should be sent:
- **Topic**: `/switch/rf/[ID]`
    - e.g. `/switch/rf/5`
- **Payload**: `[protocol]|[pulselength]|[binary code]` 
    - e.g. `4|101|101010100100101100101100`

These protocol, pulselength and binary variables can be sniffed as described in [Obtaining RF codes](#obtaining-rf-codes).

Remember to modify lines 12 & 13 with your MQTT broker's username and password, or uncomment line 135 and comment line 136 if you have no credentials needed for your MQTT broker.

For persistency, the _retain-flag_ can be set such that the codes are re-sent when the ESP8266 is rebooted.

## Code setup

* Get the Arduino ESP8266 firmware. For install instructions please check [here](https://github.com/esp8266/Arduino).
* Install the Arduino [RC-switch](https://github.com/sui77/rc-switch) library
* Install the Arduino [PubSubClient](https://github.com/knolleary/pubsubclient) library
* Configure your wireless password in the `WIFI_SSID` and `WIFI_PASSWORD` variables

## Wiring
The firmware was tested on a NodeMCU development board.
The 433/315 transmitter data pin was connected to D2 pin (GPIO4), but any other gpio can be used. Most transmitters can be powered using 3.3V directly.

## Obtaining RF codes
The RF codes can be sniffed using the [RC-switch examples](https://github.com/sui77/rc-switch/tree/master/examples/ReceiveDemo_Advanced) using an Arduino or ESP8266 as well. Keep in mind that there might be a difference measured in pulselength depending on the device, since rc-switch (currently) does not adjust for clock speed. It is therefore advisable to also use an ESP8266 when sniffing the codes.

## Home Assistant example
Example configuration in Home Assistant
```yaml
# Broker config
mqtt:
  broker: 127.0.0.1
  port: 1883
  client_id: home-assistant-1
  keepalive: 60

# RF switch
switch:
  - platform: mqtt
    name: "Bedroom light"
    command_topic: "switch/rf/1"
    payload_on: "4|101|101000110001100001101100"
    payload_off: "4|101|101010100100101100101100"
    optimistic: false
    qos: 0
    retain: true
```

## License
The code falls under the MIT license.