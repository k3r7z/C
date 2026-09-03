#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <threads.h>
#include "tui.h"

typedef struct{
    char user[50];
    char password[50];
    char additional[255];
} Credential;

const char *FILENAME = ".vault";



int size_of_file(FILE *file){
    
    
}

void read_credentials(){
    FILE *vault_ptr = fopen(FILENAME, "rb");

    if(vault_ptr == NULL){
	tui_print_error("There are no credentials saved yet or the ");
	return;
    }


    fseek(vault_ptr, 0, SEEK_END);
    long file_size = ftell(vault_ptr);
    int cred_size = sizeof(Credential);
    // Check the file isn't corrupted
    if(file_size % cred_size != 0){
	tui_print_error("There was a problem reading the file (probably corrupted)");
	fclose(vault_ptr);
	return;
    }
    
    int total_creds = file_size / cred_size;
    fseek(vault_ptr, 0, SEEK_SET);
    

	    
    Credential *creds_ptr = malloc(total_creds * cred_size);
    if(creds_ptr == NULL){
	tui_print_error("It was not possible to reserve ram");
	fclose(vault_ptr);
	return;
    }

    size_t creds_read = fread(creds_ptr, cred_size, total_creds, vault_ptr);
    if(creds_read != total_creds){
	tui_print_error("Error reading file");
	free(creds_ptr);
	fclose(vault_ptr);
	return;
    }
    fclose(vault_ptr);

    for(long i = 0; i < total_creds; ++i)
	tui_print_info("User: %s Password: %s Additionals: %s", creds_ptr[i].user, creds_ptr[i].password, creds_ptr[i].additional);
    
    free(creds_ptr);
}

void add_credentials(){
    Credential credential;

    tui_print_info("Username: ");
    if( fgets(credential.user, sizeof(credential.user), stdin) == NULL){
	tui_print_error("Error reading username");
	return;
    }
    credential.user[ strcspn(credential.user, "\n") ] = '\0';

    tui_print_info("Password: ");
    if(fgets(credential.password, sizeof(credential.password), stdin) == NULL){
	tui_print_error("Error reading password");
	return;
    }
    credential.password[strcspn(credential.password, "\n")] = '\0';

    tui_print_info("Any other details: ");
    if(fgets(credential.additional, sizeof(credential.additional), stdin) == NULL){
	tui_print_error("Error reading additional details");
	return;
    }
    credential.additional[strcspn(credential.additional, "\n")] = '\0';

    FILE *vault = fopen(FILENAME, "ab");

    if(vault == NULL){
	tui_print_error("There was a problem with the vault file");
	return;
    }

    if( fwrite( (void*) &credential, sizeof(credential), 1, vault) == 0)
	tui_print_error("There was an error trying to save the credentials");
    else
	tui_print_success("Credentials saved correctly!");
    
    fclose(vault);
}





int main(int argc, int **argv){
    char buffer[255];
    while(true){
	system("clear");
	tui_print_info("1. See passwords");
	tui_print_info("2. Save new password");
	tui_print_info("3. Exit");

	if(fgets(buffer, sizeof(buffer), stdin) != NULL){

	    buffer[strcspn(buffer, "\n")] = '\0';
	    
	    if(strcmp(buffer, "1") == 0)
		read_credentials();
	    
	    else if(strcmp(buffer, "2") == 0)
		add_credentials();
	    
	    else if(strcmp(buffer, "3") == 0){
		tui_print_info("Option 3. Bye bye");
		break;
	    }
	    else{
		tui_print_error("No option matched");
		break;
	    }
		
	}
	else{
	    tui_print_error("Error reading the input");
	}
    }

    return 0;
}


