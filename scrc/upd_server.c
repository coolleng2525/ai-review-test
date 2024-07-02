#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[BUFFER_SIZE];

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    memset(&clientAddr, 0, sizeof(clientAddr));

    // Configure server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind socket to the specified address and port
    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("UDP server is listening on port %d...\n", PORT);

    while (1) {
        socklen_t len = sizeof(clientAddr);

        // Receive data from client
        ssize_t numBytes = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &len);
        if (numBytes < 0) {
            perror("Error in receiving data");
            exit(EXIT_FAILURE);
        }

        buffer[numBytes] = '\0';
        printf("Received message from client: %s\n", buffer);

        // Process received data (add your logic here)

        // Send response back to client
        if (sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *)&clientAddr, len) < 0) {
            perror("Error in sending response");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}


TAC_U8 *ft_prepare_init_md_assoc_resp_ftie(stamgr_VAP *vap, struct sta_info *sta,
                                   TAC_U8 *ft_ie_len)
{
    /* FTE[MIC(0x0), ANonce(0x0), SNonce(0x0), R1KH-ID, R0KH-ID] */
    size_t buf_len = 0;
    TAC_U8 *buf = NULL, *pos = NULL, *ielen = NULL;
    stamgr_WLAN_svc *wlan_svc = vap->wlan_svc;

    /* FTE length = FTE length + R1KH-ID subelement length + R0KH-ID subelement length */
    buf_len = 2 + sizeof(struct rsn_ftie) + 2+ TAC_FT_R1KH_ID_LEN + 2 +
              wlan_svc->ft_r0_key_holder_len;
    buf = malloc(buf_len);
    if (buf == NULL) {
        ft_ie_len = 0;
        return NULL;
    }
    *ft_ie_len = buf_len;

    memset(buf, 0, buf_len);
    pos = buf;
    *pos++ = WLAN_EID_FAST_BSS_TRANSITION;
    ielen = pos++;
    pos += sizeof(struct rsn_ftie);
    *pos++ = FTIE_SUBELEM_R1KH_ID;
    *pos++ = TAC_FT_R1KH_ID_LEN;
    memcpy(pos, wlan_svc->ft_r1_key_holder, TAC_FT_R1KH_ID_LEN);
    pos += TAC_FT_R1KH_ID_LEN;

    *pos++ = FTIE_SUBELEM_R0KH_ID;
    *pos++ = wlan_svc->ft_r0_key_holder_len;
    memcpy(pos, wlan_svc->ft_r0_key_holder, wlan_svc->ft_r0_key_holder_len);
    pos += wlan_svc->ft_r0_key_holder_len;
    *ielen = pos - buf - 2;

    return buf;
}


void ft_prepare_reassoc_resp_ftie(stamgr_VAP *vap, struct sta_info *sta,
                                  TAC_U8 *ft_ie, TAC_U8 ft_ie_len)
{
    /* FTE[MIC, ANonce, SNonce, R1KH-ID, R0KH-ID, GTK[N]], IGTK[M]] */
    size_t gtk_subelem_len = 0, buf_len = 0;
    TAC_U8 *gtk_subelem = NULL, *buf = NULL, *pos = NULL;
    struct rsn_ftie * _ftie;

    ft_ie_len = 0;
    if (ft_ie == NULL) {
        sta->ft_ie = NULL;
        sta->ft_ie_len = 0;
        return;
    }

    if (sta->ft_ie != NULL) {
        free(sta->ft_ie);
    }

    /* generate GTK sub-element and get GTK sub-element length */
    gtk_subelem = ft_gtk_subelem(vap, sta, &gtk_subelem_len);

    /* FTE length = FTE length of re-association req. + GTK sub-element length */
    buf_len = ft_ie_len + gtk_subelem_len;
    buf = malloc(buf_len);
    if (buf == NULL) {
        return;
    }

    pos = buf;
    /* reuse containce of FTE in re-assocation request */
    memcpy(pos, ft_ie, ft_ie_len);
    pos += ft_ie_len;
        
    /* append GTK */
    memcpy(pos, gtk_subelem, gtk_subelem_len);

    /* free GTK sub-element buff */
    free(gtk_subelem);

    /* reset FTE[MIC] */
    _ftie = (struct rsn_ftie *)buf;
    memset(_ftie->mic, 0, 16);
    sta->ft_ie_len = buf_len;
    /* The "buf" is not free!But I'm not sure if should "sta->ft_ie = _ftie" or "free buf".
         * It seems that there is no any place call this function. So, maybe, this function is invalid.*/

    free(buf);
    return;
}