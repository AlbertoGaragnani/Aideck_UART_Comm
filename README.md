# UART communication on Crazyflie expansion deck

This is an example on how to use the UART communication protocol to send data from the Aideck 1.1 to the STM32 processor on the Crazyflie 2.1 drone.
The module has been developed on Ubuntu 20.04 LTS and had been tested using an oscilloscope.

To get started with the example, you need to configure your Crazyflie with the Aideck 1.1: https://www.bitcraze.io/documentation/tutorials/getting-started-with-aideck/
If you installed the docker correctly, you can build the example on the Crazyflie with the following commands:

```
cd Aideck_UART_Comm
sudo docker run --rm -v ${PWD}:/module aideck-with-autotiler tools/build/make-example UART_rw image
```

Flashing the example using the cfclient as stated on the official website won't work, you can use a JTAG instead. You will need to set the JTAG configuration based on you JTAG model in tools -> make-example modifing the following line:

```
export GAPY_OPENOCD_CABLE=interface/ftdi/olimex-arm-usb-ocd-h.cfg
```

setting your own cfg file.

You can now flash the example with the following command:

```
cd Aideck_UART_Comm
sudo docker run --rm -v ${PWD}:/module --device /dev/ttyUSB0 --privileged -P aideck-with-autotiler tools/build/make-example UART_rw flash
```

Once you flashed the module, restart the Crazyflie and you should see the m1 led on.

The communication has been tested using an oscilloscope.
