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

#include "WDS.h"
#ifndef RIS
#define RIS	1
#define NIC_DRIVER_LIST_FILE		"Nics.txt"

#define BINL_BUSTYPE_PCI			"5"

typedef struct _DRIVER
{
	uint16_t vid, pid;
	char driver[256];
	char service[256];
} DRIVER;

int Handle_NCQ_Request(int con, char* Data, size_t Packetlen);
int find_drv(uint16_t cvid, uint16_t cpid, DRIVER *drv);
#endif