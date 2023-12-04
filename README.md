# Simple_TCP_Chatting_Server  

  
C++의 Boost 라이브러리를 이용한 채팅 서버  

![KakaoTalk_20231204_141418156_02](https://github.com/NineHearts/Simple_TCP_Chatting_Server/assets/46129253/03d6c22e-b544-468c-aa4d-3c88a6a86403)  


서버는 세션의 정보를 Vector에 저장하여 브로드캐스팅 기능을 수행하고, 세션을 생성하고 접속을 받아들이는 기능, 세션이 접속하면 벡터에 저장, 세션이 끊기면 벡터에서 세션의 정보를 삭제한다.

세션에서는 메세지를 받아들이고 strand를 이용하여 읽고 쓰기를 핸들링한다.
