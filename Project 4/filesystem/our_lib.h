#include "params.h"

/*
 * generate a hash from the give data
 * hash has a length of 40
 */
char *generate_hash(char *data);

/* 
 * read all hashes on the hashesfile and compares them with the give hash
 * if two hashes are identical and also the counter bigger than 0
 * then its a match
 */
int compare_hashes(struct bb_state *bbs, char *hash_to_compare);

/* 
 * write the given hash to the hashes file ex. 001009cf95dacd216dcf43da376cdb6cbba6035118921
 * and update the counter
 */
void write_hash(struct bb_state *bbs, char *hash, int id);

/*generate a new block*/
int generate_block(struct bb_state *bbs, const char *dirname, char *block_data);

/*read a block*/
char *read_block(struct bb_state *bbs, char *id, int size);

/*
 * uses read_block to read all blocks and append their data together
 * returns the content of the file
 */
char *read_file(struct bb_state *bbs, const char *fpath);

/*gets a buffer and write its content to a block file*/
char *write_blocks(const char *buf, struct bb_state *bbs, const char *dirname, size_t size);

/*get the real size of the file given*/
int get_real_size(struct bb_state *bbs, const char *fpath);

/*read the number of files in the .real_storage*/
int get_initial_id_number(const char *path);

/*delete the blocks (if they are not used by other files)*/
void delete_blocks(char *fpath, struct bb_state *bbs);
