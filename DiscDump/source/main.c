
#include "ps4.h"
#include "patch.h"

int nthread_run;
char notify_buf[1024];
int  xfer_pct;
long xfer_cnt;
char *cfile;
int tmpcnt;
int isxfer;

void copyFile(char *sourcefile, char* destfile)
{
    int src = open(sourcefile, O_RDONLY, 0);
    if (src != -1)
    {
        int out = open(destfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (out != -1)
        {
            cfile = sourcefile;
            isxfer = 1;
            size_t bytes, bytes_size, bytes_copied = 0;
            char *buffer = malloc(65536);
            if (buffer != NULL)
            {
                lseek(src, 0L, SEEK_END);
                bytes_size = lseek(src, 0, SEEK_CUR);
                lseek(src, 0L, SEEK_SET);
                while (0 < (bytes = read(src, buffer, 65536))) {
                    write(out, buffer, bytes);
                    bytes_copied += bytes;
                    if (bytes_copied > bytes_size) bytes_copied = bytes_size;
                   xfer_pct = bytes_copied * 100 / bytes_size;
                   xfer_cnt += bytes;
                }
                free(buffer);
            }
            close(out);
            isxfer = 0;
            xfer_pct = 0;
            xfer_cnt = 0;
        }
        else {
        }
        close(src);
    }
    else {
    }
}


void copyDir(char *sourcedir, char* destdir)
{
    DIR *dir;
    struct dirent *dp;
    struct stat info;
    char src_path[1024], dst_path[1024];

    dir = opendir(sourcedir);
    if (!dir)
        return;
        mkdir(destdir, 0777);
    while ((dp = readdir(dir)) != NULL)
    {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
        {}
        else
        {
            sprintf(src_path, "%s/%s", sourcedir, dp->d_name);
            sprintf(dst_path, "%s/%s", destdir  , dp->d_name);

            if (!stat(src_path, &info))
            {
                if (S_ISDIR(info.st_mode))
                {
                  copyDir(src_path, dst_path);
                }
                else
                if (S_ISREG(info.st_mode))
                {
                  copyFile(src_path, dst_path);
                }
            }
        }
    }
    closedir(dir);
}


void *nthread_func(void *arg)
{
        time_t t1, t2;
        t1 = 0;
	while (nthread_run)
	{
		if (isxfer)
		{
			t2 = time(NULL);
			if ((t2 - t1) >= 10)
			{
				t1 = t2;
				if (tmpcnt >= 1048576)
				{
				   sprintf(notify_buf, "Copying: %s\n\n%u%% completed\nSpeed: %u MB/s", cfile , xfer_pct, tmpcnt / 1048576);
				}
				else if (tmpcnt >= 1024)
				{
				   sprintf(notify_buf, "Copying: %s\n\n%u%% completed\nSpeed: %u KB/s", cfile , xfer_pct, tmpcnt / 1024);
				}
				else
				{
				   sprintf(notify_buf, "Copying: %s\n\n%u%% completed\nSpeed: %u B/s", cfile , xfer_pct, tmpcnt);
				}
				
				systemMessage(notify_buf);
			}
		}
		else t1 = 0;
		sceKernelSleep(1);
	}
	return NULL;
}


void *sthread_func(void *arg)
{
	while (nthread_run)
	{
           if (isxfer)
           {
              tmpcnt = xfer_cnt;
              xfer_cnt = 0;
           }
          sceKernelSleep(1);
	}
	return NULL;
}


int _main(struct thread *td) {
    initKernel();
    initLibc();
    initPthread();
    DIR *dir;
    dir = opendir("/mnt/disc");
    if (!dir)
    {
       syscall(11,patcher,td);
    }
    else
    {
       closedir(dir);
    }
    initSysUtil();
    xfer_cnt = 0;
    isxfer = 0;
	nthread_run = 1;
	ScePthread nthread;
	scePthreadCreate(&nthread, NULL, nthread_func, NULL, "nthread");
	ScePthread sthread;
	scePthreadCreate(&sthread, NULL, sthread_func, NULL, "sthread");
        FILE *usbdir = fopen("/mnt/usb0/.dirtest", "wb");
    
         if (!usbdir)
            {
                usbdir = fopen("/mnt/usb1/.dirtest", "wb");
                if (!usbdir)
                {
                        systemMessage("No usb drive found.\nYou must insert a usb drive with about 64gb of free space to dump the disk.");
                        nthread_run = 0;
                        return 0;
                }
                else
                {
                        fclose(usbdir);
                        systemMessage("Dumping disc to USB1");
                        unlink("/mnt/usb1/.dirtest");
                        mkdir("/mnt/usb1/Disc_Dump/", 0777);
                        copyDir("/mnt/disc","/mnt/usb1/Disc_Dump");
                        systemMessage("Dump Complete.");

                }
            }
            else
            {
                        fclose(usbdir);
                        systemMessage("Dumping disc to USB0");
                        unlink("/mnt/usb0/.dirtest");
                        mkdir("/mnt/usb0/Disc_Dump/", 0777);
                        copyDir("/mnt/disc","/mnt/usb0/Disc_Dump");
                        systemMessage("Dump Complete.");
            }

    nthread_run = 0;
    return 0;
}


