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
#define PORT 8090
#define LISTENQ 23 
#define BUFFERSIZE 8096

void web (int);

int main (int argc, char **argv)
{
	int listenfd, socketfd, pid;
	socklen_t length;
	static struct sockaddr_in serv_addr;
	static struct sockaddr_in cli_addr;


	// Se crea el socket 
	listenfd = socket (AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	serv_addr.sin_port = htons (PORT);

	// Se asociamos la direccion con el socket
	bind (listenfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr));

	listen (listenfd, LISTENQ);

	// Creamos los hijos(fork) para manejar las solicitudes 
	for (; ;) {
		length = sizeof (cli_addr);
		//socketfd = accept (listenfd, (struct sockaddr *) &cli_addr, &length);
		socketfd = accept (listenfd, (struct sockaddr *) &cli_addr, &length);
		if ((pid = fork ()) == 0) {
			close (listenfd);
			web (socketfd);
		} else {
			close (socketfd);
		}
	}
}

void
web (int fd)
{
	int i, file_fd;
	long ret, size;
	static char buffer[BUFFERSIZE+1];
	struct stat st;
	char * ext;

	ret = read (fd, buffer, BUFFERSIZE);

	// Si leemos un numero de bytes menor al tamano del buffer, pongo el caracter de terminacion
	if (ret > 0 && ret < BUFFERSIZE) {
		buffer[ret] = 0;
	} else {
		buffer[0] = 0;
	}

	// verifico que el navegador nos envia la instruccion GET
	if (strncmp (buffer, "GET ", 4) == 0) {
		for (i = 4; i < BUFFERSIZE; i++) {
			if (buffer[i] == ' ') {
				buffer[i] = 0;
				break;
			}
		}

		// Verifico que el usuario especifico una pagina, si no lo mando al index
		if (!strncmp (&buffer[0], "GET /\0", 6)) {
			strcpy (buffer, "GET /index.html");
		}

		// Se abre el archivo, si no existe se abre el de error 
		if ((file_fd = open (&buffer[5], O_RDONLY)) == -1) {
			file_fd = open ("404.html", O_RDONLY);
			fstat (file_fd, &st);
			size = st.st_size;
	                sprintf (buffer, "HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\nContent-Lenhttp://localhost/test.htmlhttp://localhost/test.htmlgth: %ld\r\n\r\n",size);

		} else { // Si el archivo existe se muestra el contenido
			fstat (file_fd, &st);
			size = st.st_size;


			ext = strstr(buffer,".");
			ext = ext+1;
			fprintf(stderr,"%s",ext);
			if(strcmp("html",ext)==0){

		        sprintf (buffer, "HTTP/1.1 200 OK\r\nContent-Type: text/html \r\nContent-Length: %ld\r\n\r\n", size);
			} else if(strcmp("jpg",ext)==0){

			sprintf (buffer, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg \r\nContent-Length: %ld\r\n\r\n", size);
			} else if (strcmp("txt",ext)==0){

			sprintf (buffer, "HTTP/1.1 200 OK\r\nContent-Type: text/plain \r\nContent-Length: %ld\r\n\r\n", size);
			} else{

			sprintf (buffer, "HTTP/1.1 200 OK\r\nContent-Type: unknown/unknown \r\nContent-Length: %ld\r\n\r\n", size);
			}
		}

		write (fd, buffer, strlen (buffer));

		// Obtenemos el archivo 
		while ((ret = read (file_fd, buffer, BUFFERSIZE)) > 0 ) {
			write (fd, buffer, ret);
		}
	}

	sleep (1);
	exit (1);
}
