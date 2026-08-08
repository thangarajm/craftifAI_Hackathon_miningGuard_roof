# craftifAI_Hackathon_miningGuard_roof
Single ESP32-C3 + HC-SR04 ultrasonic mine roof stability monitor. Detects gradual roof creep vs sudden collapse using baseline + rate-of-change logic, alerts via onboard RGB LED. Low-cost (&lt;Rs 400) alternative to lakh-rupee geotechnical monitoring for Indian mine safety. Built with FirmGen.

Later this can be communicated via LoRa mesh, the packet hops until it reaches the Anchor (Hub)

Im unable to use LoRa because i dont have the module at the moment.

Also RSSI can be used to detect if there are miners present during collapse.


