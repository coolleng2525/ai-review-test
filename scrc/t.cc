int EmfdIPC::getstat_request(const char *url, char *post_data)
{
    // for CLI privilege check next step.
    //char *buffer[2] = {(char*)"AjaxCmdStat", (char*)session_element.cid};
    char *buffer[2] = {(char*)"AjaxCmdStat", (char*)""};
    const char *ipc_caller = NULL;
    const char *session_id = NULL;
    FILE *fp = NULL;
    char remote_ip[256] = {0};

	char process_name[64] = {0};
    ipc_caller = EMF_ADAPTER_IPC_CALLER_ZD_CLI;

    if (0 == get_current_process_name(process_name, sizeof(process_name))) {
        if (strstr(process_name, "snmpd")) {
            memset(snmp_ip_str, 0, sizeof(snmp_ip_str));

            fp = fopen(SNMP_IP_FILE, "r");
            if (fp != NULL) {
                fscanf(fp, "%s\n", snmp_ip_str);
                fclose(fp);

                if (0 != snmp_ip_str[0]) {
                    session_id = snmp_ip_str;
                }
            }

            ipc_caller = EMF_ADAPTER_IPC_CALLER_SNMP;
        } else if (strstr(process_name, "ruckus_cli2")) {
            session_id = getenv("SSH_CLIENT");

            if (NULL == session_id) {
                if (0 == get_cli_session_remote_ip(remote_ip, sizeof(remote_ip))) {
                    session_id = remote_ip;
                } else {
                    session_id = "";
                }
            }

            ipc_caller = EMF_ADAPTER_IPC_CALLER_ZD_CLI;
        }
    }

    return ipc_request(url, post_data, 2, buffer, ipc_caller, session_id);
}