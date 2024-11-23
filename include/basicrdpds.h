/*
 * struct mem_node: a pointer to this structure is returned from the call to pds_mem().
 * It is a linked list of information about the member - each array contains a member
 * name and possibly user data. Each next pointer points * to the next member, except the last
 * next member which points to NULL.
*/

#define NAMELEN 8      /* Length of a MVS member name */

typedef struct mem_node {
  struct mem_node *next;
  char name[NAMELEN+1];
  char userdata_len;
  char userdata[64];
};

struct mem_node* pds_mem(const char *pds);
void free_mem(struct mem_node* nodes);
