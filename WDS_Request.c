/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "WDS.h"

int GetClientinfo(int arch, unsigned char* hwadr, int found)
{
	if (found == 1)
	{
		Config.PXEClientPrompt = WDSBP_OPTVAL_PXE_PROMPT_OPTOUT;
        
                sprintf(logbuffer, "============== WDS Client ==============\n");
		logger(logbuffer);

		sprintf(logbuffer, "MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
			hwadr[0], hwadr[1], hwadr[2], hwadr[3], hwadr[4], hwadr[5]);
		logger(logbuffer);

                sprintf(logbuffer, "CLIENT IP: %s\n", inet_ntoa(from.sin_addr));
		logger(logbuffer);

		switch (Client.Action)
		{
		case WDSBP_OPTVAL_ACTION_ABORT:
			sprintf(logbuffer, "ACTION: Abort\n");
			break;
		case WDSBP_OPTVAL_ACTION_APPROVAL:
			sprintf(logbuffer, "ACTION: Approval\n");
			break;
		case WDSBP_OPTVAL_ACTION_REFERRAL:
			sprintf(logbuffer, "ACTION: Referral\n");
			break;
		default:
			sprintf(logbuffer, "ACTION: Unknown\n");
			break;
		}

		logger(logbuffer);

		switch (Client.WDSMode)
		{
		case WDS_MODE_RIS:
			sprintf(logbuffer, "MODE: RIS\n");
			break;
		case WDS_MODE_WDS:
			sprintf(logbuffer, "MODE: WDS\n");
			break;
		case WDS_MODE_UNK:
			sprintf(logbuffer, "MODE: PXELinux\n");
			break;
		default:
			sprintf(logbuffer, "MODE: Unknown\n");
			break;
		}

		logger(logbuffer);

		switch (arch)
		{
		case SYSARCH_INTEL_X86:
			if (Client.ClientArch != arch)
				sprintf(logbuffer, "ARCH: x86 (DHCP reports %d!)\n", Client.ClientArch);
			else
				sprintf(logbuffer, "ARCH: x86\n");

			logger(logbuffer);

			if (Client.Action != WDSBP_OPTVAL_ACTION_ABORT)
				if (Client.WDSMode == WDS_MODE_WDS)
				{
					sprintf(Client.Bootfile, WDS_BOOTFILE_X86);
					sprintf(Client.BCDPath, WDS_BOOTSTORE_X86);
				}
				else
				{
					sprintf(Client.Bootfile, RIS_BOOTFILE_DEFAULT);
					sprintf(Client.BCDPath, WDS_BOOTSTORE_DEFAULT);
				}
			else
			{
				sprintf(Client.Bootfile, WDS_ABORT_BOOTFILE_X86);
				sprintf(Client.BCDPath, WDS_BOOTSTORE_X86);
			}

			Client.WDSMode = WDS_MODE_WDS;
			break;
		case SYSARCH_INTEL_IA64:
			if (Client.ClientArch != arch)
				sprintf(logbuffer, "ARCH: IA64 (DHCP reports %d!)\n", Client.ClientArch);
			else
				sprintf(logbuffer, "ARCH: IA64\n");

			logger(logbuffer);
			Client.WDSMode = WDS_MODE_WDS;
			break;
		case SYSARCH_INTEL_X64:
			if (Client.ClientArch != arch)
				sprintf(logbuffer, "ARCH: x64 (DHCP reports %d!)\n", Client.ClientArch);
			else
				sprintf(logbuffer, "ARCH: x64\n");

			logger(logbuffer);

			if (Client.Action != WDSBP_OPTVAL_ACTION_ABORT)
			{
				sprintf(Client.Bootfile, WDS_BOOTFILE_X64);
				sprintf(Client.BCDPath, WDS_BOOTSTORE_X64);
			}
			else
			{
				sprintf(Client.Bootfile, WDS_ABORT_BOOTFILE_X64);
				sprintf(Client.BCDPath, WDS_BOOTSTORE_X64);
			}

			Client.WDSMode = WDS_MODE_WDS;
			break;
		case SYSARCH_INTEL_EFI:
			if (Client.ClientArch != arch)
				sprintf(logbuffer, "ARCH: x64 EFI (DHCP reports %d!)\n", Client.ClientArch);
			else
				sprintf(logbuffer, "ARCH: x64 EFI\n");

			logger(logbuffer);

			if (Client.Action != WDSBP_OPTVAL_ACTION_ABORT)
			{
				sprintf(Client.Bootfile, WDS_BOOTFILE_EFI);
				sprintf(Client.BCDPath, WDS_BOOTSTORE_EFI);
			}
			else
			{
				sprintf(Client.Bootfile, WDS_ABORT_BOOTFILE_EFI);
				sprintf(Client.BCDPath, WDS_BOOTSTORE_EFI);
			}

			Client.WDSMode = WDS_MODE_WDS;
			break;
		default:
			break;
		}

		Server.RequestID = Server.RequestID + 1;

		if (Server.RequestID == 99)
			Server.RequestID = 1;
        
        Client.ActionDone = 1;
		return 1;
	}
	else
	{
        if (Config.AllowUnknownClients == 0 && Config.ShowClientRequests == 1)
		{
            sprintf(logbuffer, "======== WDS (Request #%d) ========\n", Server.RequestID);
			logger(logbuffer);

			sprintf(logbuffer, "MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
				hwadr[0], hwadr[1], hwadr[2], hwadr[3], hwadr[4], hwadr[5]);
			logger(logbuffer);

			sprintf(logbuffer, "===================================\n");
			logger(logbuffer);
		}

		return 0;
	}
}

int Handle_DHCP_Request(int con, char* Data, int found, saddr* socket, int mode)
{
    char ZeroIP[4] = { 0x00, 0x00, 0x00, 0x00 };
    char Bootreply[1] = { BOOTP_REPLY };
	char DHCP_ACK[3] = { 0x35, 0x01, setDHCPRespType(found) };
	char Vendoropt[2] = { 0x3C, 0x09 };
	char DHCPEnd[1] = { 0xff };
	char idtopt[2] = { 0x36, 0x04 };
	char Rtropt[2] = { 0x03, IPV4_ADDR_LENGTH };
    char netopt[2] = { 0x01, IPV4_ADDR_LENGTH };

	if (memcmp(&Data[BOOTP_OFFSET_CLIENTIP], ZeroIP, IPV4_ADDR_LENGTH) != 0 &&
		memcmp(&Data[BOOTP_OFFSET_NEXTSERVER], ZeroIP, IPV4_ADDR_LENGTH) == 0)
		return 0;

	RESPsize = 0;

	/* BOOTP Message Type */
	memcpy(&RESPData[RESPsize], Bootreply, sizeof(Bootreply));
	Set_Size(sizeof(Bootreply));

	/* Hardware Type */
	memcpy(&RESPData[RESPsize], &Data[BOOTP_OFFSET_HWTYPE], 1);
	Set_Size(1);

	/* Hardware-Address-Length */
	memcpy(&RESPData[RESPsize], &Data[BOOTP_OFFSET_MACLEN], 1);
	Set_Size(1);

	/* Hops */
	memcpy(&RESPData[RESPsize], &Data[BOOTP_OFFSET_HOPS], 1);
	Set_Size(1);

	/* Transaction-ID */
	memcpy(&RESPData[RESPsize], &Data[BOOTP_OFFSET_TRANSID], 4);
	Set_Size(4);

	/* Elapsed Seconds */
	memcpy(&RESPData[RESPsize], &Data[BOOTP_OFFSET_SECONDS], 2);
	Set_Size(2);

	/* BOOTP Flags */
	memcpy(&RESPData[RESPsize], &Data[BOOTP_OFFSET_BOOTPFLAGS], 2);
	Set_Size(2);

	/* Client-IP */
	memcpy(&RESPData[RESPsize], &Data[BOOTP_OFFSET_YOURIP], IPV4_ADDR_LENGTH);
	Set_Size(IPV4_ADDR_LENGTH);

	/* Your-IP */
	memcpy(&RESPData[RESPsize], &Data[BOOTP_OFFSET_CLIENTIP], IPV4_ADDR_LENGTH);
	Set_Size(IPV4_ADDR_LENGTH);

	/* Next Server-IP */
	memcpy(&RESPData[RESPsize], &Config.ServerIP, sizeof(Config.ServerIP));
	Set_Size(sizeof(Config.ServerIP));

	/* Relay Agent-IP */
	memcpy(&RESPData[RESPsize], &Data[BOOTP_OFFSET_RELAYIP], IPV4_ADDR_LENGTH);
	Set_Size(IPV4_ADDR_LENGTH);

	/* Client Mac-Address */
	memcpy(&RESPData[RESPsize], &Data[BOOTP_OFFSET_MACADDR], Data[BOOTP_OFFSET_MACLEN]);
	Set_Size(Data[2]);

	/* Address Padding */
	memcpy(&RESPData[RESPsize], &Data[BOOTP_OFFSET_MACPADDING], 10);
	Set_Size(10);

	/* Server Hostname */
	memcpy(&RESPData[RESPsize], Server.nbname, sizeof(Server.nbname));
	Set_Size(sizeof(Server.nbname));

	if (Client.isWDSRequest == 0)
		sprintf(Client.Bootfile, DHCP_BOOTFILE);
    
	memcpy(&RESPData[RESPsize], Client.Bootfile, sizeof(Client.Bootfile));
	Set_Size(sizeof(Client.Bootfile));

	/* MAGIC COOKIE */
	memcpy(&RESPData[RESPsize], &Data[BOOTP_OFFSET_COOKIE], 4);
	Set_Size(4);

	/* DHCP Response Type (53) */
	memcpy(&RESPData[RESPsize], DHCP_ACK, sizeof(DHCP_ACK));
	Set_Size(sizeof(DHCP_ACK));

	/* Option Server Ident (54) */
	memcpy(&RESPData[RESPsize], idtopt, sizeof(idtopt));
	Set_Size(sizeof(idtopt));

	memcpy(&RESPData[RESPsize], &Config.ServerIP, sizeof(Config.ServerIP));
	Set_Size(sizeof(Config.ServerIP));

	/* DHCP-Option "Vendor-Class" (60) */
	memcpy(&RESPData[RESPsize], Vendoropt, 2);
	Set_Size(2);

	sprintf(&RESPData[RESPsize], "PXEClient");
	Set_Size(9);

    /* Send option 1 & 3 only when we have an WDS request... otherwise it is 
     * already done by the provided dhcp service! */
    
    if (Client.isWDSRequest == 1)
    {
        /* Option Netmask (1) */
	
        memcpy(&RESPData[RESPsize], netopt, sizeof(netopt));
        Set_Size(sizeof(netopt));

        memcpy(&RESPData[RESPsize], &Config.SubnetMask, sizeof(Config.SubnetMask));
        Set_Size(sizeof(Config.SubnetMask));
    
        /* Option Router (3) */
        memcpy(&RESPData[RESPsize], Rtropt, sizeof(Rtropt));
        Set_Size(sizeof(Rtropt));

        memcpy(&RESPData[RESPsize], &Config.ServerIP, sizeof(Config.ServerIP));
        Set_Size(sizeof(Config.ServerIP));
    }
    
    if (found == 1)
    {
        if (Client.WDSMode == WDS_MODE_WDS)
        {
            /* Boot Configuration Store (252) */
            char bcdopt[2] = { 0xfc, strlen(Client.BCDPath) };

            memcpy(&RESPData[RESPsize], bcdopt, sizeof(bcdopt));
            Set_Size(sizeof(bcdopt));

            memcpy(&RESPData[RESPsize], Client.BCDPath, strlen(Client.BCDPath));
            Set_Size(strlen(Client.BCDPath));
        }

        sprintf(logbuffer, "========================================\n");
        logger(logbuffer);
    }
    else
    {
        /* WDSNBP related informations */
        if (Client.ActionDone == 0)
            Client.Action = WDSBP_OPTVAL_ACTION_APPROVAL;

        if (Config.AllowUnknownClients == 0)
        {
            char aprovalmsg[29] = {
                WDSBP_OPT_NEXT_ACTION, 0x01, Client.Action,
                WDSBP_OPT_REQUEST_ID, 0x04, 0x00, 0x00, 0x00, Server.RequestID,
                WDSBP_OPT_POLL_INTERVAL, 0x02, 0x00, Config.PollIntervall,
                WDSBP_OPT_POLL_RETRY_COUNT, 0x02, 0x00, Config.TFTPRetryCount,
                WDSBP_OPT_ACTION_DONE, 0x01, Client.ActionDone,
                WDSBP_OPT_VERSION_QUERY, 0x01, Config.VersionQuery,
                WDSBP_OPT_PXE_CLIENT_PROMPT, 0x01, Config.PXEClientPrompt, 
                WDSBP_OPT_PXE_PROMPT_DONE, 0x01, Config.PXEClientPrompt };

            char admopt[2] = { 0xfa, sizeof(aprovalmsg) };

            memcpy(&RESPData[RESPsize], admopt, sizeof(admopt));
            Set_Size(sizeof(admopt));

            memcpy(&RESPData[RESPsize], aprovalmsg, sizeof(aprovalmsg));
            Set_Size(sizeof(aprovalmsg));
            
        }
    }

    /* End of DHCP-Options */
	memcpy(&RESPData[RESPsize], DHCPEnd, sizeof(DHCPEnd));
	Set_Size(sizeof(DHCPEnd));
    
    if (Client.inDHCPMode == 0)
       return WDS_Send(con, RESPData, RESPsize, NULL, mode);
    else
       return DHCP_Send(con, RESPData, RESPsize, NULL, mode);
}
