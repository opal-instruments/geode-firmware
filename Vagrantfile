Vagrant.configure(2) do |config|
  config.vm.box = "hashicorp/precise64"

  config.ssh.forward_agent = true
  config.vm.network "forwarded_port", guest: 80, host: 8080
  config.vm.network "private_network", ip: "192.168.33.10"
  config.vm.network "public_network"
  config.vm.synced_folder ".", "/vagrant"
  
  config.vm.provider "virtualbox" do |vb|
    vb.customize ['modifyvm', :id, '--usb', 'on']

    # Configure for avrdude programmer
    vb.customize ['usbfilter', 'add', '0', '--target', :id, '--name', 'AVR ISPmkII Programmer', '--vendorid', '0x03eb', '--productid', '0x2ffa']
  end

   config.vm.provision "ansible" do |ansible|
     ansible.playbook = "_ansible/dev.yml"
     ansible.inventory_path = "_ansible/inventory"

     # Taken from: https://github.com/mitchellh/vagrant/issues/3096
     ansible.limit = "all"
   end
end
