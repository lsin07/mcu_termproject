# mcu_termproject
2023 2학기 마이크로프로세서설계실험 term project

## 주제 선정 동기

S32K144가 원래 자동차용 제어기 보드니까 자동차용 제어기를 만들어보았다..!

## 제어 모듈

### Throttle
DC 모터 사용

### Accelerator
가변저항 사용

### Sensor
+ 초음파 센서 동작 원리:  

    1. 60ms마다 최소 10us 길이의 TRIG HIGH 신호 입력
    2. TRIG 신호가 감지되면 자체적으로 8번의 초음파 펄스 생성
    3. 초음파 펄스가 끝나면, 반사된 초음파가 도착할 때까지 ECHO가 set됨
    4. ECHO HIGH 시간이 길수록 물체가 멀리 있는것.

1. 10us마다 LPIT 인터럽트 신호 생성. TRIG 주기는 60ms이므로 인터럽트 신호가 6000번 생성되면 1주기가 실행된 것임.

2. 매 주기의 첫 실행에 TRIG SET, 두 번째 실행에 TRIG CLEAR 하면 10us 길이의 신호가 생성됨

3. ISR이 1주기(6000번) 실행되는 동안, ECHO HIGH일 때 high counter를 1 증가시킴.

4. 그러면 1주기가 끝난 이후 high counter의 값은 거리와 비례함.  
주기가 끝나면 high counter의 값 리셋

### Alarm
센서가 읽어온 값에 따라 작동.

### Lamp (방향지시등, 후진등)
기어, 램프 점등 유무를 확인하여 알맞게 켜짐.  
방향지시등은 0.5초 간격으로 깜빡임.

### Panel
버튼이 눌리면 인터럽트를 통해 전역 변수 값 설정

### Dashboard
LPUART로 PC와 통신  
PC에서는 PyQt, PySerial 사용