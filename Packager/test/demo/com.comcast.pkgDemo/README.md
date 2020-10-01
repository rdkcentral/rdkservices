
# Packager demo app

This project demonstrates the use of the *Packager* plugin for application/packager installation.

The project is JS and uses `npm`.

## Setup

Install `node_modules` used in this project in the usual way.

```
npm install
```

Install *Lightning CLI* globally as follows.

```
npm install -g rdkcentral/Lightning-CLI
```

## Build

Use the following to build this Lightning++ app...

```
lng build

lng dist
```


## Running

Example usage...

```http://127.0.0.1/?appList=http://127.0.0.1/AppList.json&thunderCfg=http://127.0.0.1/ThunderCfg.json```

URL Paramaters -

  * `appList`    ... the URL for the "App Store" inventory JSON file
  * `thunderCfg` ... the URL for the "Thunder Frmawork" configuration JSON file

Development testing used a Vagrant VM and port forwarding...

          Host OS      <----->      Vagrant VM

  *  Thunder / `WPEFramework` is running the VM ... providing *Packager* plugin

  *  The `Demo App` is hosted on the HOST using ...

      ``` http-server -p 80 -c-1 ```

  *  The "App Store Inventory" is hosted on the VM, again using ...

      ``` http-server -p 80 -c-1 ```

## Creative Commons / Attribution

Some of the images used in this demo are CC Licensed.

Attributions to the following -

  *  https://commons.wikimedia.org/wiki/File:Eo_circle_light-green_white_checkmark.svg

## Author

[Hugh Fitzpatrick]()  - September 2020
