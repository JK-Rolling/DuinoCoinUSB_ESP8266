# DuinoCoinUSB_ESP8266
This project design to mine [Duino-Coin](https://github.com/revoxhere/duino-coin) using an ESP8266 via USB-Serial


## Python Environment Setup

### Linux

```BASH
sudo apt update
sudo apt install python3 python3-pip git screen -y # Install dependencies
git clone https://github.com/JK-Rolling/DuinoCoinUSB_ESP8266.git # Clone DuinoCoinUSB_ESP8266 repository
cd DuinoCoinUSB_ESP8266
python3 -m pip install -r requirements.txt # Install pip dependencies
````

Finally, connect your USB ESP8266 miner and launch the software (e.g. `python3 ./ESP8266_Miner.py`)

### Windows

1. Download and install [Python 3](https://www.python.org/downloads/) (add Python and Pip to Windows PATH)
2. Download [DuinoCoinUSB_ESP8266](https://github.com/JK-Rolling/DuinoCoinUSB_ESP8266/releases)
3. Extract the zip archive you've downloaded and open the folder in command prompt
4. In command prompt type `py -m pip install -r requirements.txt` to install required pip dependencies

Finally, connect your USB ESP8266 miner and launch the software (e.g. `python3 ./ESP8266_Miner.py` or `py ESP8266_Miner.py` in the command prompt)

## Version

ESP8266_Miner Version 4.3


# License and Terms of service

All refers back to original [Duino-Coin licensee and terms of service](https://github.com/revoxhere/duino-coin)
