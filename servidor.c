#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PUERTO 80
#define ESCUCHAC 23
#define TAMBUFFER 8096

void navega (int);

int main (void)
{
  int escucha_da, socket_da, pid;
  size_t longitud;
  static struct sockaddr_in servidordir;
  static struct sockaddr_in clientedir;

	
  escucha_da = socket (AF_INET, SOCK_STREAM, 0);
	
  servidordir.sin_family = AF_INET;
  servidordir.sin_addr.s_addr = htonl (INADDR_ANY);
  servidordir.sin_port = htons (PUERTO);
	
  bind (escucha_da, (struct sockaddr *) &servidordir, sizeof (servidordir));
	
  listen (escucha_da, ESCUCHAC);
	 
  for (; ;) {
    longitud = sizeof (clientedir);
    socket_da = accept (escucha_da, (struct sockaddr *) &clientedir, &longitud);
    if ((pid = fork ()) == 0) {
      close (escucha_da);
      navega (socket_da);
      exit (0);
    } else {
      close (socket_da);
    }
  }

  return 0;
}

void navega (int socket_da)
{
  int i, archivo_da;
  long r, tam;
  static char buffer[TAMBUFFER+1];
  struct stat st;

  r = read (socket_da, buffer, TAMBUFFER);
  buffer[r-1] = '\0';

  printf ("%s\n", buffer);
	
  if (strncmp (buffer, "GET ", 4) == 0) {
    for (i = 4; i < TAMBUFFER; i++)
      if (buffer[i] == ' ') {
	buffer[i] = '\0';
	break;
      }
		
    if (!strncmp (&buffer[0], "GET /\0", 6))
      strcpy (buffer, "GET /index.html");
		
    if ((archivo_da = open (&buffer[5], O_RDONLY)) == -1) {
      archivo_da = open ("404.html", O_RDONLY);
      fstat (archivo_da, &st);
      tam = st.st_size;
      sprintf (buffer, "HTTP/1.0 404 Not Found\r\n\r\n", tam);
    } else { 
      fstat (archivo_da, &st);
      tam = st.st_size;		
      sprintf (buffer, "HTTP/1.0 200 OK\r\n\r\n", tam);
    }

    write (socket_da, buffer, strlen (buffer));
		
    while ((r = read (archivo_da, buffer, TAMBUFFER)) > 0 )
      write (socket_da, buffer, r);
  }
}
