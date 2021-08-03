# Qt Socket Program
### v1.

##### (ThreadSocketServer - 서버)

1. QTcpSocket을 이용한 기본통신 기능 구현
2. sever의 ip확인기능과  포트번호를 입력하고 QTcpServer를 이용하여 해당 포트에서 listen하는 기능 구현
3. 현재 서버의 상태를 text로 보여주는 기능 구현
4. 접속한 클라이언트의 정보(소켓 기술자,ip,port,연결한 시간)를 보여주고, 연결이 끊기면 정보를 제거하는 기능 구현.
5. 메세지를 전송하면 연결된 모든 클라이언트로 전송하는 기능 구현
6. 클라이언트로 부터 text데이터를 받으면 받은 데이터를 표시하고, 특정한 문자열을 붙여서 클라이언트쪽으로 보내주는 기능 구현.

##### (socket_client -  클라이언트)

1. QTcpSocket을 이용한 기본통신 기능 구현
2. 접속할 서버의 ip와 포트번호를 입력하여 연결시도를 하는 기능 구현.
3. 연결된 서버와 접속을 끊는 기능 구현.
4. 소켓의 상태를 출력해주는 기능 구현.
5. 서버로 메세지를 보내고 리턴되는 메시지를 받아서 표시하는 기능 구현.

### v2.

##### (multiPortServer - 서버)

1. 스레드의 경우 수신버퍼의 크기를 넘는 데이터를 읽어올때 비동기 방식으로 문제발생, 단일스레드+멀티포트로 변경
2. 멀티포트를 이용하여 기능 구현(스레드 x)
3. 대용량 메시지, 파일(모든 확장자 가능)전송이 가능하도록 기능 추가
4. listen할 포트범위를 입력하면 해당 범위의 모든 포트를 여는 기능 추가
5. 클라이언트와 연결해제시에 메시지박스로 알려주는 기능 추가

##### (socket_client - 클라이언트)

1. 전송데이터에 헤더를 추가해서 서버가 인식할수 있도록 기능 추가
2. 파일 선택버튼을 이용하여 pc내의 파일을 선택하는 기능 추가
3. 파일 전송버튼을 이용하여 모든 확장자에 대해 전송가능

### v2.5

##### (공통)

1.  대용량 전송시, 파일이 전송되고 있음을 알수 있도록 문구를 출력하고, 전송이 끝나면 서버에서 클라이언트 쪽으로 전송완료 문구를 보내서 확인할수 있도록 함.
2. 기존 기능 그대로 유지하면서 메세지 포맷을 아래그림과 같이 변경함.

![image](https://user-images.githubusercontent.com/56991244/127982771-17d8cf63-26db-4671-9dcd-a8bae354d9bb.png)
