/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * File plug-in for Sony Playstation .tim files
 * Andrew Kieschnick <andrewk@mail.utexas.edu>
 *
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

/* TIM plugin, version 1.0.0, September 6, 1998
 * 
 * Andrew Kieschnick <andrewk@mail.utexas.edu> 
 *
 * Bugfix: the "next" field of the chunks only handled 16-bit,
 *         so really big images couldn't be written.
 *
 * TODO
 *   24 bpp support
 */

/*
	some modifications from dbalster@psxdev.de
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>

#define TIM   0x10  /* tim id */
#define TIM4  0x08  /*  4 bpp indexed  */
#define TIM8  0x09  /*  8 bpp indexed */
#define TIM16 0x02  /* 16 bpp */
#define TIM24 0x03  /* 24 bpp */

struct tim_common
{
  guchar id[4];
  guchar type[4];
};

struct tim_image
{
  guint next;
  guchar x[2];
  guchar y[2];
  guchar w[2];
  guchar h[2];
};

struct tim_clut
{
  guint next;
  guchar x[2];
  guchar y[2];
  guchar colors[2];
  guchar palettes[2];
  guchar clut[256][2];
};

/* Declare some local functions.
 */
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

//static gint   save_dialog ();

//static void   save_close_callback  (GtkWidget *widget,
//				    gpointer   data);
//static void   save_ok_callback     (GtkWidget *widget,
//				    gpointer   data);
//static void   save_toggle_update   (GtkWidget *widget,
//				    gpointer   data);

GPlugInInfo PLUG_IN_INFO =
{
  NULL,    /* init_proc */
  NULL,    /* quit_proc */
  query,   /* query_proc */
  run,     /* run_proc */
};


MAIN ();

static void
query ()
{
	static GParamDef loadvram_args[] =
	{
		{ PARAM_INT32, "run_mode", "Interactive, non-interactive" },
	};
	gint nloadvram_args = sizeof (loadvram_args) / sizeof (loadvram_args[0]);

	static GParamDef view_args[] =
	{
		{ PARAM_INT32, "run_mode", "Interactive, non-interactive" },
		{ PARAM_IMAGE, "image", "Input image" },
		{ PARAM_DRAWABLE, "drawable", "Drawable to save" },
	};
	gint nview_args = sizeof (view_args) / sizeof (view_args[0]);

	static GParamDef makeclut_args[] =
	{
		{ PARAM_INT32, "run_mode", "Interactive, non-interactive" },
		{ PARAM_IMAGE, "image", "Input image" },
		{ PARAM_DRAWABLE, "drawable", "Drawable to save" },
	};
	gint nmakeclut_args = sizeof (makeclut_args) / sizeof (makeclut_args[0]);
	static GParamDef makemultitexture_args[] =
	{
		{ PARAM_INT32, "run_mode", "Interactive, non-interactive" },
		{ PARAM_IMAGE, "image", "Input image" },
		{ PARAM_DRAWABLE, "drawable", "Drawable to save" },
	};
	gint nmakemultitexture_args = sizeof (makemultitexture_args) / sizeof (makemultitexture_args[0]);
	static GParamDef setclut_args[] =
	{
		{ PARAM_INT32, "run_mode", "Interactive, non-interactive" },
		{ PARAM_IMAGE, "image", "Input image" },
		{ PARAM_DRAWABLE, "drawable", "Drawable to save" },
	};
	gint nsetclut_args = sizeof (setclut_args) / sizeof (setclut_args[0]);


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
    { PARAM_INT32, "run_mode", "Interactive, non-interactive" },
    { PARAM_IMAGE, "image", "Input image" },
    { PARAM_DRAWABLE, "drawable", "Drawable to save" },
    { PARAM_STRING, "filename", "The name of the file to save the image in" },
    { PARAM_STRING, "raw_filename", "The name of the file to save the image in" },
  } ;
  static int nsave_args = sizeof (save_args) / sizeof (save_args[0]);

	gimp_install_procedure ("psx_view_vram",
		"view psx vram",
		"Just writes a TIM image to /proc/pccl/0/vram",
		"Daniel Balster <dbalster@psxdev.de>",
		"Daniel Balster <dbalster@psxdev.de>",
		"1999",
		"<Image>/PSX/View VRAM",
		"*",
		PROC_PLUG_IN,
		nview_args, 0, view_args, NULL);

	gimp_install_procedure ("psx_import_vram",
		"import psx vram",
		"Just reads a TIM image from /proc/pccl/0/vram",
		"Daniel Balster <dbalster@psxdev.de>",
		"Daniel Balster <dbalster@psxdev.de>",
		"1999",
		"<Toolbox>/PSX/Screenshot",
		"",
		PROC_EXTENSION,
		nloadvram_args, 0, loadvram_args, NULL);

	gimp_install_procedure ("psx_make_clut",
		"create 4-Bit CLUTs from RGB image",
		"Just apply this to a RGB image sized (16*x)*y pixels. It will write all 16x1 images to your user palettes directory.",
		"Daniel Balster <dbalster@psxdev.de>",
		"Daniel Balster <dbalster@psxdev.de>",
		"1999",
		"<Image>/PSX/Make CLUT",
		"RGB",
		PROC_PLUG_IN,
		nmakeclut_args, 0, makeclut_args, NULL);

	gimp_install_procedure ("psx_make_multitexture",
		"create complex multipalette texture",
		"It will create a sequence of NetPBM command calls..",
		"Daniel Balster <dbalster@psxdev.de>",
		"Daniel Balster <dbalster@psxdev.de>",
		"1999",
		"<Image>/PSX/Make MultiTexture",
		"*",
		PROC_PLUG_IN,
		nmakemultitexture_args, 0, makemultitexture_args, NULL);

	gimp_install_procedure ("psx_set_clut",
		"create 4-Bit CLUTs from RGB image",
		"Overwrites the current palette with the content of the clipboard selection",
		"Daniel Balster <dbalster@psxdev.de>",
		"Daniel Balster <dbalster@psxdev.de>",
		"1999",
		"<Image>/PSX/Set CLUT",
		"INDEXED",
		PROC_PLUG_IN,
		nsetclut_args, 0, setclut_args, NULL);

  gimp_install_procedure ("file_tim_load",
                          "Loads Sony Playstation .TIM files",
                          "FIXME: write help for file_tim_load",
                          "Andrew Kieschnick",
                          "Andrew Kieschnick",
                          "1998",
                          "<Load>/TIM",
			  NULL,
                          PROC_PLUG_IN,
                          nload_args, nload_return_vals,
                          load_args, load_return_vals);

  gimp_install_procedure ("file_tim_save",
                          "Saves Sony Playstation .TIM files",
                          "FIXME: write help for file_tim_save",
			  "Andrew Kieschnick",
                          "Andrew Kieschnick",
                          "1998",
                          "<Save>/TIM",
			  "RGB, GRAY, INDEXED", 
                          PROC_PLUG_IN,
                          nsave_args, 0,
                          save_args, NULL);

  gimp_register_magic_load_handler ("file_tim_load", "tim", "",
				    "0&,byte,10,2&,byte,1,3&,byte,>0,3,byte,<9"); /* FIXME, what the hell is the .tim magic? */
  gimp_register_save_handler ("file_tim_save", "tim", "");

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

  if (strcmp (name, "file_tim_load") == 0)
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
  else if (strcmp (name, "file_tim_save") == 0)
    {
      switch (run_mode)
	{
	case RUN_INTERACTIVE:
	  break;

	case RUN_NONINTERACTIVE:
	  /*  Make sure all the arguments are there!  */
	  if (nparams != 5)
	    status = STATUS_CALLING_ERROR;
	  break;

	case RUN_WITH_LAST_VALS:
	  break;

	default:
	  break;
	}

      *nreturn_vals = 1;
      if (save_image (param[3].data.d_string, param[1].data.d_int32, param[2].data.d_int32))
	  values[0].data.d_status = STATUS_SUCCESS;
      else
	values[0].data.d_status = STATUS_EXECUTION_ERROR;
    }

//
// psx_view_vram
//

  else if (strcmp (name, "psx_view_vram") == 0)
  {
	save_image ("/proc/pccl/0/vram", param[1].data.d_int32, param[2].data.d_int32);
  }

  else if (strcmp (name, "psx_import_vram") == 0)
  {
      image_ID = load_image ("/proc/pccl/0/vram");

      if (image_ID != -1)
        {
          *nreturn_vals = 2;
          values[0].data.d_status = STATUS_SUCCESS;
          values[1].type = PARAM_IMAGE;
          values[1].data.d_image = image_ID;
		  
		  gimp_display_new (image_ID);
		  gimp_displays_flush();
        }
      else
        {
          values[0].data.d_status = STATUS_EXECUTION_ERROR;
        }
  }

//
// the next one creates a script, which composes a multipalette texture
// (NetPBM needed)
//

  else if (strcmp (name, "psx_make_multitexture") == 0)
  {
	gint32 image_id = param[1].data.d_int32;

	
	gint nlayers;
	gint32 *layers;
	int i;

	gint width, height;

	layers = gimp_image_get_layers (image_id,&nlayers);
	
	width = gimp_image_width   (image_id);
	height = gimp_image_height (image_id);
printf("\n\n");
	/* cut the layer parts */
	for (i=0;i<nlayers;i++)
	{
		gint x,y;
		gimp_drawable_offsets (layers[i],&x,&y);
		printf ("pnmcut %d %d %d %d source.ppm > tmp.%d.ppm\n"
			,x,y,gimp_layer_width(layers[i]),gimp_layer_height(layers[i]),i);
	}

	/* quantization */
	for (i=0;i<nlayers;i++)
	{
		gint x,y;
		gimp_drawable_offsets (layers[i],&x,&y);
		printf ("ppmquant 16 tmp.%d.ppm > tmp.img.%d.ppm\n",i,i);
		printf ("ppmtotim -n -0 -X %d -Y %d tmp.img.%d.ppm > tmp.%d.tim\n",x,y,i,i);
	}

	/* paste together */
	printf ("timmontage -o source.tim -W %d -H %d -m %d -n %d ",width,height,0,nlayers);
	for (i=0;i<nlayers;i++)
	{
		printf ("tmp.%d.tim ",i);
	}
	printf ("\n");
	printf ("rm tmp.*\n");


  }

//
// psx_make_clut
//

  else if (strcmp (name, "psx_make_clut") == 0)
  {

  	gint32 drawable_id = param[2].data.d_int32;	
	GDrawable *drawable = gimp_drawable_get(drawable_id);
	int width, height, tileheight, i,j,x,y;
	guchar *data;
	GPixelRgn pixel_rgn;
	double full,count=0;

	width = drawable->width;
	height = drawable->height;

     gimp_pixel_rgn_init (&pixel_rgn, drawable, 0, 0, width, height, TRUE, FALSE);

	gimp_progress_init ("Exporting palettes..");

	full = (int) ((width/16) * (height));

	tileheight = gimp_tile_height ();
	data = (guchar *) g_malloc(width * tileheight * drawable->bpp);
	for (i = 0; i < height; i += tileheight)
	{
		tileheight = MIN(tileheight, height - i);
		gimp_pixel_rgn_get_rect(&pixel_rgn, data, 0, i, width, tileheight);
		
		for (x=0;x<width;x+=16)
		{
			for (y=0;y<tileheight;y++)
			{
				guchar *p = data + (y*width*drawable->bpp)+(x*drawable->bpp);
				FILE *file;
				char *filename;
				int chk;

				// don't produce "empty, black" palettes.
				for (chk=0,j=0;j<16;j++)
				{
					chk += p[j];
				}
				// "32" is just a threshold, based on experience.
				if (chk<32) continue;

				filename = g_strdup_printf("%s/palettes/CLUT:%05d",
				gimp_directory(),(int)count);

				file = fopen(filename,"w+");
				g_free (filename);

	  gimp_progress_update ((double) count/full);count+=1.0;
				fprintf(file,"GIMP Palette\n");
				for (j=0;j<16;j++)
				{
					fprintf(file,"%d %d %d\n",p[0],p[1],p[2]); p+=3;
				}
				
				fclose (file);
			}
		}
		
	}
	g_free (data);
	gimp_drawable_detach(drawable);
  }

//
// psx_set_clut
//

  else if (strcmp (name, "psx_set_clut") == 0)
  {
	gint32 imageid, image_id = param[1].data.d_int32;
	guchar *palette;
	gint ncolors;
	gint i;
	gint32 *images;
	gint nimages;
	gint32 selection=-1;

	images=gimp_query_images(&nimages);
	if (nimages==0) return;
	for (i=0;i<nimages;i++)
	{
		char *filename = gimp_image_get_filename(images[i]);
		if (strstr(filename,"palette.tim"))
		{
			imageid = images[i];
			selection = gimp_image_get_selection(images[i]);
		}
		
		g_free (filename);
	}

	if (selection==-1)
	{
		gimp_message("palette.tim not loaded!\n");
	
		return;
	}

{
	GDrawable *drawable = gimp_drawable_get (selection);
	int x1,y1,x2,y2;
	GPixelRgn pixel_rgn;
	
	if (drawable)
	{
		gimp_drawable_mask_bounds (selection,&x1,&y1,&x2,&y2);
		gimp_pixel_rgn_init (&pixel_rgn, drawable, 0, 0, drawable->width, drawable->height, TRUE, FALSE);
		palette = gimp_image_get_cmap (image_id,&ncolors);
		gimp_pixel_rgn_get_rect (&pixel_rgn,palette,x1,y1,ncolors,1);
		gimp_image_set_cmap (image_id,palette,ncolors);
		g_free (palette);

	}
	gimp_drawable_detach(drawable);
}
	g_free (images);
  }

}

static gint32
load_image (char *filename)
{
  FILE *fp;
  char * name_buf;
  struct tim_common common;
  struct tim_image image; 
  struct tim_clut clut;

  gint32 image_ID = -1;
  gint32 layer_ID;
  GPixelRgn pixel_rgn;
  GDrawable *drawable;
  guchar *data;
  GDrawableType dtype;
  GImageType itype;
  guchar *cmap;
  
  int width, height, bpp=0;
  int i, j, k;
  int pelbytes, tileheight, wbytes, bsize, npels, pels, ncols, npals;
  int badread;
  gushort tmpval;

  fp = fopen (filename, "rb");
  if (!fp) {
      printf ("TIM: can't open \"%s\"\n", filename);
      return -1;
    }

  name_buf = g_malloc (strlen (filename) + 11);
  sprintf (name_buf, "Loading %s:", filename);
  gimp_progress_init (name_buf);
  g_free (name_buf);

  if (fseek (fp, 0, SEEK_SET) ||
      fread (&common, sizeof (common), 1, fp) != 1) {
    printf("TIM: Cannot read header from \"%s\"\n", filename);
    return -1;
  }

  if (common.id[0] != TIM)
    {
      printf("TIM: Not a .tim file, aborting\n");
      return -1;
    }

  switch (common.type[0])
    {
    case TIM4:
    case TIM8:
      break;
    case TIM16:
      fread( &image, sizeof(image), 1, fp );
      width=image.w[1]*256+image.w[0];
      height=image.h[1]*256+image.h[0];

      itype = RGB;
      dtype = RGB_IMAGE;

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

      pelbytes = drawable->bpp; /* bytes per pixel in the gimp */
      bpp = 2; /* bytes per pixel in the .tim file */

      /* Allocate the data. */
      tileheight = gimp_tile_height ();
      data = (guchar *) g_malloc (width * tileheight * pelbytes);

      wbytes = width * pelbytes;
      badread = 0;
      for (i = 0; i < height; i += tileheight)
	{
	  tileheight = MIN (tileheight, height - i);

	  npels = width * tileheight;
	  bsize = wbytes * tileheight;
	  
	  /* Suck in the data one tileheight at a time. */

	  pels = 0;
	  if (badread)
	    pels = 0;
	  else
	    for(k=0;k<npels;k++)
	      {
		fread(&j, bpp, 1, fp); /* read 1 pixel into j */
		data[k*pelbytes+2]   = (j>>10)<<3;
		data[k*pelbytes+1] = ((j>>5)&0x1f)<<3;
		data[k*pelbytes] = (j&0x1f)<<3;
		pels++; /* increment # pels read */
	      }
	  if (pels != npels)
	    {
	      if (!badread)
		{
		  /* Probably premature end of file. */
		  printf ("TIM: error reading\n");
		  badread = 1;
		}
	      
	      /* Fill the rest of this tile with zeros. */
	      memset (data + (pels * bpp), 0, ((npels - pels) * bpp));
	    }
	  
	  gimp_progress_update ((double) (i + tileheight) / (double) height);

	  gimp_pixel_rgn_set_rect (&pixel_rgn, data, 0, i, width, tileheight);
	}
      
      if (fgetc (fp) != EOF)
	printf ("TIM: too much input data, ignoring extra\n");

      g_free (data);

      gimp_drawable_flush (drawable);
      gimp_drawable_detach (drawable);
      
      fclose(fp);

      return image_ID;

      break;
    case TIM24:
      printf("TIM: 24 bpp unsupported, aborting\n");
      return -1;
    default:
      printf("TIM: Unknown bitdepth, aborting\n");
      return -1;
    }

  if (common.type[0]==TIM8)
    fread( &clut, sizeof(clut), 1, fp );
  else
    fread( &clut, sizeof(clut)-480, 1, fp );

  ncols=clut.colors[1]*256+clut.colors[0];
  npals=clut.palettes[1]*256+clut.palettes[0];

  if (npals != 1)
    {
      printf("TIM: more than 1 palette, discarding extras\n");
      for(j=0;j<(npals-1)*ncols*2;j++)
	fread(&tmpval, 1, 1, fp);
    }

  fread( &image, sizeof(image), 1, fp );

  height=image.h[1]*256+image.h[0];

  width=image.w[1]*256+image.w[0];

  if (common.type[0]==TIM4)
    width=width*4; /* the size in the tim header is in 16-bit chunks */
  else
    width=width*2;

  itype = INDEXED;
  dtype = INDEXED_IMAGE;
  
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
  
  pelbytes = drawable->bpp; /* bytes per pixel in the gimp */

  /* convert the 16bpp CLUT into a Gimp 24bpp cmap */

  cmap = g_malloc (ncols * 3);

  for(j=0;j<ncols;j++)
    {
      tmpval=clut.clut[j][1]*256+clut.clut[j][0];
      cmap[j*3+2]   = (tmpval>>10)<<3;
      cmap[j*3+1] = ((tmpval>>5)&0x1f)<<3;
      cmap[j*3] = (tmpval&0x1f)<<3;
    }

  gimp_image_set_cmap (image_ID, cmap, ncols);
  g_free (cmap);
  
  /* Allocate the data. */
  tileheight = gimp_tile_height ();
  data = (guchar *) g_malloc (width * tileheight * pelbytes);
  
  wbytes = width * pelbytes;
  badread = 0;
  for (i = 0; i < height; i += tileheight)
    {
      tileheight = MIN (tileheight, height - i);
      
      npels = width * tileheight;
      bsize = wbytes * tileheight;
      
      /* Suck in the data one tileheight at a time. */
      
      pels = 0;
      if (badread)
	pels = 0;
      else
	switch(common.type[0])
	  {	
	  case TIM4:
	    for(j=0;j<npels/2;j++)
	      {
		fread(&k,1,1,fp); /* read in 1 byte, which is 2 pixels */
		data[j*2] = k&0x0F; /* low 4 bits */
		data[j*2+1] = k>>4;   /* high 4 bits */
		pels+=2;
	      }
	    break;
	  case TIM8:
	    pels = fread(data, 1, npels, fp);
	    break;
	  default:
	    printf("TIM: This should never happen\n");
	    exit(-1);
	    break;
	  }
      if (pels != npels)
	{
	  if (!badread)
	    {
	      /* Probably premature end of file. */
	      printf ("TIM: error reading\n");
	      badread = 1;
	    }
	  
	  /* Fill the rest of this tile with zeros. */
	  memset (data + (pels * bpp), 0, ((npels - pels) * bpp));
	}
      
      gimp_progress_update ((double) (i + tileheight) / (double) height);
      
      gimp_pixel_rgn_set_rect (&pixel_rgn, data, 0, i, width, tileheight);
    }
  
  if (fgetc (fp) != EOF)
    printf ("TIM: too much input data, ignoring extra\n"); /* this seems to happen a lot */
  
  g_free (data);
  
  gimp_drawable_flush (drawable);
  gimp_drawable_detach (drawable);
  
  fclose (fp);

  return image_ID;
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

  struct tim_common common;
  struct tim_clut clut;
  struct tim_image image;

  guchar *data;
  guchar *cmap;
  int colors;
  int i,j,r,g,b;
  
  drawable = gimp_drawable_get(drawable_ID);
  dtype = gimp_drawable_type(drawable_ID);
  width = drawable->width;
  height = drawable->height;

  name_buf = (guchar *) g_malloc(strlen(filename) + 11);
  sprintf(name_buf, "Saving %s:", filename);
  gimp_progress_init(name_buf);
  g_free(name_buf);

  memset (&common, 0, sizeof (common));
  memset (&clut, 0, sizeof (clut));
  memset (&image, 0, sizeof (image));

  switch (dtype)
    {
    case INDEXED_IMAGE:
      common.type[0] = TIM8;
      clut.next = sizeof(clut);
      image.w[0]=(width/2)%256;
      image.w[1]=(width/2)>>8;
      cmap = gimp_image_get_cmap (image_ID, &colors);
      image.next = (width*height+12);
      clut.colors[0] = colors%256;
      clut.colors[1] = colors>>8;
      clut.palettes[0] = 1;
      for(i=0;i<colors;i++)
	{
		tmpval = ((cmap[i*3+2]>>3)<<10) + ((cmap[i*3+1]>>3)<<5) + (cmap[i*3]>>3);

		if (tmpval == 0) tmpval = 0x8000;

		if ((cmap[i*3+2] == 0xFF) && (cmap[i*3+1] == 0x00) && (cmap[i*3+0] == 0xFF))
		{
			tmpval = 0;
		}
	  clut.clut[i][0] = tmpval%256;
	  clut.clut[i][1] = tmpval>>8;
	}
      if (colors<=16)
	{ /* use TIM4 if we can */
	  common.type[0] = TIM4;
	  image.w[0]=(width/4)%256; 
	  image.w[1]=(width/4)>>8; 
	  image.next = (width/2*height+12);
	  clut.next = (sizeof(clut)-480);
	}
      pelbytes=1;
      break;
    case GRAY_IMAGE:
      common.type[0] = TIM8;
      clut.next = sizeof(clut);
      image.w[0]=(width/2)%256;
      image.w[1]=(width/2)>>8;
      image.next = (width*height+12);
      clut.colors[0] = 0;
      clut.colors[1] = 1;
      clut.palettes[0] = 1;
      clut.palettes[1] = 0;
      for(i=0;i<32;i++)  
	{
	  clut.clut[i*8][0]   = (i*0x421)%256;
	  clut.clut[i*8][1]   = (i*0x421)>>8;
	  clut.clut[i*8+1][0] = (i*0x421)%256;
	  clut.clut[i*8+1][1] = (i*0x421)>>8;
	  clut.clut[i*8+2][0] = (i*0x421)%256;
	  clut.clut[i*8+2][1] = (i*0x421)>>8;
	  clut.clut[i*8+3][0] = (i*0x421)%256;
	  clut.clut[i*8+3][1] = (i*0x421)>>8;
	  clut.clut[i*8+4][0] = (i*0x421)%256;
	  clut.clut[i*8+4][1] = (i*0x421)>>8;
	  clut.clut[i*8+5][0] = (i*0x421)%256;
	  clut.clut[i*8+5][1] = (i*0x421)>>8;
	  clut.clut[i*8+6][0] = (i*0x421)%256;
	  clut.clut[i*8+6][1] = (i*0x421)>>8;
	  clut.clut[i*8+7][0] = (i*0x421)%256;
	  clut.clut[i*8+7][1] = (i*0x421)>>8;
	}
      pelbytes=1;
      break;
    case RGB_IMAGE:
      common.type[0] = TIM16;
      image.next = ((width*height*2)+12);
      image.w[0]=width%256;
      image.w[1]=width>>8;
      pelbytes=2;
      break;
    default:
      printf("TIM: unknown image type, aborting\n");
      return FALSE;
    }
  common.id[0]=TIM;
  image.h[0]=height%256;
  image.h[1]=height>>8;

  image.x[0]=0;
  image.x[1]=0;
  image.y[0]=0;
  image.y[1]=0;

  if((fp = fopen(filename, "wb")) == NULL) {
    printf("TIM: can't create \"%s\"\n", filename);
    return FALSE;
  }

  /* write out the various headers */

  fwrite(&common, sizeof(common), 1, fp);
  if (common.type[0]==TIM4)
    fwrite(&clut, sizeof(clut)-480, 1, fp);
  else if (common.type[0]==TIM8)
    fwrite(&clut, sizeof(clut), 1, fp);

  fwrite(&image, sizeof(image), 1, fp);
  /* Allocate a new set of pixels. */
  tileheight = gimp_tile_height ();
  data = (guchar *) g_malloc(width * tileheight * drawable->bpp);

  gimp_pixel_rgn_init(&pixel_rgn, drawable, 0, 0, width, height, FALSE, FALSE);

  /* Write out the pixel data. */
  status = TRUE;
  for (i = 0; i < height; i += tileheight)
    {
      /* Get a horizontal slice of the image. */
      tileheight = MIN(tileheight, height - i);
      gimp_pixel_rgn_get_rect(&pixel_rgn, data, 0, i, width, tileheight);
      
      npels = width * tileheight;
      bsize = npels * pelbytes;
      
      if(pelbytes==2)
	for(j=0;j<npels;j++)
	  {
	    r=data[j*3+2];
	    g=data[j*3+1];
	    b=data[j*3];
	    data[j*2+1]=(((r>>3)<<2)&0x7c)+((g>>6)&0x03);
	    data[j*2]=(((g>>3)<<5)&0xe0)+((b>>3)&0x1f);
	  }
      if(common.type[0]==TIM4)
	{
	  for(j=0;j<npels/2;j++)
	    {
	      r=data[j*2]; /* not really r */
	      b=data[j*2+1]; /* not really b */
	      data[j]=(b<<4)+r;
	    }
	  npels/=2;
	}
      
      fwrite(data, pelbytes, npels, fp);

      gimp_progress_update ((double) (i + tileheight) / (double) height);
    }

  gimp_drawable_detach(drawable);
  g_free (data);

  fclose(fp);
  return status;
}



