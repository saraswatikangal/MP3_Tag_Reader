#ifndef MP3_TAG_READER_H
#define MP3_TAG_READER_H

#include <stdio.h>
#include "types.h"

#define FRAME_COUNT 6

typedef struct
{
    FILE *fptr_mp3;
    char *filename;

}MP3TagInfo;

typedef struct
{
    FILE *fptr_src_mp3;
    FILE *fptr_temp_mp3;

    char *src_filename;
    char *temp_filename;

    char *edit_option;
    char *new_data;
}EditInfo;

/* Operation */
/* Function to check the operation type */
OperationType check_operation_type(char *symbol);

/* MP3 file */
/*Function to check MP3 file extension */
Status check_mp3_file(char *filename);

/* Function to open the mp3 file */
Status open_mp3_file(MP3TagInfo *mp3Info);

/* ID3 header */
/*Function to check ID3 header */
Status check_id3_header(MP3TagInfo *mp3Info);

/*Function to check ID3 version */
Status check_id3_version(MP3TagInfo *mp3Info);

/*Function to check ID3 flags */
Status check_id3_flags(MP3TagInfo *mp3Info);

/*Function to read ID3 tag size */
Status read_id3_tag_size(MP3TagInfo *mp3Info, unsigned int *tag_size);

/* Frame */
/*Function to read frame tag */
Status read_frame_tag(MP3TagInfo *mp3Info, char *tag);

/*Function to read frame size */
Status read_frame_size(MP3TagInfo *mp3Info, unsigned int *frame_size);

/*Function to read frame flags */
Status read_frame_flags(MP3TagInfo *mp3Info);

/*Function to check frame tag */
Status check_frame_tag(char *tag);

/*Function to read frame content */
Status read_frame_content(MP3TagInfo *mp3Info, unsigned int frame_size,char *data);

/* View */
/* view operation */
Status view_operation(MP3TagInfo *mp3Info);

/* Edit */
/* Edit operation */
Status edit_operation(EditInfo *editInfo);

#endif