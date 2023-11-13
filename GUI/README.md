# 대시보드 GUI 사용법

1. Python 설치

2. pyserial, PyQt6 설치
```powershell
python -m pip install pyserial pyqt6
```

3. (Optional) PySide6 설치  
PySide6는 GUI 파일 (dashboard.py, dashboard.ui) 편집에 필요한 Qt 6 Designer (UI 편집기)와 pyuic6 (파이썬 코드 생성기) 툴을 포함하고 있습니다.  
위 두 파일은 아래 [GUI 코드 편집하기](#gui-코드-편집하기)를 참고하여 수정해 주세요.
```powershell
python -m pip install pyside6
```

4. S32K144 보드를 PC와 연결한 후, 장치 관리자 → 포트(COM & LPT) 항목에서 OpenSDA - CDC Serial Port 번호 확인

5. display.py 파일 열고, port 변수에 포트 번호 입력하기  
ex) port = 'COM7'
6. display.py 실행
```powershell
cd .\GUI\
python display.py
```

## GUI 코드 편집하기
GUI 구현 코드 (dashboard.ui, dashboard.py)는 직접 수정할 수도 있지만, 아래와 같은 방법으로 수정하는 것을 권장합니다.
1. PySide6 설치

2. Qt 6 Designer로 dashborad.ui 파일 불러온 후 필요한 수정 진행 및 저장

3. pyuic6으로 dashboard.ui 파일을 파이썬 코드로 변환  
**주의!! 아래 명령은 이전의 dashboard.py 파일을 덮어씁니다.**  
환경 변수 설정이 필요할 수 있습니다.
```powershell
pyuic6 .\dashboard.ui -o dashboard.py
```
