#include <inttypes.h>
#include <stdlib.h>
#include "hiredis/hiredis.h"
#include "../../mongoose.h"

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;
static const struct mg_str s_get_method = MG_MK_STR("GET");
static const struct mg_str s_put_method = MG_MK_STR("PUT");
static const struct mg_str s_delete_method = MG_MK_STR("DELETE");
static const struct mg_str s_post_method = MG_MK_STR("POST");

static int is_equal(const struct mg_str *s1, const struct mg_str *s2) {
  return s1->len == s2->len && memcmp(s1->p, s2->p, s2->len) == 0;
}

static void handle_get_call(struct mg_connection *nc, redisContext *rc) {
  /* Send headers */
  mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");

  redisReply *reply;
  reply = redisCommand(rc,"LRANGE db %d %d",0, -1);
  printf("PING: %s\n", reply->str);
  /* Compute the result and send it back as a JSON object */
  mg_printf_http_chunk(nc, "{ \"result\": %lf }", reply->str);
  freeReplyObject(reply);
  mg_send_http_chunk(nc, "", 0); /* Send empty chunk, the end of response */
}

static void handle_post_call(struct mg_connection *nc, struct http_message *hm, redisContext *rc) {
  char post_str[100];
  mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
  /* Get form variables */
  mg_get_http_var(&hm->body, "post_str", post_str, sizeof(post_str));
  printf("post_str = %s\n", post_str);
  mg_printf(nc, "BODY = *s\n", hm->body);
  redisReply *reply;
  reply = redisCommand(rc,"LPUSH db %s",post_str);
  printf("RESULT: %s\n", reply->str);
  /* Compute the result and send it back as a JSON object */
  mg_printf_http_chunk(nc, "{ \"result\": %lf }", reply->str);
  freeReplyObject(reply);
  mg_send_http_chunk(nc, "", 0); /* Send empty chunk, the end of response */

}

/* Main event handler. */
static void ev_handler(struct mg_connection *nc, int ev, void *p ) {
  redisContext *rc = redisConnect("127.0.0.1", 6379);
  if (rc == NULL || rc->err) {
      if (rc) {
          printf("Error: %s\n", rc->errstr);
          // handle error
      } else {
          printf("Can't allocate redis context\n");
      }
  }
  if (ev == MG_EV_HTTP_REQUEST) {

      struct http_message *hm = (struct http_message *) p;

      if (mg_vcmp(&hm->uri, "/api/v1/get") == 0 && is_equal(&hm->method, &s_get_method)) {

          fprintf(stderr, " requested %.*s\n", (int) hm->uri.len, hm->uri.p);
          handle_get_call(nc, rc);

      } else if (mg_vcmp(&hm->uri, "/api/v1/put") == 0 && is_equal(&hm->method, &s_put_method)) {


      } else if (mg_vcmp(&hm->uri, "/api/v1/post") == 0 && is_equal(&hm->method, &s_post_method)) {

          handle_post_call(nc, hm, rc);

      } else if (mg_vcmp(&hm->uri, "/api/v1/delete") == 0 && is_equal(&hm->method, &s_delete_method)) {

      }


      mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
  }
}

int main(void) {
  struct mg_mgr mgr;
  struct mg_connection *nc;
  mg_mgr_init(&mgr, NULL);

  nc = mg_bind(&mgr, s_http_port, ev_handler);


  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = ".";

  printf("Starting web server on port %s\n", s_http_port);
  while (1) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}

