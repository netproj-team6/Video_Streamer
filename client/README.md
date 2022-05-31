# 클라이언트

## 1. 실행방법

  - scratch/streaming 경로에 아래 파일 복사

    - client.cc
    - client.h
    - client-helper.cc
    - client-helper.h
    - load-balancer-test.cc
    - server.cc
    - server.h
    - server-helper.cc
    - server-helper.h

  - src/applications/helper/ 경로에 아래 파일 복사

    - load-balancer-helper.cc
    - load-balancer-helper.h

  - src/applications/model/ 경로에 아래 파일 복사

    - load-balancer.cc
    - load-balancer.h
    - load-balancer-header.cc
    - load-balancer-header.h

  - [유의사항 참조](https://github.com/netproj-team6/skku_chat/tree/main/lb#%EC%9C%A0%EC%9D%98-%EC%82%AC%ED%95%AD)

  - /ns-allinone-3.29/ns-3.29 경로에서 아래 명령어로 실행

    - ``` ./waf --run streaming ```

## 2. 클라이언트 동작

#### 1. 클라이언트에서 패킷을 보낼 때 헤더(SeqTsHeader)에 필요한 정보를 담아서 전송

  - 바깥쪽 헤더: 요청 종류
    - 0: resume 요청
    - 1: pause 요청
    - 2: streaming 요청(시작할 때)
    - 3: ACK 패킷
    
  - 안쪽 헤더: ACK 패킷만 가지고 있고 sequence number 담아서 전송

#### 2. 클라이언트가 시작하면 LB서버에 스트리밍 요청 패킷 전송, 서버로부터 패킷을 받기 전까지 반복 전송

#### 3. 서버에서 패킷을 받으면 프레임 생성 및 소비 시작, 반복

#### 4. 받은 패킷의 sequence number 저장 및 LB서버에 ACK 패킷 전송

#### 5. 프레임을 생성할 때는 받은 패킷으로 만들 수 있는 프레임을 모두 찾아서 생성

#### 6. 프레임을 소비할 때는 하나씩 소비

  - 버퍼에 있는 프레임 수 >= bufferingSize인 경우: 버퍼에 소비해야하는 프레임이 있으면 프레임 소비, 없으면 다음 프레임으로 넘어감

  - 버퍼에 있는 프레임 수 < bufferingSize인 경우: 버퍼에 있는 프레임 수가 bufferingSize보다 커질 때 까지 프레임 소비 안함(버퍼링)

  - 버퍼에 있는 프레임 수 > pauseSize인 경우: LB서버에 pause 요청 패킷 전송

  - 버퍼에 있는 프레임 수 < resumeSize인 경우: LB서버에 resume 요청 패킷 전송

## 3. 클라이언트 - 시나리오

```
StreamingClientHelper client(Address, Port);                // Address, Port : 패킷을 보낼 LB서버의 IP주소와 포트
client.SetAttribute("LossRate", DoubleValue(0.));           // 패킷 손실률(0 ~ 1)
client.SetAttribute("PacketSize", UintegerValue(100));      // 보낼 패킷의 크기
client.SetAttribute("PacketsPerFrame", UintegerValue(100)); // 프레임 하나를 만드는데 필요한 패킷 수
client.SetAttribute("BufferingSize", UintegerValue(25));    // 버퍼에 있는 프레임 수가 이 값 보다 작아지면 버퍼링 시작
client.SetAttribute("PauseSize", UintegerValue(30));        // 버퍼에 있는 프레임 수가 이 값 보다 커지면 pause 요청
client.SetAttribute("ResumeSize", UintegerValue(25));       // 버퍼에 있는 프레임 수가 이 값 보다 작아지면 resume 요청
client.SetAttribute("RequestInterval", TimeValue(Seconds(1. / 10.)));   // 처음에 LB서버에 스트리밍 요청하는 간격
client.SetAttribute("GeneratorInterval", TimeValue(Seconds(1. / 20.))); // 프레임을 생성하는 간격, 생성할 수 있는 프레임 모두 생성
client.SetAttribute("ConsumerInterval", TimeValue(Seconds(1. / 60.)));  // 프레임 하나를 소비하는 간격(fps)
ApplicationContainer clientApp = client.Install(nClient);
```

- 예시에서 설정된 값들이 default, 변경해야 할 때만 SetAttribute 설정

## 4. 클라이언트 - LB서버

- 클라이언트가 시작하면 LB서버에 스트리밍 요청 패킷 전송

- 버퍼에 있는 프레임 수에 따라 LB서버에 pause, resume 요청 패킷 전송

- 서버로부터 패킷을 받으면 LB서버에 ACK 패킷 전송

## 5. 클라이언트 - 서버

- 서버로부터 받은 패킷의 헤더(SeqTsHeader)를 통해 패킷의 sequence number 확인
