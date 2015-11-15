/* $Id: bs.c,v 1.1.1.1 2001/05/17 11:50:26 dbalster Exp $ */

/*
 * File plug-in for Sony Playstation .bs files
 * Daniel Balster <dbalster@psxdev.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <libgimp/gimpintl.h>

static void   query      (void);
static void   run        (char    *name,
                          int      nparams,
                          GParam  *param,
                          int     *nreturn_vals,
                          GParam **return_vals);
static gint32 load_image (char   *filename);
static gint   save_image (char   *filename,
			  gint32  image_ID,
			  gint32  drawable_ID);

static void config_dialog (void);

GPlugInInfo PLUG_IN_INFO =
{
  0,    /* init_proc */
  0,    /* quit_proc */
  query,   /* query_proc */
  run,     /* run_proc */
};

MAIN ();

static gint32 load_image (char *filename)
{
	FILE *fp;

	gint32 image_ID = -1;
	gint32 layer_ID;
	GPixelRgn pixel_rgn;
	GDrawable *drawable;
	guchar *data;
	GDrawableType dtype;
	GImageType itype;
	struct stat st;
  
	int width, height, bpp=0;
	int i, j, k;
	int pelbytes, tileheight, wbytes, bsize, npels, pels, ncols, npals;
	int badread;
	gushort tmpval;

	if (stat (filename,&st) == -1) return -1;

	fp = fopen (filename, "rb");
	if (!fp) return -1;
	bs = (bs_header_t *) g_malloc (st.st_size);
	fread (bs,st.st_size,1,fp);
	fclose (fp);

	if (bs->magic != BS_MAGIC)
	{
		printf("not a .bs file\n");
		return -1;
	}

	gimp_get_data ("bs_width",&width);
	gimp_get_data ("bs_height",&height);

	printf("%d x %d\n",width,height);

	itype	= RGB;
	dtype	= RGB_IMAGE;

	image_ID = gimp_image_new (width, height, itype);
	gimp_image_set_filename (image_ID, filename);
	layer_ID = gimp_layer_new (image_ID,
		"Background",
		width, height,
		dtype,
		100,
		NORMAL_MODE);
	gimp_image_add_layer (image_ID, layer_ID, 0);
	drawable = gimp_drawable_get (layer_ID);
	gimp_pixel_rgn_init (&pixel_rgn, drawable, 0, 0, width, height, TRUE, FALSE);
	data = (guchar *) g_malloc (width * height * 3);

	bs_init();
	bs_decode_rgb24 (data,bs,width,height,0);
	
	printf ("image decoded!\n");
	
	gimp_pixel_rgn_set_rect (&pixel_rgn, data, 0, 0, width, height);
	g_free (data);
	gimp_drawable_flush (drawable);
	gimp_drawable_detach (drawable);
	return image_ID;
}

static void
query ()
{
	static GParamDef load_args[] =
	{
		{ PARAM_INT32, "run_mode", "Interactive, non-interactive" },
		{ PARAM_STRING, "filename", "The name of the file to load" },
		{ PARAM_STRING, "raw_filename", "The name entered" },
	};
	static GParamDef load_return_vals[] =
	{
		{ PARAM_IMAGE, "image", "Output image" },
	};
	static int nload_args = sizeof (load_args) / sizeof (load_args[0]);
	static int nload_return_vals = sizeof (load_return_vals) / sizeof (load_return_vals[0]);

	static GParamDef save_args[] =
	{
		{ PARAM_INT32,		"run_mode",		"Interactive, non-interactive" },
		{ PARAM_IMAGE,		"image",		"Input image" },
		{ PARAM_DRAWABLE,	"drawable",		"Drawable to save" },
		{ PARAM_STRING,		"filename",		"The name of the file to save the image in" },
		{ PARAM_STRING,		"raw_filename",	"The name of the file to save the image in" },
	};
	static int nsave_args = sizeof (save_args) / sizeof (save_args[0]);

	static GParamDef config_args[] =
	{
		{ PARAM_INT32, "run_mode", "Interactive, non-interactive" },
	};
	gint nconfig_args = sizeof (config_args) / sizeof (config_args[0]);

	gimp_install_procedure ("file_rgba_save",
		"Saves OpenGL textures (RGBA)",
		"...",
		"Daniel Balster <dbalster@psxdev.de>",
		"Daniel Balster <dbalster@psxdev.de>",
		"2000",
		"<Save>/RGBA",
		"RGB", 
		PROC_PLUG_IN,
		nsave_args, 0,
		save_args, NULL);

	gimp_install_procedure ("file_rgba_load",
		"Loads OpenGL textures (RGBA)",
		"...",
		"Daniel Balster <dbalster@psxdev.de>",
		"Daniel Balster <dbalster@psxdev.de>",
		"2000",
		"<Load>/RGBA",
		NULL,
		PROC_PLUG_IN,
		nload_args, nload_return_vals,
		load_args, load_return_vals);

	gimp_register_magic_load_handler ("file_rgba_load", "rgba", "",NULL);
	gimp_register_save_handler ("file_rgba_save", "rgba", "");
}

static void
run (char    *name,
     int      nparams,
     GParam  *param,
     int     *nreturn_vals,
     GParam **return_vals)
{
	static GParam values[2];
	GStatusType status = STATUS_SUCCESS;
	GRunModeType run_mode;
	gint32 image_ID;

	run_mode = param[0].data.d_int32;

	*nreturn_vals = 1;
	*return_vals = values;
	values[0].type = PARAM_STATUS;
	values[0].data.d_status = STATUS_CALLING_ERROR;

	if (strcmp (name, "file_rgba_load") == 0)
	{
		image_ID = load_image (param[1].data.d_string);

		if (image_ID != -1)
		{
			*nreturn_vals = 2;
			values[0].data.d_status = STATUS_SUCCESS;
			values[1].type = PARAM_IMAGE;
			values[1].data.d_image = image_ID;
		}
		else
		{
			values[0].data.d_status = STATUS_EXECUTION_ERROR;
		}
	}

	if (strcmp (name, "file_rgba_save") == 0)
	{
		{
		  gchar **argv;
		  gint    argc;

		  argc    = 1;
		  argv    = g_new (gchar *, 1);
		  argv[0] = g_strdup ("bs");

		  gtk_init (&argc, &argv);
		  gtk_rc_parse (gimp_gtkrc ());
		}

		switch (run_mode)
		{
			case RUN_INTERACTIVE:
			printf("RUN_INTERACTIVE\n");


			break;

			case RUN_NONINTERACTIVE:
			printf("RUN_NONINTERACTIVE\n");
			if (nparams != 5) status = STATUS_CALLING_ERROR;
			break;

			case RUN_WITH_LAST_VALS:
			printf("RUN_WITH_LAST_VALS\n");
			break;

			default:
			printf("UNKNOWN RUN MODE\n");
			break;
		}

		*nreturn_vals = 1;
		if (save_image (param[3].data.d_string, param[1].data.d_int32, param[2].data.d_int32))
			values[0].data.d_status = STATUS_SUCCESS;
		else
			values[0].data.d_status = STATUS_EXECUTION_ERROR;
	}
}

gint
save_image (char   *filename,
	    gint32  image_ID,
	    gint32  drawable_ID)
{
	GPixelRgn pixel_rgn;
	GDrawable *drawable;
	GDrawableType dtype;
	guint tmpval;
	int width, height;
	FILE *fp;
	guchar *name_buf;
	int npels, tileheight, pelbytes, bsize;
	int status;

	guchar *data, *out;
	guchar *cmap;
	int colors;
	int size;
	gint compression;
	
	bs_input_image_t img;
  
	drawable = gimp_drawable_get(drawable_ID);
	
	if (drawable->bpp == 3)
	{
		dtype = gimp_drawable_type(drawable_ID);
		width = drawable->width;
		height = drawable->height;

	printf ("%d x %d\n",width,height);

		data = (guchar *) g_malloc (width * height * 3);
		gimp_pixel_rgn_init (&pixel_rgn, drawable, 0, 0, width, height, FALSE, FALSE);
		gimp_pixel_rgn_get_rect (&pixel_rgn, data, 0, 0, width, height);

		img.width    = width;
		img.height   = height;
		img.bit      = 24;
		img.nextline = width*3;
		img.top = img.lpbits = data;

		gimp_get_data ("bs_compression",&compression);
		printf("compression=%d\n",compression);

		out = g_malloc (width * height * 3);
		size = bs_encode ((bs_header_t*)out,&img,BS_TYPE,compression,NULL);
		if (fp = fopen(filename,"wb"))
		{
			fwrite (out,size,1,fp);
			fclose (fp);
		}
		g_free (data);
		g_free (out);
	}
	gimp_drawable_detach(drawable);

	return TRUE;
}

static GtkWidget *matrix[8*8];

void on_compression_changed (GtkAdjustment *adj, GtkSpinButton *data)
{
	gint value = gtk_spin_button_get_value_as_int (data);
	gimp_set_data ("bs_compression",&value,sizeof(gint));
}

void on_width_changed (GtkAdjustment *adj, GtkSpinButton *data)
{
	gint value = gtk_spin_button_get_value_as_int (data);
	gimp_set_data ("bs_width",&value,sizeof(gint));
}

void on_height_changed (GtkAdjustment *adj, GtkSpinButton *data)
{
	gint value = gtk_spin_button_get_value_as_int (data);
	gimp_set_data ("bs_height",&value,sizeof(gint));
}

static void config_dialog (void)
{
	GtkAdjustment *adj;
	GtkWidget *widget;
	GtkWidget *window;
	GtkWidget *vbox, *hbox;
	GtkWidget *frame;
	GtkWidget *table;
	gchar **argv;
	gint argc;
	gint i,j,n,value;
	guchar *iqtab = (guchar*) bs_iqtab();

	argc = 1;
	argv = g_new (gchar *,1);
	argv[0] = g_strdup ("bs_config");

	gtk_init (&argc, &argv);
	gtk_rc_parse (gimp_gtkrc ());

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "BS Config");
	gtk_window_set_policy (GTK_WINDOW (window), TRUE, TRUE, FALSE);
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

	vbox = gtk_vbox_new (FALSE,4);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	frame = gtk_frame_new ("Width x Height");
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX(vbox),GTK_WIDGET(frame), FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE,4);
	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (frame), hbox);

	gimp_get_data ("bs_width",&value);
	adj = (GtkAdjustment*) gtk_adjustment_new ((gfloat)value, 0.0, 1024.0, 1.0, 16.0, 16.0);
	widget = gtk_spin_button_new (adj,1,0);
	gtk_widget_show (widget);
	gtk_box_pack_start (GTK_BOX(hbox),widget, FALSE, FALSE, 0);

	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (on_width_changed), widget);

	gimp_get_data ("bs_height",&value);
	adj = (GtkAdjustment*) gtk_adjustment_new ((gfloat)value, 0.0, 512.0, 1.0, 16.0, 16.0);
	widget = gtk_spin_button_new (adj,1,0);
	gtk_widget_show (widget);
	gtk_box_pack_start (GTK_BOX(hbox),widget, FALSE, FALSE, 0);

	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (on_height_changed), widget);

	frame = gtk_frame_new ("Compression");
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX(vbox),frame, FALSE, FALSE, 0);

	gimp_get_data ("bs_compression",&value);
	adj = (GtkAdjustment*) gtk_adjustment_new ((gfloat)value, 1.0, 255.0, 1.0, 16.0, 16.0);
	widget = gtk_spin_button_new (adj,1,0);
	gtk_widget_show (widget);
	gtk_container_add (GTK_CONTAINER (frame), widget);

	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (on_compression_changed), widget);

	frame = gtk_frame_new ("Q Matrix");
	gtk_box_pack_start (GTK_BOX(vbox),frame, FALSE, FALSE, 0);

	table = gtk_table_new (8, 8, FALSE);
	gtk_widget_show (table);
	gtk_container_add (GTK_CONTAINER (frame), table);

	n = 0;
	for (j=0;j<8;j++)
	{
		for (i=0;i<8;i++)
		{
			adj = (GtkAdjustment*) gtk_adjustment_new ((gfloat)iqtab[n],
				0.0, 255.0, 1.0, 16.0, 16.0);
		
			widget = gtk_spin_button_new (adj,1,0);
			gtk_widget_show (widget);
			
			matrix[n++] = widget;

			gtk_table_attach (GTK_TABLE (table), widget, i, i+1, j, j+1,
				0,
				0, 0, 0);
		}
	}
	gtk_widget_show (frame);

	gtk_widget_set_sensitive (frame, FALSE);

	gtk_widget_show (window);
	gtk_main ();
}
