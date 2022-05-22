## Descrption

### `load-balancer-test.cc`

- 시나리오 파일에서 Load Balancer를 어떤 방식으로 사용할 수 있는지 알려주기 위함
- `scratch/` 경로에 위치

### `load-balancer-hepler.h`, `load-balancer-helper.cc`

- Load Balancer 생성 및 초기 세팅을 위함
- `src/applications/helper/` 경로에 위치

### `load-balancer.h`, `load-balancer.cc`

- 실질적인 Load Balancer 동작을 위함
- `src/applications/model/` 경로에 위치

## 유의 사항

- `load-balancer-helper.*` 파일들은 `src/applications/helper/` 경로에 위치시켜야 함

- `load-balancer.*` 파일들은 `src/applications/model/` 경로에 위치시켜야 함

- `./waf build`시 빌드되도록 `src/applications/wscript` 파일을 아래와 같이 수정해야 함

  ```
  def build(bld):
  	# ... #
  	module.source = [
  			# ... #
  			'model/load-balancer.cc',
              'helper/load-balancer-helper.cc',
  			]
  
  	# ... #
      headers.source = [
  			# ... #
  			'model/load-balancer.h',
  			'helper/load-balancer-helper.h',
  			]
  
  	# ... #
  ```

  
