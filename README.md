# Video Streamer with Load-balancing

## Intro

### 회의록

[Notion Link](https://jonjjon.notion.site/jonjjon/6-1058851269db476c9ceeba7f8f121e94)

### Issue Tracking

[Github Issue Tab](https://github.com/netproj-team6/Video_Streamer/issues?q=is%3Aissue+is%3Aclosed)

### Docker Image

[Docker Hub Link](https://hub.docker.com/repository/docker/jonjjon/network_project_team6/general)

## 설치 및 실행 방법

0. scratch/ 경로에서 다음 명령서 실행
```
git clone https://github.com/netproj-team6/Video_Streamer.git
```

1. scratch/Video_Streamer 경로에 아래 파일 복사 - 시나리오 파일은 둘 중 하나만 선택하여 넣어둘것

   - ```client.cc```
   - ```client.h```
   - ```client-helper.cc```
   - ```client-helper.h```
   - ```Video_Streamer_p2p.cc(시나리오)```
   - ```Video_Streamer_CSMA.cc(시나리오)```
   - ```server.cc```
   - ```server.h```
   - ```server-helper.cc```
   - ```server-helper.h```

2. src/applications/helper/ 경로에 아래 파일 복사

   - ```load-balancer-helper.cc```
   - ```load-balancer-helper.h```

3. src/applications/model/ 경로에 아래 파일 복사

   - ```load-balancer.cc```
   - ```load-balancer.h```
   - ```load-balancer-header.cc```
   - ```load-balancer-header.h```

4. src/internet/model/ 경로에 아래 파일 덮어쓰기

   - ```global-route-manager-impl.h```
   - ```global-route-manager-impl.cc```

5. ns-allinone-3.29/ns-3.29 경로에서 아래 명령어로 실행

   ```bash
   ./waf --run Video_Streamer
   ```

6. Load Balancer 관련 유의사항 참조
   [GitHub Link](https://github.com/netproj-team6/Video_Streamer/tree/main/lb#%EC%9C%A0%EC%9D%98-%EC%82%AC%ED%95%AD)


- `./waf build`시 빌드되도록 `src/applications/wscript` 파일을 아래와 같이 수정해야 함

  ```
  def build(bld):
    # ... #
    
    module.source = [
        # ... #
        'model/load-balancer.cc',
        'model/load-balancer-header.cc',
        'helper/load-balancer-helper.cc'
        ]
    
    # ... #
    
    headers.source = [
        # ... #
        'model/load-balancer.h',
        'model/load-balancer-header.h',
        'helper/load-balancer-helper.h'
        ]
    
    # ... #
  ```

  

   

## 클라이언트 동작

[Link to the details of client (Github)](https://github.com/netproj-team6/Video_Streamer/blob/main/client/README.md)

## 스트리밍 서버 동작

[Link to the details of streaming server (Github)](https://github.com/netproj-team6/Video_Streamer/blob/main/server/README.md)

## 로드밸런서 서버 동작

[Link to the details of load balancer (Github)](https://github.com/netproj-team6/Video_Streamer/blob/main/lb/README.md)


## Ip table
*topology 변경 없을 때 ip 상태*

|  Node |  Address 0 | Address 1  |  
|---|---|---|
| global swith 0  |  10.1.1.1 |  10.1.4.2  |   
|  global swith 1 |  10.1.2.1  |  10.1.1.2 |   
|  global swith 2  | 10.1.3.1  | 10.1.2.2  |  
|  global swith 3  |  10.1.4.1  |  10.1.3.2 |  
|  stream server 0  |  10.1.5.2  |    |
| stream server 1   |  10.1.6.2   |    |
|  stream server 2   |  10.1.7.2   |    |
|  balance server  |  10.1.8.2   |    |
|   global user 0 |   10.1.9.2  |    |
|  global user 1  |  10.1.10.2   |    |
|  global user 2  |  10.1.11.2  |    |
|  csma user 0  |  10.1.13.2  |    |
|  csma user 1  |  10.1.13.3  |    |
|  csma user 2  | 10.1.13.4   |    |
|  wifi user 0  |  192.168.2.2  |    | 
|  wifi user 1  |  192.168.2.3  |    | 
|  wifi user 2  |   192.168.2.4  |    | 
