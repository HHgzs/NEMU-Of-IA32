FROM nemu:latest

WORKDIR /NEMU

COPY . /NEMU

RUN apt-get update && apt-get install -y \
    bsdmainutils \
    && git config --global user.email "3166237657@qq.com"  \
    && git config --global user.name "HHgzs" \
    # && cd /NEMU && make clean && make
# RUN apt-get update && apt-get install -y \
#     build-essential \
#     gcc-doc \
#     gdb \
#     git \
#     time \
#     libreadline-dev \
#     libsdl-dev \
#     vim \
#     ctags \
#     tmux \
#     python-software-properties \
#     && add-apt-repository 'deb http://archive.ubuntu.com/ubuntu/ trusty main' \
#     && add-apt-repository 'deb http://archive.ubuntu.com/ubuntu/ trusty universe' \
#     && apt-get update \
#     && apt-get install -y gcc-4.4 gcc-4.4-multilib 
#     && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.4 100
