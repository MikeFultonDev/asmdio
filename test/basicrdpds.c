/*
* struct mem_node* pds_mem(const char *pds):
* pds must be a fully qualified pds name, for example,
* ID.PDS.DATASET * returns a * pointer to a linked list of
* nodes.  Each node contains a member of the * pds and a
* pointer to the next node.  If no members exist, the pointer
* is NULL.
*
* Note:  Behavior is undefined if pds is the name of a sequential file.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "basicrdpds.h"

/*
 * RECORD: each record of a pds will be read into one of these structures.
 *         The first 2 bytes is the record length, which is put into 'count',
 *         the remaining 254 bytes are put into rest. Each record is 256 bytes long.
*/

#define RECLEN  254

typedef struct {
          unsigned short int count;
          char rest[RECLEN];
          } RECORD;

/* Local function prototypes   */

static int gen_node(struct mem_node** node, RECORD *rec, struct mem_node** last_ptr);
static char *add_name(struct mem_node** node, const char *name, struct mem_node** last_ptr, const char* userdata, char userdata_len);

struct mem_node* pds_mem(const char *pds) 
{

  FILE *fp;
  int bytes;
  struct mem_node* node, *last_ptr;
  RECORD rec;
  int list_end;
  char *qual_pds;

  node = NULL;
  last_ptr = NULL;

  /*
   * Allocate a new variable, qual_pds, which will be the same as pds, except
   * with single quotes around it, i.e. ID.PDS.DATASET ==> 'ID.PDS.DATA SET'
  */

  qual_pds = (char *)malloc(strlen(pds) + 5);
  if (qual_pds == NULL) {
    fprintf(stderr,"malloc failed for %d bytes\n",strlen(pds) + 3);
    exit(-1);
  }
  sprintf(qual_pds,"//'%s'",pds);
  /*
   * Open the pds in binary read mode. The PDS directory will be read one
   * record at a time until either the end of the directory or end-of-file
   * is detected. Call up gen_node() with every record read, to add member
   * names to the linked list
  */

  fp = fopen(qual_pds,"rb");
  if (fp == NULL)
    return(NULL);

  do {    
    bytes = fread(&rec, 1, sizeof(rec), fp);
    if ((bytes != sizeof(rec)) && !feof(fp)) {
      perror("FREAD:");
      fprintf(stderr,"Failed in %s, line %d\n"
             "Expected to read %d bytes but read %d bytes\n",
              __FILE__,__LINE__,sizeof(rec), bytes);
      exit(-1);
    }

    list_end = gen_node(&node, &rec, &last_ptr);

  } while (!feof(fp) && !list_end);
  fclose(fp);
  free(qual_pds);
  return(node);
}
/*
 * GEN_struct mem_node() processes the record passed. The main loop scans through the
 * record until it has read at least rec->count bytes, or a directory end
 * marker is detected.
 *
 * Each record has the form:
 *
 * +------------+------+------+------+------+----------------+
 * + # of bytes ¦Member¦Member¦......¦Member¦  Unused        +
 * + in record  ¦  1   ¦  2   ¦      ¦  n   ¦                +
 * +------------+------+------+------+------+----------------+
 *  ¦--count---¦¦-----------------rest-----------------------¦
 *  (Note that the number stored in count includes its own
 *   two bytes)
 *
 * And, each member has the form:
 *
 * +--------+-------+----+-----------------------------------+
 * + Member ¦TTR    ¦info¦                                   +
 * + Name   ¦       ¦byte¦  User Data TTRN's (halfwords)     +
 * + 8 bytes¦3 bytes¦    ¦                                   +
 * +--------+-------+----+-----------------------------------+
*/
#define TTRLEN 3      /* The TTR's are 3 bytes long */

/*
 * bit 0 of the info-byte is '1' if the member is an alias,
 * 0 otherwise. ALIAS_MASK is used to extract this information
*/

#define ALIAS_MASK ((unsigned int) 0x80)

/*
 * The number of user data half-words is in bits 3-7 of the info byte.
 * SKIP_MASK is used to extract this information.  Since this number is
 * in half-words, it needs to be double to obtain the number of bytes.
*/
#define SKIP_MASK ((unsigned int) 0x1F)

/*
 * 8 hex FF's mark the end of the directory
*/

char *endmark = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
static int gen_node(struct mem_node** node, RECORD *rec, struct mem_node** last_ptr) 
{

   char *ptr, *name;
   int skip, count = 2;
   unsigned int info_byte, alias, ttrn;
   char ttr[TTRLEN];
   int list_end = 0;

   ptr = rec->rest;

   while(count < rec->count) {
     if (!memcmp(ptr,endmark,NAMELEN)) {
       list_end = 1;
       break;
     }

     /* member name */
     name = ptr;
     ptr += NAMELEN;

     /* ttr */
     memcpy(ttr,ptr,TTRLEN);
     ptr += TTRLEN;

     /* info_byte */
     info_byte = (unsigned int) (*ptr);
     alias = info_byte & ALIAS_MASK;
     skip = (info_byte & SKIP_MASK) * 2 + 1;
     if (!alias) add_name(node,name,last_ptr,ptr,skip);
     ptr += skip;
     count += (TTRLEN + NAMELEN + skip);
   }
   return(list_end);
}

/*
 * ADD_NAME: Add a new member name to the linked node. The new member is
 * added to the end so that the original ordering is maintained.
*/

static char *add_name(struct mem_node** node, const char *name, struct mem_node** last_ptr, const char* userdata, char userdata_len) 
{

  struct mem_node* newnode;

  /*
   * malloc space for the new node
  */

  newnode = (struct mem_node*)malloc(sizeof(struct mem_node));
  if (newnode == NULL) {
    fprintf(stderr,"malloc failed for %d bytes\n",sizeof(struct mem_node));
    exit(-1);
  }

   /* copy the name into the node and NULL terminate it */

  memcpy(newnode->name,name,NAMELEN);
  newnode->name[NAMELEN] = '\0';
  newnode->next = NULL;

  memcpy(newnode->userdata, userdata, userdata_len);
  newnode->userdata_len = userdata_len;

  /*
   * add the new node to the linked list
  */

  if (*last_ptr != NULL) {
    (*last_ptr)->next = newnode;
    *last_ptr = newnode;
  }
  else {
    *node = newnode;
    *last_ptr = newnode;
  }
  return(newnode->name);
}
/*
 * FREE_MEM: This function should be used
 * as soon as you are finished using the linked list. It frees the storage
 * allocated by the linked list.
*/

void free_mem(struct mem_node* node) 
{
  struct mem_node* next_node=node;

  while (next_node != NULL) {
     next_node = node->next;
     free(node);
     node = next_node;
  }
  return;
}

#pragma pack(full)
struct ispf_disk_stats {
  char encoded_userdata_length;
  unsigned char ver_num;
  unsigned char mod_num;
  int sclm:1;
  int reserve_a:1;
  int extended:1;
  int reserve_b:5;
  unsigned char pd_mod_seconds;
  unsigned char create_century;
  char pd_create_julian[3];
  unsigned char mod_century;
  char pd_mod_julian[3];
  unsigned char mod_hours;
  unsigned char mod_minutes;

  unsigned short curr_num_lines;
  unsigned short init_num_lines;
  unsigned short mod_num_lines;

  char userid[NAMELEN];

  /* following is available only in extended format */
  unsigned int full_curr_num_lines;
  unsigned int full_init_num_lines;
  unsigned int full_mod_num_lines;
};
#pragma pack(pop)

struct ispf_stats {
  struct tm create_time;
  struct tm mod_time;
  unsigned short curr_num_lines;
  unsigned short init_num_lines;
  unsigned short mod_num_lines;
  unsigned char userid[NAMELEN+1];
  unsigned char ver_num;
  unsigned char mod_num;
  unsigned char sclm;
};

/*
 * msf - need to implement check of ranges of values
 */
static int valid_ispf_disk_stats(const struct ispf_disk_stats* ids)
{
  return 0; 
}

const struct tm zerotime = { 0 };
static void set_create_time(struct ispf_stats* is, struct ispf_disk_stats* id)
{
  is->create_time = zerotime;
  if (id->create_century == 1) {
    is->create_time.tm_year = 100;
  }
  /*
   * msf - write code to convert julian date to year/month
   */
}

static void set_mod_time(struct ispf_stats* is, struct ispf_disk_stats* id)
{
  is->mod_time = zerotime;
  if (id->mod_century == 1) {
    is->mod_time.tm_year = 100;
  }
  /*
   * msf - write code to convert julian date to year/month and packed decimal hours/minutes to hours/minutes
   */
}

static int ispf_stats(const struct mem_node* np, struct ispf_stats* is)
{
  struct ispf_disk_stats* id = (struct ispf_disk_stats*) (np->userdata);
  int rc = valid_ispf_disk_stats(id);
  if (rc) {
    return rc;
  }
  set_create_time(is, id);
  set_mod_time(is, id);
  memcpy(is->userid, id->userid, NAMELEN);
  is->userid[NAMELEN] = '\0';

  is->ver_num = id->ver_num;
  is->mod_num = id->mod_num;
  is->sclm = id->sclm;

  if (id->extended) {
    is->curr_num_lines = id->full_curr_num_lines;
    is->init_num_lines = id->full_init_num_lines;
    is->mod_num_lines = id->full_mod_num_lines;
  } else {
    is->curr_num_lines = id->curr_num_lines;
    is->init_num_lines = id->init_num_lines;
    is->mod_num_lines = id->mod_num_lines;
  }
    
  return 0;
}

int main(int argc, char* argv[])
{
  if (argc < 2) {
    fprintf(stderr, "Usage: basicrdpds <dataset>\n");
    return 4;
  }
  char* dataset = argv[1];
  struct mem_node* np = pds_mem(dataset);
  while (np) {
    if (np->userdata_len == 31 || np->userdata_len == 41) {
      /* ISPF USER DATA */
      /* https://tech.mikefulton.ca/ISPFStatsLayout */
      struct ispf_stats is;
      int rc = ispf_stats(np, &is);
      if (!rc) {
        char crttime_buff[4+1+2+1+2+1];                /* YYYY/MM/DD       */
        char modtime_buff[4+1+2+1+2+1+2+1+2+1];        /* YYYY/MM/DD HH:MM */
        strftime(crttime_buff, sizeof(crttime_buff), "%Y/%m/%d", &is.create_time);
        strftime(modtime_buff, sizeof(modtime_buff), "%Y/%m/%d %H:%M", &is.create_time);
        printf(" %s %d.%d %s %s %d %d %d %s\n", 
         np->name, 
         is.ver_num, is.mod_num, crttime_buff, modtime_buff, is.curr_num_lines, is.mod_num_lines, is.init_num_lines, is.userid);
      } else {
        printf(" %s %d\n", np->name, np->userdata_len);
      }
    } else {
      printf(" %s %d\n", np->name, np->userdata_len);
    }
    np = np->next;
  }
  return 0;
}
