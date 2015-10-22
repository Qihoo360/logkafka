## install libuv


For Centos, just run `sudo yum install -y libuv`
  
Or, build from source
  
```
cd /tmp
git clone https://github.com/libuv/libuv.git
cd libuv
sh autogen.sh
./configure
make
sudo make install
```
