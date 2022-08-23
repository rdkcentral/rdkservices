# SwitchBoard Plugin #

This plugin is a loose adaptation of Switchboard ThunderNanoService. The plugins that needs to be offloaded as well as the memory configurations is configuratable.

## Configurable parameters ##
The following values can be configured at runtime.

| Parameter | Description |
| --------- | ----------- | 
| homeurl | The ResidentApp Home URL |
| lowmem  | Limit set for Low memory event trigger in MB. |
| criticalmem | Limit set for Critical memory event trigger in MB. |
| callsigns | An array of plugins/applications that can be offloaded. |

## Things to do #

The following are planned in the next phase.
* Automatic loading of offloaded application back when memory is available.
