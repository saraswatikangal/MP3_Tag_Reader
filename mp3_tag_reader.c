#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mp3_tag_reader.h"

/* Function to check operation type */
OperationType check_operation_type(char *symbol)
{
    if(strcmp(symbol, "-v") == 0)
    {
        return e_view;
    }
    else if(strcmp(symbol, "-e") == 0)
    {
        return e_edit;
    }
    else if(strcmp(symbol,"-h") == 0 ||
            strcmp(symbol, "--help") == 0)
    {
        return e_help;
    }
    else
    {
        return e_unsupported;
    }
}

/*Function to check MP3 file extension */
Status check_mp3_file(char *filename)
{
    char *ptr;

    ptr = strrchr(filename, '.');

    if(ptr != NULL && strcmp(ptr, ".mp3") == 0)
    {
        return e_success;
    }

    printf("Error: Input file is not an MP3 file\n");

    return e_failure;
}

/* Function to open the MP3 file */
Status open_mp3_file(MP3TagInfo *mp3Info)
{
    mp3Info->fptr_mp3 = fopen(mp3Info->filename,"rb");

    if(mp3Info->fptr_mp3 == NULL)
    {
        printf("ERROR:Unable to open MP3 file\n");

        return e_failure;
    }
    return e_success;
}

/*Function to check ID3 header/signature*/
Status check_id3_header(MP3TagInfo *mp3Info)
{
    char header[4];

    /*Read first 3 bytes from MP3 file*/
    if(fread(header, 3, 1, mp3Info->fptr_mp3) != 1)
    {
        printf("Error: Unable to read ID3 header\n");

        return e_failure;
    }
    header[3] = '\0';

    if(strcmp(header, "ID3") == 0)
    {
        return e_success;
    }
    printf("Error:Invalid ID3 header\n");

    return e_failure;
}

/*Function to check ID3 version*/
Status check_id3_version(MP3TagInfo *mp3Info)
{
    unsigned char version[2];

    /*Read version and revision from ID3 header*/
    if(fread(version, 1, 2,mp3Info->fptr_mp3)!=2)
    {
        printf("Error: Unable to read ID3 version\n");

        return e_failure;
    }
    /* Check ID3v2.3 */
    if(version[0] == 3 && version[1] == 0)
    {
        return e_success;
    }
    printf("Error:Unsupported ID3 version\n");

    return e_failure;
}

/*Function to check check ID3 flags */
Status check_id3_flags(MP3TagInfo *mp3Info)
{
    unsigned char flags;

    /*Read flags byte from ID3 header */
    if(fread(&flags, 1, 1, mp3Info->fptr_mp3) !=1 )
    {
        printf("Error: Unable to read ID3 flags\n");

        return e_failure;
    }
    return e_success;
}

Status read_id3_tag_size(MP3TagInfo *mp3Info, unsigned int *tag_size)
{
    unsigned char size[4];

    /*Read 4 bytes of ID3 tag size */
    if(fread(size, 1, 4, mp3Info->fptr_mp3) != 4)
    {
        printf("Error: Unable to read ID3 tag size\n");

        return e_failure;
    }
    /* Convert synchsafe integer */
    *tag_size = ((size[0] & 0x7F) << 21)|
                ((size[1] & 0x7F) << 14) |
                ((size[2] & 0x7F) << 7) |
                (size[3] & 0x7F);
    
    //printf("ID3 Tag Size : %u bytes\n",*tag_size);

    return e_success;
}


Status read_frame_tag(MP3TagInfo *mp3Info, char *tag)
{
    if(fread(tag, 1, 4, mp3Info->fptr_mp3) != 4)
    {
        printf("Error: Unable to read frame tag\n");

        return e_failure;
    }
    tag[4] = '\0';

    return e_success;
}

/*Function to read frame size */
Status read_frame_size(MP3TagInfo *mp3Info, unsigned int *frame_size)
{
    unsigned char size[4];

    /*Read 4 bytes of frame size */
    if(fread(size, 1, 4, mp3Info->fptr_mp3) != 4)
    {
        printf("Error : Unable to read frame size\n");

        return e_failure;
    }

     /*
     * ID3v2.3 frame size is
     * stored in big-endian format.
     */
    *frame_size = ((unsigned int)size[0] << 24) |
                    ((unsigned int)size[1] << 16) |
                    ((unsigned int)size[2] << 8) |
                    (unsigned int)size[3];

    return e_success;
}

/*Function to read frame flags */
Status read_frame_flags(MP3TagInfo *mp3Info)
{
    unsigned char flags[2];

    /*Read 2 bytes of frame flags */
    if(fread(flags, 1, 2, mp3Info->fptr_mp3) != 2)
    {
        printf("Error: Unable to read frame flags\n");

        return e_failure;
    }

    return e_success;
}
/* Function to check frame tag */
Status check_frame_tag(char *tag)
{
    if(strcmp(tag, "TIT2") == 0)
    {
        return e_success;
    }
    if(strcmp(tag, "TPE1") == 0)
    {
        return e_success;
    }
    if(strcmp(tag, "TALB") == 0)
    {
        return e_success;
    }
    if(strcmp(tag, "TYER") == 0)
    {
        return e_success;
    }
    if(strcmp(tag, "TCON") == 0)
    {
        return e_success;
    }
    if(strcmp(tag, "COMM") == 0)
    {
        return e_success;
    }

    return e_failure;
}

/* Function to read frame content
 * Function to read frame content */
Status read_frame_content(MP3TagInfo *mp3Info,
                          unsigned int frame_size,
                          char *data)
{
    if(fread(data, 1, frame_size, mp3Info->fptr_mp3) != frame_size)
    {
        printf("Error: Unable to read frame content\n");
        return e_failure;
    }
    data[frame_size] = '\0';
    return e_success;
}

/* View operation */
Status view_operation(MP3TagInfo *mp3Info)
{
    unsigned int tag_size;
    unsigned int frame_size;

    char tag[5];
    char *data;

    int count;
    int serial_no = 1;

    char *display_names[6]={"Title","Artist","Album","Year","Genre","Comment"};

    /* Check ID3 header */
    if(check_id3_header(mp3Info) == e_failure)
    {
        return e_failure;
    }

    /* check ID3 version */
    if(check_id3_version(mp3Info) == e_failure)
    {
        return e_failure;
    }

    /* check ID3 flags */
    if(check_id3_flags(mp3Info) == e_failure)
    {
        return e_failure;
    }

    /* check ID3 version */
    if(read_id3_tag_size(mp3Info, &tag_size) == e_failure)
    {
        return e_failure;
    }
    printf("\n");
    printf("<----------Started view---------->\n");
    printf("\n");

    printf("------------------------------------------------------------\n");

    printf("Sl.no\t|\tTAG\t|\tContent\n");
    printf("------------------------------------------------------------\n");

    /* *Read six frames */
    for(count = 0; count < FRAME_COUNT; count++)
    {
        /* Read frame tag */
        if(read_frame_tag(mp3Info, tag) == e_failure)
        {
            return e_failure;
        }

        /* *Read frame size */
        if(read_frame_size(mp3Info,&frame_size) == e_failure)
        {
            return e_failure;
        }

        /* *Read frame flags */
        if(read_frame_flags(mp3Info) == e_failure)
        {
            return e_failure;
        }
    
        /* *Allocate memory for content */
        data = malloc(frame_size + 1);
        if(data == NULL)
        {
            printf("Error: Memory allocation failed\n");
            return e_failure;
        }
        
        /* *Read frame content */
        if(read_frame_content(mp3Info,frame_size,data) == e_failure)
        {
            free(data);
            return e_failure;
        }
        
        /* *check frame tag */
        if(check_frame_tag(tag) == e_success)
        {
            /* *First byte is encoding byte for text frames */
            if(strcmp(tag, "TIT2") == 0 ||
                strcmp(tag, "TPE1") == 0 ||
                strcmp(tag, "TALB") == 0 ||
                strcmp(tag, "TYER") == 0 ||
                strcmp(tag, "TCON") == 0||
                strcmp(tag, "COMM") == 0)
            {
                printf("%d\t|\t%s\t|\t%s\n",serial_no,display_names[count],data + 1);
            }
            else if(strcmp(tag, "COMM") == 0)
            {
                printf("%d\t|\t%s\t|\t%s\n",serial_no,display_names[count],data+5);
            }
            serial_no++;
        }
        free(data);
    }
    printf("------------------------------------------------------------\n");

    printf("\n");
    printf("<----------End of view---------->\n");
    printf("\n");

    return e_success;
}
