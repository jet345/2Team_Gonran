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
int main(int argc, char *argv[])
{
  fd_set status;
  MESSAGE m_message;
  
  int sock;
  struct sockaddr_in echoServAddr;
  unsigned short echoServPort;
  char *servIP;
  char *echoString;
  char buffer[500];
  time_t stTempTime;
  int iRet;
  servIP = "35.185.16.66";
  
  sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if( sock < 0 )
  {
    printf( "Socket Function Failed!\n" );
    return 0;
  }
  memset( &echoServAddr, 0, sizeof( echoServAddr ) );
  echoServAddr.sin_family = AF_INET;
  echoServAddr.sin_addr.s_addr = inet_addr( servIP );
  echoServAddr.sin_port = htons( 4000 );
  iRet = connect( sock, ( struct sockaddr* )&echoServAddr, sizeof( echoServAddr ) );
  
  if( iRet < 0 )  // 접속 요청 실패
  {
    printf( "Connect Function Failed!\n" );
    close( sock );  // 소켓이 열려 있으므로 닫아준다.
    return 0;
  }
  while( 1 )
  {
    FD_ZERO( &status );
    FD_SET( 0, &status );
    FD_SET( sock, &status );
    iRet = select( sock+1, &status, 0, 0, 0 );
    if( iRet < 0 )
    {
      printf( "Select Function Error!\n" );
      break;
    }
    
    // client가 server로 전송하고자 할 때
    /*
    strcpy함수를 이용하여 m_message.m_buffer에 보내고자 하는 센서값을 저장하고
    write(sock...)함수를 통해 서버로 전송하면 됩니다.
    아래는 stdin으로 입력받은 message를 서버로 전송하는 코드입니다. 참고해주세요^.^
    */
    if( 1 == FD_ISSET( 0, &status ) )
    {
    // 0은 stdin을 의미합니다.
        iRet = read( 0, m_message.m_buffer, sizeof( m_message.m_buffer ) );      
        m_message.m_buffer[iRet-1] = 0;
        printf("hey : %s\n",m_message.m_buffer);      
        write( sock, &m_message, sizeof( m_message ) );
    }
    /*
        server가 보낸 명령이 있을 시 진입하는 코드입니다.
        IR모듈부만 작성하면 됩니다^.^
    */
    else if( 1 == FD_ISSET( sock, &status ) )
    {
        read( sock, &m_message, sizeof( m_message ) ); 
        printf( "server : %s", m_message.m_buffer );
    }
  }
  close( sock );  //소켓을 닫아준다.
  return 0;
}
