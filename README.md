# geode-firmware

This project contains the firmware for Opal Instrument's Geode Timer Module.

## getting started

### Vagrant

This project can be developed on a Virtual Machine, which can be provisioned by [Vagrant](https://www.vagrantup.com/).  The Vagrant setup for this project is currently configured to be provisioned by the [Ansible](http://www.ansible.com/) orchaestration tool.

To get started, please be sure to install both [Vagrant](https://www.vagrantup.com/) and [Ansible](http://www.ansible.com/), ensure they are both accessible in your `$PATH`, and then run the following command:

```bash
$ vagrant up
```

This will provision a development box on which you can freely develop the firmware for the Geode.