# 서버 역할
1. LB 서버에서 받은 헤더 정보를 토대로 클라이언트에 패킷 전달

# 서버 동작
동작방식 설명

# 서버 시나리오
setAttribute관련 설명

# 시나리오 파일
1. ./waf 해보면 로그 결과로 LB 서버에서 보낸 ipv4 주소와 포트를 서버에서 제대로 받고 있음을 확인할 수 있음.
  - LB 서버에서 Header에 넣은 결과<br/>
  ![다운로드](https://user-images.githubusercontent.com/43779340/170795478-23d0f096-3ed9-465c-903f-d549b5a24275.png)
  - Streaming 서버에서 Header 확인 결과<br/>
  ![다운로드](https://user-images.githubusercontent.com/43779340/170795654-3d2ae777-0a5c-4337-9ed5-70737613255d.png)

2. server.cc 내 HandleRead 함수에서 m_peerPort를 10으로 바꾸고 ./waf시 보냈던 nSRC 노드들에서 패킷을 받고 있음을 확인할 수 있음. <br/>
  (보낸 노드(nSRC) 포트를 알 수 있는 방법이 없어서 일단 이 방식으로 확인했습니다 )
  ![다운로드](https://user-images.githubusercontent.com/43779340/170795894-74d9ac52-87fa-4037-9e13-dbcb87cd2b6e.png)

# 서버 - LB서버

# 서버 - 클라이언트


# 의문점 
계속 고민을 해봤는데 잘 모르겠습니다. 일단은 실행이 되도록 고쳤지만, 임시 방편이라 한번 고민해주시면 감사하겠습니다. 
- 시나리오 파일
  1. dst0.SetAttribute("Interval", TimeValue(Seconds(4))) => TimeValue 설정이 너무 작으면 오류가 나옴. NS_ASSERT (m_sendEvent.IsExpired ())
