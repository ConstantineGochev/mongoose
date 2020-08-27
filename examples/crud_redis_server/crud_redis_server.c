#include <inttypes.h>
#include <stdlib.h>

#include "mongoose.h"

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

/* Main event handler. */
static void ev_handler(struct mg_connection *nc, int ev, void *p) {
  switch (ev) {
    case MG_EV_HTTP_REQUEST: {
      struct http_message *hm = (struct http_message *) p;
      fprintf(stderr, " requested %.*s\n", (int) hm->uri.len, hm->uri.p);

      mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
      break;
    }
    case MG_EV_SSI_CALL: {
      /* Expand variables in a page by using session data. */
      //const char *var = (const char *) p;
      break;
    }
  }
}

int main(void) {
  struct mg_mgr mgr;
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, s_http_port, ev_handler);

  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = ".";
  //mg_register_http_endpoint(nc, "/login.html", login_handler);
  //mg_register_http_endpoint(nc, "/logout", logout_handler);

  printf("Starting web server on port %s\n", s_http_port);
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}

