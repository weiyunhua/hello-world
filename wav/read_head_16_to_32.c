#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>
#include"wav.h"

/* Absolute min volume in dB (can be represented in single precision normal float value) */
#define VOLUME_MIN_DB (-758)

int strncasecmpy(char *s1, char *s2, int n) {
	while (--n >= 0 && toupper((uint16_t)*s1) == toupper((uint16_t)*s2++))
	if (*s1++ == ' ') return 0;
	return (n < 0 ? 0 : toupper((uint16_t)*s1) - toupper((uint16_t)*--s2));
}

wav_t *wav_open(char *file_name) {
	wav_t *wav = NULL;
	uint8_t buffer[256];
	int32_t read_len = 0;
	int32_t offset = 0;

	if (NULL == file_name) {
		printf("file_name is NULL\n");
		return NULL;
	}
	
	wav = (wav_t *)malloc(sizeof(wav_t));
	
	if (NULL == wav) {
		printf("malloc wav failedly\n");
		return NULL;
	}
	
	memset(wav, 0, sizeof(wav_t));
	wav->fp = fopen(file_name, "rw+");
	
	if (NULL == wav->fp) {
		printf("fopen %s failedly\n", file_name);
		free(wav);
		return NULL;
	}

	/* handle RIFF WAVE chunk */
	read_len = fread(buffer, 1, 12, wav->fp);
	if (read_len < 12){
		printf("error wav file\n");
		wav_close(&wav);
		return NULL;
	}
	
	if (strncasecmpy("RIFF", buffer, 4)) {
		printf("error wav file\n");
		wav_close(&wav);
		return NULL;
	}
	
	memcpy(wav->riff.id, buffer, 4);
	wav->riff.size = *(int32_t *)(buffer + 4);
	
	if (strncasecmpy("WAVE", buffer + 8, 4)) {
		printf("error wav file\n");
		wav_close(&wav);
		return NULL;
	}
	
	memcpy(wav->riff.type, buffer + 8, 4);
	wav->file_size = wav->riff.size + 8;
	offset += 12;
	
	while(1) {
		uint8_t id_buffer[5] = {0};
		int32_t tmp_size = 0;

		read_len = fread(buffer, 1, 8, wav->fp);
			if (read_len < 8) {
			printf("error wav file\n");
			wav_close(&wav);
			return NULL;
		}
		
		memcpy(id_buffer, buffer, 4);
		tmp_size = *(int32_t *)(buffer + 4);

		if (0 == strncasecmpy("FMT", id_buffer, 3)) {
			memcpy(wav->format.id, id_buffer, 3);
			wav->format.size = tmp_size;
			read_len = fread(buffer, 1, tmp_size, wav->fp);
			if(read_len < tmp_size) {
				printf("error wav file\n");
				wav_close(&wav);
				return NULL;
			}
			
			wav->format.compression_code = *(uint16_t *)buffer;
			wav->format.channels = *(uint16_t *)(buffer + 2);
			wav->format.samples_per_sec = *(int32_t *)(buffer + 4);
			wav->format.avg_bytes_per_sec = *(int32_t *)(buffer + 8);
			wav->format.block_align = *(uint16_t *)(buffer + 12);
			wav->format.bits_per_sample = *(uint16_t *)(buffer + 14);
		} else if (0 == strncasecmpy("DATA", id_buffer, 4)) {
			memcpy(wav->data.id, id_buffer, 4);
			wav->data.size = tmp_size;
			offset += 8;
			wav->data_offset = offset;
			wav->data_size = wav->data.size;
			break;
		} else {
			printf("unhandled chunk: %s, size: %d\n", id_buffer, tmp_size);
			fseek(wav->fp, tmp_size, SEEK_CUR);
		}
		
		offset += 8 + tmp_size;
	}
	

	return wav;
}

void wav_16_to_32(char *file_des, char *file_src, float coe) {
	int32_t i, num;
	int32_t data_size_des, wav_des_size, data_size_src;
	
	static wav_t *wav_src = NULL;
	static wav_t *wav_des = NULL;
	static uint8_t head[256] = {0};
	static uint8_t tmp_head[256] = {0};
	static uint16_t tmp_data = 0;
	static int32_t tmp_data1 = 0;
	static uint8_t wav_size[4] = {0};
	static uint8_t data_size[4] = {0};
	
	/* acquerir file_src data, data_size and soon... */
	wav_src = wav_open(file_src);
	
	wav_des_size = wav_src->data_size*2 + wav_src->data_offset;
	data_size_src = wav_src->data_size;
	data_size_des = (wav_src->data_size * 2);
	
	wav_des = (wav_t *)malloc(wav_src->data_offset);
	
	memset(wav_des, 0, wav_src->data_offset);
	
	wav_src->fp = fopen(file_src, "r");
	wav_des->fp = fopen(file_des, "awb+");
	
	if (NULL == wav_des->fp) {
		printf("fopen %s failedly\n", file_des);
		free(wav_des);
		return ;
	}	
	
	for (i=0; i<4; i++) {
		wav_size[3-i] = (uint8_t)((wav_des_size >> 8*i) & 0xFF);
		data_size[3-i] = (uint8_t)((data_size_des >> 8*i) & 0xFF);
	}
	
	/* num = wav_src->data_offset - 4 */
	num = wav_src->data_offset;
	data_size_src = wav_src->data_size;
	
	fread(head, 1, num, wav_src->fp);
	
	/* change file_size && data_size */
	head[34] = 0x20;	
	for (i=0; i<4; i++) {
		head[4+i] = wav_size[3-i];
		head[wav_src->data_offset-4+i] = data_size[3-i];
	}

	/* copy head data */
	fwrite(&head, 1, num, wav_des->fp);

	/* copy audio data */
	for (i=0; i<data_size_src; i+=2) {
		fseek(wav_src->fp, i+num, SEEK_SET);
		fread(&tmp_data, sizeof(uint16_t), 1, wav_src->fp);
		tmp_data1 = ((int32_t)tmp_data << 16)*coe*10/10;
		fwrite(&tmp_data1, sizeof(int32_t), 1, wav_des->fp);
	}
	
	printf("The file after 16bit to 32bit info\n");
}

void wav_close(wav_t **wav) {
	wav_t *tmp_wav;
	if (NULL == wav) {
		return ;
	}

	tmp_wav = *wav;
	if (NULL == tmp_wav) {
		return ;
	}

	if (NULL != tmp_wav->fp) {
		fclose(tmp_wav->fp);
	}
	
	free(tmp_wav);
	*wav = NULL;
}

void wav_dump(wav_t *wav) {
	printf("###### Read File Head Data Begin ######\n");
	
	printf("\nThis file length is %d\n", wav->file_size);
	printf("\n------ RIFF WAVE Chunk ------\n");
	printf("id: %s\n", wav->riff.id);
	printf("size: %d\n", wav->riff.size);
	printf("type: %s\n", wav->riff.type);

	printf("\n------ FORMAT Chunk ------\n");
	printf("id: %s\n", wav->format.id);
	printf("size: %d\n", wav->format.size);
	
	if (wav->format.compression_code == 0) {
		printf("compression: Unknown\n");
	} else if (wav->format.compression_code == 1) {
		printf("compression: PCM/uncompressed\n");
	} else {
		printf("compression: Unknown\n");
	}
	
	printf("channels: %d\n", wav->format.channels);
	printf("samples: %d\n", wav->format.samples_per_sec);
	printf("avg_bytes_per_sec: %d\n", wav->format.avg_bytes_per_sec);
	printf("block_align: %d\n", wav->format.block_align);
	printf("bits_per_sample: %d\n", wav->format.bits_per_sample);

	printf("\n------ DATA Chunk ------\n");
	printf("id: %s\n", wav->data.id);
	printf("size: %d\n", wav->data.size);
	printf("data offset: %d\n", wav->data_offset);
	
	printf("\n###### Read File Head Data End ######\n");
}

static inline float DbToAmpl(float decibels)
{
    if (decibels <= VOLUME_MIN_DB) {
        return 0.0f;
    }
	
    return exp( decibels * 0.115129f); // exp( dB * ln(10) / 20 )
}

static inline float AmplToDb(float amplification)
{
    if (amplification == 0) {
        return VOLUME_MIN_DB;
    }
	
    return 20 * log10(amplification);
}

int main(int argc, char **argv) {
	float num;
	float db;
	char* file_src = NULL;
	char* file_des = NULL;	
	wav_t *wav = NULL;
	
	if (argc == 1) {
		/* num = 1; */
		db = 0;
		file_src = "1.5MBPS-44.1-2ch.wav";
		file_des = "16_to_32bits.wav";
	} else if (argc == 4){
		/* sscanf(argv[1], "%f", &num); */
		sscanf(argv[1], "%f", &db);
		file_src = argv[2];
		file_des = argv[3];
	} else {
		printf("please input three parametre!\n");
		/* printf("parametre one: 0~1, experimer the coefficient of audio son\n"); */
		printf("parametre one: -x, experimer the coefficient db of audio son\n");
        printf("parametre two: file_src, 16 bit src file\n");
        printf("parametre three: file_des, after 16bit to 32bit des file\n");
        exit(-1);
	}
	
	num = DbToAmpl(db);
	
	wav_16_to_32(file_des, file_src, num);
	
	wav = wav_open(file_des);
	
	if (NULL != wav) {
		wav_dump(wav);		
		wav_close(&wav);
	}
	
	wav_close(&wav);

	return 0;
}