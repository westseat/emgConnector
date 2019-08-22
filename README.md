## Brief

emgConnect is the communication agent between the EMG sensor and EMG client.

Currently, the EMG client is just used to dispaly the emg waves.

## Install Dependency
> Note: Before running the script, Please make sure the `python3` version >= 3.6

```sh
$ pip3 install -r requirements.txt
```

## Run

Since the script need to access the USB interface, please run the script in the `root` user.

```sh
$ python3 emgServer.py
```

