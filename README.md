# NEMU-Of-IA32

天津大学计算机系统综合实践——面向IA-32的模拟器“NEMU”设计


为方便构建本地调试环境，本项目构建了基于ubuntu18.04的docker镜像


**构建 Docker 镜像**

```
docker build -t mynemu .
```


**运行镜像**

```
docker run mynemu:latest
```


**进入容器终端**

```
docker run -it mynemu:latest /bin/bash
```
