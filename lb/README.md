## Operation

여러 개의 스트리밍 서버가 존재할 경우, 로드 밸런서 서버는 클라이언트 요청을 스트리밍 서버에게 골고루 분배해주어서 어느 한쪽에만 작업이 몰리지 않도록 해주는 역할을 한다.

1. Interleaved Weighted Round Robin

   각 스트리밍 서버의 가용한 처리량을 고려하여 비율에 맞춰 클라이언트 요청을 처리하도록 만들기 위해, Round Robin 기반 알고리즘에서 **Weighted** 개념을 사용한다.

   더불어 각 스트리밍 서버의 Weight가 다를 경우, Weight가 비교적 큰 스트리밍 서버에 클라이언트 요청이 먼저 할당되고 난 후에야 다음으로 Weight가 큰 스트리밍 서버에 할당되는 식의 흐름을 막기 위해 **Interleaved** 개념을 사용한다.

   [참고 Wikipedia](https://en.wikipedia.org/wiki/Weighted_round_robin#Interleaved_WRR)

2. Sticky Session

   처음 클라이언트와 스트리밍 서버 연결을 맺을 때 매칭된 정보, 즉 세션을 기록해놓고 이후 동일한 클라이언트 요청이 오면 과거 매칭했던 서버로 가도록 동작함으로써, 로드 밸런싱 Cost가 매우 적다.

3. Direct Server Return

   스트리밍 애플리케이션의 클라이언트와 서버 구조 상, In-Bound 보다 Out-Bound 패킷을 훨씬 많다는 점을 고려하여, 스트리밍 서버는 로드 밸런싱 서버를 통하지 않고 곧바로 클라이언트에게 Response 패킷을 전송한다.

## Descrption

### `load-balancer-test.cc`

- 시나리오 파일에서 Load Balancer를 어떤 방식으로 사용할 수 있는지 알려주기 위함
- `scratch/` 경로에 위치

### `load-balancer-hepler.h`, `load-balancer-helper.cc`

- Load Balancer 생성 및 초기 세팅을 위함
- `src/applications/helper/` 경로에 위치

### `load-balancer-header.h`, `load-balancer-header.cc`

- Load Balancer가 Streaming Server에게 패킷 전달 시 클라이언트 Ipv4Address와 Port를 알려주기 위함
- `src/applications/model/` 경로에 위치

### `load-balancer.h`, `load-balancer.cc`

- 실질적인 Load Balancer 동작을 위함
- `src/applications/model/` 경로에 위치

## 유의 사항

- `load-balancer-helper.*` 파일들은 `src/applications/helper/` 경로에 위치시켜야 함

- `load-balancer.*`, `load-balancer-header.*` 파일들은 `src/applications/model/` 경로에 위치시켜야 함

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

  
