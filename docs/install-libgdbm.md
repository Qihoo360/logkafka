## install libgdbm


For Centos, just run `sudo yum install -y gdbm`
  
Or, build from source
  
```
cd /tmp
wget -N -O- http://ftp.gnu.org/gnu/gdbm/gdbm-1.8.3.tar.gz | tar zxvf -
cd gdbm-1.8.3
./configure
make -j4
sudo make BINOWN=root BINGRP=root install-compat
```
