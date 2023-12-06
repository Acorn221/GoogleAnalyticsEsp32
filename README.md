# Google Analytics Esp32

For the LILYGO E-Paper display, T5 v2.3.

This project displays the Google Analytics data from my chrome extension [LighterFuel](https://github.com/J4A-Industries/LighterFuel-For-Tinder).

LighterFuel uses the GA4 measurement protocol, as that's the only way Mv3 extensions can send data to GA.

![Image](https://github.com/Acorn221/GoogleAnalyticsEsp32/blob/master/20231206_165240.jpg?raw=true)

## Setup

The setup requires you get a refresh token from Google, from an oauth2 flow with the scope `https://www.googleapis.com/auth/analytics.readonly`.
