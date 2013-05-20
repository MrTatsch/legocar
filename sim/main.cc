#include "myarea.h"
#include <gtkmm/application.h>
#include <gtkmm/window.h>

#include "config.h"
#include "tcpserver.h"

void *tcp_thread(void * v)
{
  struct tcpserver ts;
  ts.port = SERVER_PORT;
  tcpserver_start ( &ts );
}

int main(int argc, char** argv)
{
  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

  Gtk::Window win;
  win.set_title("DrawingArea");

  pthread_t tcpthread;

  MyArea area;
  win.add(area);
  area.show();

  if( pthread_create( &tcpthread , NULL ,  tcp_thread , NULL) < 0)
  {
   perror("E: main: could not create tcp thread\n");
   return -1;
  }

  return app->run(win);
}

