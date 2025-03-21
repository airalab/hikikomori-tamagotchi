# Example 3 - Button Press Counter

In this example, the Hikikomori Tamagotchi will count the number of times you press the button and send a Datalog with the total count.

## Setting Up

After flashing, the Hikikomory device will enter setup mode. You’ll see the following screen:  

<img src="../images/hiki1.jpg" alt="hiki1" width="300"/>

1. **Connect to the Access Point**  
   The device will create a Wi-Fi access point named `Hikkikomory-Tamagotchi`. Connect to this network.  

2. **Open the Web Interface**  
   In your browser, navigate to `192.168.4.1`. Fill in your WiFi network credentials.

3. **Setup Your Device**  
   At the bottom of the page, you’ll find your device's address in the Robonomics Network. To send transactions it must have XRT tokens on its balance.  

   <img src="../images/hiki-web.png" alt="hiki-web" width="600"/>

4. **Save Settings**  
   Press the `Save` button to complete the setup.


## Using

The Hikikomory device can count button presses and send Datalog transactions with messages like `Button was pressed n times`.

1. **Increase the Counter**  
   A short button press increments the counter.  

   <img src="../images/hiki2.jpg" alt="hiki2" width="300"/>

2. **Send a Datalog Transaction**  
   Hold the button for 2–3 seconds to send a Datalog transaction. Afterward, the device will display the transaction hash and a QR code linking to the transaction on [Subscan](https://robonomics.subscan.io).  

   <img src="../images/hiki3.jpg" alt="hiki3" width="300"/>

3. **Turn Off the Device**  
   To put the device into deep sleep mode, quickly double-press the left button. To turn it back on, simply press the right button.