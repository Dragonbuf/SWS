#include "csapp.h"

int main(void) {

    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], conatent[MAXLINE];
    int n1 = 0, n2 = 0;


    if ((buf = getenv("QUERY_STRING")) != NULL) {
        p = strchr(buf, '&');
        *p = '\0';
        strcpy(arg1, buf);
        strcpy(arg2, p + 1);
        n1 = atoi(arg1);
        n2 = atoi(arg2);
    }


    sprintf(conatent, "QUERY_STRING=%s", buf);
    sprintf(conatent, "Welcome to localhost:");
    sprintf(conatent, "%sThe Internet addition portal.\r\n<p>", conatent);
    sprintf(conatent, "%sThe answer is : %d + %d = %d\r\n<p>",
            conatent, n1, n2, n1 + n2);

    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int) strlen(conatent));
    printf("Content-Type: text/html\r\n\r\n");
    printf("%s", conatent);
    fflush(stdout);

    exit(0);

}