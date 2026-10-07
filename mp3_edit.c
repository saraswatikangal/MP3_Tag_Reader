#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mp3_tag_reader.h"


/* Convert big endian bytes to integer */
unsigned int convert_to_integer(unsigned char *size)
{
    unsigned int value = 0;
    value = ((unsigned int)size[0] << 24) |
            ((unsigned int)size[1] << 16) |
            ((unsigned int)size[2] << 8) |
            ((unsigned int)size[3]);

    return value;
}

/* Convert integer to big endian */
void convert_to_big_endian(unsigned int value,
                           unsigned char *size)
{
    size[0] = (value >> 24) & 0xFF;
    size[1] = (value >> 16) & 0xFF;
    size[2] = (value >> 8) & 0xFF;
    size[3] = value & 0xFF;
}

/* Check selected tag */
int is_selected_tag(char *option, char *tag)
{
    if(strcmp(option, "-t") == 0 &&
       strcmp(tag, "TIT2") == 0)
    {
        return 1;
    }

    if(strcmp(option, "-A") == 0 &&
       strcmp(tag, "TALB") == 0)
    {
        return 1;
    }

    if(strcmp(option, "-a") == 0 &&
       strcmp(tag, "TPE1") == 0)
    {
        return 1;
    }

    if(strcmp(option, "-y") == 0 &&
       strcmp(tag, "TYER") == 0)
    {
        return 1;
    }

    if(strcmp(option, "-c") == 0 &&
       strcmp(tag, "COMM") == 0)
    {
        return 1;
    }

    if(strcmp(option, "-m") == 0 &&
       strcmp(tag, "TCON") == 0)
    {
        return 1;
    }

    return 0;
}


/* Edit operation */
Status edit_operation(EditInfo *editInfo)
{
    FILE *src;
    FILE *dest;

    unsigned char header[10];
    char tag[5];
    unsigned char size_bytes[4];
    unsigned char flags[2];
    unsigned int old_size;
    unsigned int new_size;

    char *data;

    int count;
    int ch;

    int found = 0;

    /* Open source file */
    src = fopen(editInfo->src_filename, "r");
    if(src == NULL)
    {
       printf("Error:Unable to open source file\n");
       return 0;
    }

    /*create temporary file*/
    dest = fopen(editInfo->temp_filename,"w");
    if(dest == NULL)
    {
        printf("Error:Unable to create tempory file\n");
        fclose(src);
        return e_failure;
    }
    /*move file pointer to 10th byte 
      * ID3 header = 10 bytes */
    if(fread(header, 1, 10, src) != 10)
    {
        printf("Error:Unable to read ID3 header\n");
        fclose(src);
        fclose(dest);
        return e_failure;
    }

    /*Store header in destination */
    if(fwrite(header, 1, 10, dest) != 10)
    {
        printf("ERRor : Unable to write ID3 header to temp file\n");
        fclose(src);
        fclose(dest);
        return e_failure;
    }


    /* Read six metadata frames */
    for(count = 0; count < FRAME_COUNT; count++)
    {
        /* Read tag - 4 bytes */
        if(fread(tag, 1, 4, src) != 4)
        {
            break;
        }
        tag[4] = '\0';

        /* Read size -4 bytes */
        if(fread(size_bytes, 1, 4, src) != 4)
        {
            fclose(src);
            fclose(dest);
            return e_failure;
        }

        /* convert size to integer*/
        old_size = convert_to_integer(size_bytes);

        /* Read flags - 2 bytes */
        if(fread(flags, 1, 2, src) != 2)
        {
            fclose(src);
            fclose(dest);
            return e_failure;
        }

        /* Check Whether tag matches 
        * selected edit option*/
        if(is_selected_tag(editInfo->edit_option, tag) == 1)
        {
            found = 1;
            
            /*Store tag */
            if(fwrite(tag, 1, 4, dest) != 4)
            {
                fclose(src);
                fclose(dest);
                return e_failure;
            }

            /* * Text frame:
             * 1 byte encoding
             * + new data */
            if(strcmp(tag, "COMM") != 0)
            {
               new_size = strlen(editInfo->new_data) + 1;

                /* Convert new size to
                * big endian */
        
                convert_to_big_endian(new_size,size_bytes);

                /* Store new size */
                fwrite(size_bytes, 1, 4, dest);

                /* Store flags */
                fwrite(flags, 1, 2, dest);

                /* Store encoding byte */
                fputc(0, dest);

                /* Store new data */
                fwrite(editInfo->new_data, 1, strlen(editInfo->new_data), dest);

                /* Skip old matadata */
                fseek(src, old_size, SEEK_CUR);
            }
            else
            {
                /*
                 * COMM:
                 * 1 byte encoding
                 * 3 bytes language
                 * 1 byte description terminator 
                 * new data */
                new_size = strlen(editInfo->new_data) +5;

                convert_to_big_endian(new_size,size_bytes);

                fwrite(size_bytes, 1, 4, dest);

                fwrite(flags, 1, 2, dest);

                /* ENCODING */
                fputc(0  , dest);

                /* Language */
                fwrite("eng", 1, 3, dest);

                /* Description terminator */
                fputc(0, dest);

                /* New data */
                fwrite(editInfo->new_data, 1, strlen(editInfo->new_data), dest);

                /* Skip old metadata */
                fseek(src, old_size, SEEK_CUR);
            }
        }
        else
        {
            /* Tag is not selected 
             * copy original tag.*/
    
            fwrite(tag, 1, 4, dest);

            /* copy original size */
            fwrite(size_bytes, 1, 4, dest);

            /* copy flags */
            fwrite(flags, 1, 2, dest);

            /* copy original metadata */
            data = malloc(old_size);

            if(data == NULL)
            {
                printf("Error: Memory allocation failed\n");
                fclose(src);
                fclose(dest);
                return e_failure;
            }

            if(fread(data, 1, old_size, src) != old_size)
            {
                free(data);
                fclose(src);
                fclose(dest);
                return e_failure;
            }
            fwrite(data, 1, old_size, dest);
            free(data);
        }
    }

    /* copy remaining MP3 data */
    while((ch = fgetc(src)) != EOF)
    {
        fputc(ch, dest);
    }
    fclose(src);
    fclose(dest);

    /* Selected tag not found */
    if(found == 0)
    {
        printf("Error:selected tag not found\n");
        remove(editInfo->temp_filename);
        return e_failure;
    }

    /*Delete original file */
    if(remove(editInfo->src_filename)!=0)
    {
        printf("Error:Unable to delete original file\n");
        return e_failure;
    }

    if(rename(editInfo->temp_filename,
                editInfo->src_filename) != 0)
    {
        printf("Error:Unable to rename tempory file\n");
        return e_failure;
    }

    printf("Eddited successfully\n");
    return e_success;
}