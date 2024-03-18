#include <errno.h>
#include <fuse.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <dirent.h>
#include "params.h"
#include "log.h"
#include "our_lib.h"

#define FORMATED_ID_MAX_LEN 3
#define MAX_BLOCK_SIZE 4096
#define HASH_SIZE 40
#define ID_LENGTH 3
#define POINTERS_COUNTER_LENGTH 2

char *generate_hash(char *data) {
	unsigned char hash[SHA_DIGEST_LENGTH];
	char *toHexBuffer = malloc(sizeof(char) * SHA_DIGEST_LENGTH * 2 + 1);
	int data_length = strlen(data);
	SHA1((unsigned char *)data, data_length, hash);

	for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
		sprintf((char *)&(toHexBuffer[i * 2]), "%02x", hash[i]);
	}
	toHexBuffer[HASH_SIZE] = '\0';
	return toHexBuffer; /*length of 40*/
}

int compare_hashes(struct bb_state *bbs, char *hash_to_compare) {
	char hash[HASH_SIZE + 1];
	int bytesRead;
	char temp_id[ID_LENGTH];
	char usage[POINTERS_COUNTER_LENGTH + 1];

	char zero[3];
	sprintf(zero, "%02d", 0);

	FILE *hashesfile = fopen(bbs->hashesfile, "ab+");
	if (!hashesfile) {
		log_msg("No hashes file\n");
		return 1;
	}

	/*skip the first id length to read the counter and the hash*/
	fseek(hashesfile, ID_LENGTH, SEEK_SET);

	/*
	 * hashes file contains
	 * id + counter + hash
	 * ex 001019cf95dacd216dcf43da376cdb6cbba6035118921
	 */

	while (fread(usage, POINTERS_COUNTER_LENGTH, 1, hashesfile) == 1) {
		bytesRead = fread(hash, HASH_SIZE, 1, hashesfile);
		if (bytesRead != 1) {
			break; /*EOF*/
		}
		hash[HASH_SIZE] = '\0';
		usage[POINTERS_COUNTER_LENGTH] = '\0';
		if (strcmp(hash, hash_to_compare) == 0 && strcmp(usage, zero) != 0) {
			/*need to get the id (already skipped id and read hash and counter)*/
			fseek(hashesfile, -HASH_SIZE - ID_LENGTH - POINTERS_COUNTER_LENGTH, SEEK_CUR);
			char id[ID_LENGTH + 1];
			fread(id, (ID_LENGTH), 1, hashesfile);
			id[ID_LENGTH] = '\0';
			fclose(hashesfile);
			return atoi(id);
		}
		fread(temp_id, ID_LENGTH, 1, hashesfile);
	}
	fclose(hashesfile);

	/*update counter*/
	return bbs->last_id + 1;
}

void write_hash(struct bb_state *bbs, char *hash, int id) {
	int line_total_length = ID_LENGTH + POINTERS_COUNTER_LENGTH + HASH_SIZE;
	FILE *hashesfile = fopen(bbs->hashesfile, "rb+");
	char line_to_write[ID_LENGTH + POINTERS_COUNTER_LENGTH + HASH_SIZE + 1];
	if (!hashesfile) {
		log_msg("write_hash error opening hashes file\n");
		return;
	}
	/*if the id is smaller than the last id, then its an id of an existing block*/
	if (id > bbs->last_id) {
		/*writing a new hash*/
		log_msg("Writing a new hash\n");
		sprintf(line_to_write, "%03d01%s", id, hash); /*01 indicates how many files use this*/
		fseek(hashesfile, 0, SEEK_END);
	}
	else {
		/*ids start from 1 so we have to reduce by 1 to get the right offset*/
		int offset = (id - 1) * line_total_length;
		fseek(hashesfile, offset + ID_LENGTH, SEEK_SET);

		char usage[POINTERS_COUNTER_LENGTH + 1];
		fread(usage, POINTERS_COUNTER_LENGTH, 1, hashesfile);
		usage[POINTERS_COUNTER_LENGTH] = '\0';
		int counter = atoi(usage);
		counter++;
		fseek(hashesfile, -ID_LENGTH - POINTERS_COUNTER_LENGTH, SEEK_CUR);
		sprintf(line_to_write, "%03d%02d%s", id, counter, hash);
	}

	log_msg("\nHash to write: %s\n", line_to_write);
	fwrite(line_to_write, 1, strlen(line_to_write), hashesfile);
	fclose(hashesfile);
	free(hash);
}

int generate_block(struct bb_state *bbs, const char *dirname, char *block_data) {
	char next_block_name[PATH_MAX];
	FILE *next_block_file;

	char *hash = generate_hash(block_data);
	int lastID = compare_hashes(bbs, hash);
	int is_new = (lastID > bbs->last_id);
	if (!is_new) {
		/*just need to update the counter of the existing block*/
		write_hash(bbs, hash, lastID);
		return lastID;
	}

	/*create the block name ex hiddendirpath/001.txt*/
	sprintf(next_block_name, "%s/%03d.txt", bbs->hiddendir, bbs->last_id + 1);
	next_block_file = fopen(next_block_name, "wb+");
	if (!next_block_file) {
		log_msg("\nCan't open %s\n", next_block_name);
		return 0;
	}
	fseek(next_block_file, 0, SEEK_SET);
	write_hash(bbs, hash, bbs->last_id + 1);
	bbs->last_id += 1;
	fwrite(block_data, strlen(block_data), 1, next_block_file);
	fclose(next_block_file);
	return bbs->last_id;
}

char *read_block(struct bb_state *bbs, char *id, int size) {
	char *block_name, *block;
	FILE *block_file;

	block_name = malloc(sizeof(char) * (strlen(bbs->hiddendir) + FORMATED_ID_MAX_LEN + 5));
	if (!block_name) {
		return NULL;
	}
	/*create the block name ex hiddendirpath/001.txt*/
	sprintf(block_name, "%s/%s.txt", bbs->hiddendir, id);
	block_file = fopen(block_name, "rb");

	if (!block_file) {
		log_msg("Error opening block file: %s\n", block_name);
		free(block_name);
		return NULL;
	}

	/*get the size of the file*/
	fseek(block_file, 0L, SEEK_END);
	int real_size = ftell(block_file);
	rewind(block_file);

	block = malloc(real_size + 1);
	if (!block) {
		log_msg("Error allocating memory for the block: %s\n", id);
		free(block_name);
		return NULL;
	}

	/*read content*/
	fread(block, 1, real_size, block_file);
	block[real_size] = '\0';
	fclose(block_file);
	free(block_name);
	return block;
}

int get_real_size(struct bb_state *bbs, const char *fpath) {
	FILE *pointers_file = fopen(fpath, "rb");
	int readBytes;
	struct stat st;
	char id[ID_LENGTH + 1];
	char filename[PATH_MAX];
	int total_size = 0;
	if (!pointers_file) {
		return 0;
	}
	fseek(pointers_file, 0, SEEK_SET);

	/*read all pointers and add their real size*/
	while (1) {
		readBytes = fread(id, 1, ID_LENGTH, pointers_file);
		if (readBytes < 3) {
			break;
		}
		id[ID_LENGTH] = '\0';
		sprintf(filename, "%s/%s.txt", bbs->hiddendir, id);
		stat(filename, &st);
		total_size += st.st_size;
	}
	fclose(pointers_file);
	return total_size;
}

char *read_file(struct bb_state *bbs, const char *fpath) {
	int size = get_real_size(bbs, fpath);
	char *buffer;
	char *file_data;
	int bytesRead;
	int block = 0;
	char id[ID_LENGTH + 1];
	int num_of_blocks;

	/*pointers file content ex. 001002003*/
	FILE *pointers_file = fopen(fpath, "rb");
	if (!pointers_file) {
		log_msg("Error opening pointers_file\n");
		return NULL;
	}

	/*get number of blocks*/
	fseek(pointers_file, 0L, SEEK_END);
	num_of_blocks = ftell(pointers_file) / ID_LENGTH;
	fseek(pointers_file, 0L, SEEK_SET);

	file_data = malloc(sizeof(char) * size + 10);
	if (!file_data) {
		log_msg("\nread_file: error allocating file_data\n");
		return NULL;
	}
	if (!pointers_file) {
		free(file_data);
		log_msg("\nread_file: error opening pointers_file\n");
		return NULL;
	}

	file_data[0] = '\0';
	for (block = 0; block < num_of_blocks; block++) {
		/*get block id*/
		bytesRead = fread(id, 1, ID_LENGTH, pointers_file);
		if (bytesRead != ID_LENGTH) {
			break;
		}
		id[ID_LENGTH] = '\0';

		/*get block content*/
		buffer = read_block(bbs, id, size);
		/*append the content*/
		file_data = strcat(file_data, buffer);
		free(buffer);
	}
	fclose(pointers_file);
	return file_data;
}

char *write_blocks(const char *buf, struct bb_state *bbs, const char *dirname, size_t size) {
	char *formatted_id = malloc(sizeof(char) * ID_LENGTH + 1);
	int i;
	int id;
	char block_data[MAX_BLOCK_SIZE + 1];

	formatted_id[0] = '\0'; /*used in strcat*/
	for (i = 0; i < size; i++) {
		block_data[i] = buf[i];
	}
	block_data[i] = '\0'; /*use strlen in generate_block()*/
	id = generate_block(bbs, dirname, block_data);
	sprintf(formatted_id, "%03d", id);

	return formatted_id;
}

int get_initial_id_number(const char *path) {
	int file_count = 0;
	DIR *dirp;
	struct dirent *entry;

	dirp = opendir(path);

	if (!dirp) {
		log_msg("Hidden dir not found\n");
		return file_count;
	}
	while ((entry = readdir(dirp)) != NULL) {	
		/*If the entry is a regular file*/
		if (entry->d_type == DT_REG) { 
			file_count++;
		}
	}

	closedir(dirp);
	return file_count == 0 ? file_count : file_count - 1; /*one file is the hashes file, so -1*/
}

void delete_blocks(char *fpath, struct bb_state *bbs) {
	int line_total_length = HASH_SIZE + ID_LENGTH + POINTERS_COUNTER_LENGTH;
	char id_to_delete[ID_LENGTH + 1];
	char usage[POINTERS_COUNTER_LENGTH + 1], hash[HASH_SIZE + 1];
	int offset;
	char updated_hash[line_total_length + 1];
	char block_name[strlen(bbs->hiddendir) + FORMATED_ID_MAX_LEN + 50];
	int id_number;

	FILE *hashesfile = fopen(bbs->hashesfile, "rb+");
	FILE *pointers_file = fopen(fpath, "r");
	if (!hashesfile) {
		log_msg("\nNo hashes file\n");
		fclose(pointers_file);
		return;
	}
	if (!pointers_file) {
		log_msg("\nNo pointers file\n");
		fclose(hashesfile);
		return;
	}
	rewind(pointers_file);

	while (fread(id_to_delete, 1, ID_LENGTH, pointers_file) == 3) {
		/*get id*/
		id_to_delete[ID_LENGTH] = '\0';
		id_number = atoi(id_to_delete);

		/*get right offset. id start from 1 and not 0*/
		offset = (id_number - 1) * line_total_length + ID_LENGTH;
		fseek(hashesfile, offset, SEEK_SET);
		fread(usage, POINTERS_COUNTER_LENGTH, 1, hashesfile);
		usage[POINTERS_COUNTER_LENGTH] = '\0';
		if (atoi(usage) == 0) {
			/*
			 * keep the hash on the hashes file even if no file needs it.
			 * helps to get the right offset
			 */
			continue;
		}
		/*only 1 file uses, unlink this file*/
		if (atoi(usage) == 1) {
			sprintf(block_name, "%s/%s.txt", bbs->hiddendir, id_to_delete);
			fclose(fopen(fpath, "w")); /*remove pointers*/
			unlink(block_name);
		}
		
		/*update pointers counter*/
		fread(hash, HASH_SIZE, 1, hashesfile);
		hash[HASH_SIZE] = '\0';
		sprintf(updated_hash, "%s%02d%s", id_to_delete, (atoi(usage) - 1), hash);
		fseek(hashesfile, -line_total_length, SEEK_CUR);
		fwrite(updated_hash, 1, strlen(updated_hash), hashesfile);
	}

	rewind(pointers_file);
	fclose(hashesfile);
	fclose(pointers_file);
}
