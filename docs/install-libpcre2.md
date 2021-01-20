## install libpcre2

For Mac:
```
brew install pcre2
```

For linux:
```
cd /tmp
wget -N http://sourceforge.net/projects/pcre/files/pcre2/10.20/pcre2-10.20.tar.gz; tar zxvf pcre2-10.20.tar.gz; rm -f pcre2-10.20.tar.gz
cd pcre2-10.20
./configure
make -j4
sudo make install
```
Any problems, check [this](http://www.pcre.org/)
