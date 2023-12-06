# Google Analytics Esp32

For the LILYGO E-Paper display, T5 v2.3.

This project displays the Google Analytics data from my chrome extension [LighterFuel](https://github.com/J4A-Industries/LighterFuel-For-Tinder).

LighterFuel uses the GA4 measurement protocol, as that's the only way Mv3 extensions can send data to GA.

![Image](https://github.com/Acorn221/GoogleAnalyticsEsp32/blob/master/20231206_165240.jpg?raw=true)

## Setup

The setup requires you get a refresh token from Google, from an oauth2 flow with the scope `https://www.googleapis.com/auth/analytics.readonly`.

Get your refresh token from Google [here](https://accounts.google.com/o/oauth2/v2/auth?redirect_uri=https%3A%2F%2Fdevelopers.google.com%2Foauthplayground&prompt=consent&response_type=code&client_id=407408718192.apps.googleusercontent.com&scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fanalytics.readonly&access_type=offline) (only valid for 7 days, for a permanant refresh token, you need to create your own project on [the google cloud console](https://console.cloud.google.com) and click publish on the oauth) 
