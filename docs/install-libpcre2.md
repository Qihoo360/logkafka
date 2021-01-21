## install libpcre2

For Mac:
```
brew install pcre2
```

For linux:
```
cd /tmp
wget -N -0- http://sourceforge.net/projects/pcre/files/pcre2/10.20/pcre2-10.20.tar.gz | tar zxvf -
cd pcre2-10.20
./configure
make -j4
sudo make install
```
Any problems, check [this](http://www.pcre.org/)
