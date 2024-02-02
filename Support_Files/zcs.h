#ifndef __ZCS_H__
#define __ZCS_H__

#define ZCS_APP_TYPE 1
#define ZCS_SERVICE_TYPE 2

typedef struct {
    char *attr_name;
    char *value;
} zcs_attribute_t;

typedef struct _zcs_node_t {
  struct sockaddr_in address;
  char *name;
  int status;                         // 0 for down, 1 for up
  time_t hearbeat_time;
  struct _zcs_node_t *next;
  zcs_attribute_t *attriutes[];
} zcs_node_t;

typedef struct _node_list_t {
  zcs_node_t *head;
  zcs_node_t *tail;
} node_list_t;

typedef struct ad_node {
  char *name;
  zcs_cb_f cback;
  struct ad_node *next;
} ad_node_t;

typedef struct ad_list {
  ad_node_t *head;
  ad_node_t *tail;
} ad_list_t;

typedef void (*zcs_cb_f)(char *, char *);

int zcs_init(int type);
int zcs_start(char *name, zcs_attribute_t attr[], int num);
int zcs_post_ad(char *ad_name, char *ad_value);
int zcs_query(char *attr_name, char *attr_value, char *node_names[], int namelen);
int zcs_get_attribs(char *name, zcs_attribute_t attr[], int *num);
int zcs_listen_ad(char *name, zcs_cb_f cback);
int zcs_shutdown();
void zcs_log();

#endif

