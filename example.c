#include "inc\defs.h"
#include "inc\fs.h"
#include "inc\file.h"

void main()
{
	unsigned char buffer[10];
	asgard_fs * fs = malloc(sizeof(asgard_fs));
	asgard_fs * fs2 = malloc(sizeof(asgard_fs));
	if(fs_init()==FS_UNFORMATTED)
	{
		fs_format(32, 32768);
		file_createfile(0x3f01, "1234567890", 10);
		file_createdirectory(0x0001, "", 0);
		file_createfile(0x3f02, "1234567890", 10);
	}
	file_dirlist();
	file_select(fs, 0x0000);
	file_select(fs, 0x3f01);
	file_select(fs2, 0x0000);
	file_read(fs, 0, 10, buffer);
	printf("%s\n", buffer);
}