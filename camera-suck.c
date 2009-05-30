/*
    camera-suck: Suck the photos off a camera
    Copyright (C) 2009 Perry Lorier <camerasuck@isomer.meta.net.nz>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <gphoto2/gphoto2.h>

static void download_and_delete_all(Camera *camera, GPContext *context, char *folder) 
{
	CameraList *list;
	int i;
	const char *name;

	gp_list_new(&list);
	gp_camera_folder_list_files(camera, 
				folder,
				list,
				context);
	int count = gp_list_count(list);
	printf("Found %d files in %s\n",count,folder);
	for(i = 0; i < count; ++i) {
		CameraFile *file;

		gp_list_get_name(list, i, &name);
		printf("downloading %s/%s\n",folder,name);

		int fd = open(name,O_CREAT|O_TRUNC|O_WRONLY,0666);
		gp_file_new_from_fd(&file, fd);
		gp_camera_file_get (camera, folder, name, 
					GP_FILE_TYPE_NORMAL, file, context);
		gp_file_unref(file);

		gp_camera_file_delete (camera, folder, name, context );
	}

	gp_camera_folder_list_folders(camera, folder, list, context);

	count = gp_list_count(list);
	printf("Found %d directories in %s\n",count,folder);

	for(i=0; i<count; ++i) {
		gp_list_get_name(list, i, &name);
		char *f = NULL;
		if (folder[strlen(folder)-1]!='/')
			asprintf(&f,"%s/%s",folder,name);
		else
			asprintf(&f,"%s%s",folder,name);
		download_and_delete_all(camera, context, f);
		free(f);
	}
	gp_list_free(list);
}

int main()
{
	Camera *camera;
	GPContext *context;
	CameraAbilities a;
	GPPortInfo info;

	gp_camera_new(&camera);

	gp_camera_get_abilities(camera, &a);
	gp_camera_get_port_info(camera, &info);

	download_and_delete_all(camera, context, "/");

	return 0;

}
