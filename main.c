#include <stdio.h>
#include <string.h>

#include "mp3_tag_reader.h"

int main(int argc, char *argv[])
{
    OperationType operation;

    MP3TagInfo mp3Info;
    EditInfo editInfo;

    if(argc < 2)
    {
        printf("Error:Invalid arguments\n");
        printf("USAGE:\n");
        printf("To view please pass like : ./a.out -v mp3filename\n");
        printf("To edit please pass like : ./a.out -e -t/-a/-A/-m/-y/-g mp3filename\n");
        printf("To help please pass like : ./a.out -h\n");

        return 0;
    }

    operation = check_operation_type(argv[1]);

    /* View operation */
    if(operation == e_view)
    {
        if(argc != 3)
        {
            printf("Error:MP3 filename missing\n");
            //printf("Error:Insufficient arguments for view\n");
            return 0;
        }

        /* Store MP3 filename */
        mp3Info.filename = argv[2];

        /* Check .mp3 extension */
        if(check_mp3_file(mp3Info.filename) == e_failure)
        {
            printf("Error:Invalid MP3 file\n");
            return 0;
        }

        /* Open MP3 file */
        if(open_mp3_file(&mp3Info) == e_success)
        {
            /* View tags */
            if(view_operation(&mp3Info) == e_failure)
            {
                fclose(mp3Info.fptr_mp3);
                return 0;
            }
            {
                fclose(mp3Info.fptr_mp3);
                return 0;
            }

            fclose(mp3Info.fptr_mp3);
        }
    }


    /* Edit operation */
    else if(operation == e_edit)   
    {
        if(argc != 5)
        {
            printf("Error:Insufficient arguments for edit\n");
            printf("To edit please pass like : ./a.out -e -t/-a/-A/-m/-y/-g mp3filename\n");
            return 0;
        }

        editInfo.edit_option = argv[2];
        editInfo.new_data = argv[3];
        editInfo.src_filename = argv[4];

        /* validate edit option */
        if(strcmp(editInfo.edit_option, "-t") != 0 &&
            strcmp(editInfo.edit_option, "-A") != 0 &&
            strcmp(editInfo.edit_option, "-a") != 0 &&
            strcmp(editInfo.edit_option, "-y") != 0 &&
            strcmp(editInfo.edit_option, "-g") != 0 && 
            strcmp(editInfo.edit_option, "-m") != 0 )
        {
            printf("Error:Invalid edit option\n");
            return 0;
        }

        /* check MP3 extension */
        if(check_mp3_file(editInfo.src_filename) == e_failure)
        {
            return 0;
        }
        /* Edit */
        if(edit_operation(&editInfo) == e_failure)
        {
            return 0;
        }
    }

    /* Help operation */
    else if(operation == e_help)
    {
        printf("1. -v -> to view mp3 file contents\n");
        printf("2. -e -> to edit mp3 file contents\n");

        printf("    2.1. -t -> to edit title\n");
        printf("    2.2. -a -> to edit artist name\n");
        printf("    2.3. -A -> to edit album name\n");
        printf("    2.4. -y -> to edit year\n");
        printf("    2.5. -m -> to edit content\n");
        printf("    2.6. -c -> to edit comment\n");
    }
}