# quartz-firmware

This project contains the firmware for Opal Instrument's Quartz Timer Module.

## getting started

### Vagrant

This project can be developed on a Virtual Machine, but it is not required.  The virtual machine is an ubuntu instance that has the `avr-gcc` toolchain installed that is required to build and develop this project.

The VM for this project is managed by [Vagrant](https://www.vagrantup.com/).  The Vagrant setup for this project is currently configured to be provisioned by the [Ansible](http://www.ansible.com/) orchaestration tool.

To get started, please be sure to install both [Vagrant](https://www.vagrantup.com/) and [Ansible](http://www.ansible.com/), ensure they are both accessible in your `$PATH`, and then run the following command:

```bash
$ vagrant up
```

This will provision a development box on which you can freely develop the firmware for the Quartz.

### Building the project

This project currently uses a `Makefile` to build itself.  There are a few helpful directives that you might be intersted in knowing about:

  - `make clean` : Cleans the `bin`, `vendor/build`, and `vendor/libs` directories.
  - `make fmt` : Runs [`astyle`](http://astyle.sourceforge.net/astyle.html) on all source files.
  - `make deps` : Builds all dependent libraries and places them in `vendor/libs` for building.
  - `make` : Performs all of the previous options, then builds the project.
  - `make upload` : Uploads the resulting `.hex` to the a connceted Quartz board via a AVRmkII ISP programmer.

In most cases, the following workflow should be sufficient to upload a new build of the project:

```bash
$ make clean
$ make
$ make upload
```