// 구글 클라우드용
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>


typedef struct _message
{
    char m_buffer[128];
    char m_time[20];
} MESSAGE;

int main(){
  MESSAGE m_message;
  fd_set status;
  
  int servSock;
  int clntSock[3];
  int tempSock;

  int iMaxSock;
  
  unsigned int uiUser;

  int iCount;
  int i;
  
  struct sockaddr_in echoServAddr;
  struct sockaddr_in echoClntAddr;
  
  unsigned short echoServPort;
  unsigned int clntLen;

  time_t stTempTime;
  
  int iRet;

  unsigned char ucBuffer[500];
  int tmp_hum;
  int tmp_temp;

  echoServPort = 4000;

  servSock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );  // Socket을 생성한다.
  if( servSock < 0 )
  {
    printf( "Socket Function Error!\n" );
    return( 0 );
  }
  memset( &echoServAddr, 0, sizeof( echoServAddr ) );
  echoServAddr.sin_family = PF_INET;
  echoServAddr.sin_addr.s_addr = htonl( INADDR_ANY ); 
  echoServAddr.sin_port = htons( echoServPort );

  iRet = bind( servSock, ( struct sockaddr * )&echoServAddr, sizeof( echoServAddr ) );
  
  if( iRet < 0 )
  {
    close( servSock );
    printf( "Bind Failed Error!\n" );
    return( 0 );
  }

  iRet = listen( servSock, 3 ); // MAX PENDING : 3
  if( iRet < 0 )
  {  
    close( servSock );
    printf( "Listen  Failed Error!\n" );
    return( 0 );
  }
  clntLen = sizeof( echoClntAddr );    
  iMaxSock = servSock+1;  
  uiUser = 0;

  while(1){
    FD_ZERO( &status );
    FD_SET( 0, &status );  // 0번째 값을 1로 바꾼다(stdin) -> 키보드 감지.
    FD_SET( servSock, &status ); // 랑데부 소켓과 키보드만 감지한다.

    for( i=0; i<uiUser; i++ )
    {
      FD_SET( clntSock[i], &status );      
      if( iMaxSock <= clntSock[i] )
      {
        iMaxSock = clntSock[i]+1;
      }      
    }
    
    iRet = select( iMaxSock, &status, 0, 0, 0 );
    if( iRet < 0 )
    {
      printf( "Select Function Error!\n" );
      break;
    }
    
    if( 1 == FD_ISSET( servSock, &status ) )  // 랑데부. 새로운 클라이언트가 접속했을 때.
    {
        tempSock = accept( servSock, ( struct sockaddr * )&echoClntAddr, &clntLen );
 
      printf( "Socket Number : %d\n", tempSock );
      
      if( tempSock < 0 )
      {
        printf( "Accept Function Error!\n" );
        continue;
      }
    
      printf( "클라이언트 접속 IP : %s\n", inet_ntoa( echoClntAddr.sin_addr ) );  
      printf( "클라이언트 접속 PORT : %d\n", ntohs( echoClntAddr.sin_port ) );

      if( 3 <= uiUser )
      {
        close( tempSock );
        continue;  // 와일 문 정상화.
      }

      clntSock[uiUser] = tempSock;  // tempSock으로 받은 커뮤니케이션 소켓을 넣어준다.
      uiUser = uiUser + 1;
      printf( "현재 접속자 수는 총 %d명 입니다.\n", uiUser );      
    }
    //서버->클라이언트, 서버의 키보드 입력이있을때
    else if(1 == FD_ISSET(0, &status)){
      iRet = read(0, m_message.m_buffer, sizeof(m_message.m_buffer));
      //strcpy(For_Server_Problem, m_message.m_buffer); 
      m_message.m_buffer[iRet - 1] = 0;
      //strcpy(m_message.m_userName, "PROBLEM");

      for (iCount = 0; iCount<uiUser; iCount++) 
      {
        write(clntSock[iCount], &m_message, sizeof(m_message));       

      }
    }

    //클라이언트로부터 받은 메세지가 있을 때
    //node js 웹서버로부터 받은 메세지가 있을때 처리될 부분
    //웹에서 전달받은 메세지를 라즈베리파이로 바로 전송한다.
    //node js 웹서버가 항상 첫번째 client가 될것이기 때문에 clntSock[0]는 nodejs
    //두번째 client는 라즈베리파이 clntSock[1].
    //세번째 client는 구글서버에 돌아갈 에어컨 판단코드 client[2].

    else{
        if (1 == FD_ISSET(clntSock[0], &status))
        {
            iRet = read(clntSock[0], &m_message, sizeof(ucBuffer) - 1);
            printf("read : %s\n",m_message.m_buffer);
            char success[10] = "SUCCESS";
            iRet = write(clntSock[0], success, sizeof(success));
            if(iRet!=0)
            {
              printf("ERROR\n");
            }

            //m_message안의 변수를 읽어서 웹서버가 요청한 동작에대한 내용을 라즈베리파이로 전송
            //웹서버가 요청하는 동작 -> led밝기조절, 에어컨 희망온도
            if (iRet != 0)
            {
                printf("receive message\n");
                //write(clntSock[1], &m_message, sizeof(m_message));
                //write(clntSock[2], &m_message, sizeof(m_message));            
            }
        }

       //3_km_rx_lora.cpp 파일에서 로라로 수신한 온도,습도 값을 aircon_judge.js로 보낼것임
       /*if (1 == FD_ISSET(clntSock[1], &status)){
          iRet = read(clntSock[1], &m_message, sizeof(ucBuffer) - 1);
        
          if (iRet != 0){
            tmp_temp = m_message.m_temp;
            tmp_hum = m_message.m_hum;
            
            write(clntSock[2], &m_message, sizeof(m_message));           
            
          }
        }
        //aircon_judge.js(clntSock[2])가 불쾌지수가 68보다 낮다고 판단했을때, 3_km_rx_lora.cpp(clntSock[1])파일로 에어컨을 1도낮추라고 명령
        if (1 == FD_ISSET(clntSock[2], &status)){
            iRet = read(clntSock[2], &m_message, sizeof(ucBuffer)-1);
            if(iRet != 0){
                write(clntSock[1], &m_message, sizeof(m_message));
            }
        }*/
    }
   

  

  }
}