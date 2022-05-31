# Video Streamer with Load-balancing Server

notion 회의록입니다.
https://jonjjon.notion.site/2-127d5f99cf6e4bf3b234dd995fc69b9d
## 1. 설치 및 실행 방법

### 1.1 scratch/streaming 경로에 아래 파일 복사
- ```client.cc```
- ```client.h```
- ```client-helper.cc```
- ```client-helper.h```
- ```load-balancer-test.cc```
- ```server.cc```
- ```server.h```
- ```server-helper.cc```
- ```server-helper.h```

### 1.2 src/applications/helper/ 경로에 아래 파일 복사
- ```load-balancer-helper.cc```
- ```load-balancer-helper.h```

### 1.3 src/applications/model/ 경로에 아래 파일 복사
- ```load-balancer.cc```
- ```load-balancer.h```
- ```load-balancer-header.cc```
- ```load-balancer-header.h```

[LB-Server 관련 유의사항 참조](https://github.com/netproj-team6/skku_chat/tree/main/lb#%EC%9C%A0%EC%9D%98-%EC%82%AC%ED%95%AD)

### 1.4 /ns-allinone-3.29/ns-3.29 경로에서 아래 명령어로 실행

- ```./waf --run streaming ```




## 2. 클라이언트 동작
[클라이언트에 대한 자세한 설명은 여기로](https://github.com/netproj-team6/Video_Streamer/tree/main/client)
## 3. 서버 동작
[서버에 대한 자세한 설명은 여기로](https://github.com/netproj-team6/Video_Streamer/tree/main/server)
## 2. LB(Load-Balancing) 서버 동작
[LB 서버에 대한 자세한 설명은 여기로](https://github.com/netproj-team6/Video_Streamer/tree/main/server)
