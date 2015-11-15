#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include <sys/ioctl.h>
#include <sys/file.h>
#include <errno.h>
#include <bfd.h>
#include <psxdev.h>
#include <pccl.h>

/***********************************************************/
// types

typedef struct {
	short	m[3][3];	/* 3x3 rotation matrix */
	long    t[3];		/* transfer vector */
} matrix_t;

typedef struct {		/* long word type 3D vector */
	long	vx, vy;
	long	vz, pad;
} vector_t;
	
typedef struct {		/* short word type 3D vector */	
	short	vx, vy;
	short	vz, pad;
} svector_t;

typedef struct {		/* short rect */	
	short	x, y;
	short	w, h;
} rect_t;
	       
typedef struct {		/* color type vector */	
	u_char	r, g, b, cd;
} cvector_t;

/***********************************************************/
// externals

extern GtkWidget *clist;
extern gchar *the_filename;

/***********************************************************/
// statics

static GArray *idlist = NULL;
static gchar *pccl_device_name = "/dev/pccl";
static asymbol *current_symbol = NULL;

/***********************************************************/
// prototypes

void set_remote (unsigned long addr, void *ptr, unsigned long size);
void get_remote (unsigned long addr, void *ptr, unsigned long size);
void retrieve_symbols (char *filename, GtkCList *list);

/***********************************************************/

void retrieve_symbols (char *filename, GtkCList *list)
{
	bfd *abfd;
	caetla_execute_t exec;
	caetla_memory_t mem;  // memory transfer
	caetla_device_info_t info;
	u_char checksum;
	int pccl, rc;
	GtkWidget *appbar = lookup_widget(list,"appbar1");

	bfd_init();

	if (idlist != NULL)
	{
		g_array_free (idlist,TRUE);
	}
	idlist = g_array_new (FALSE,TRUE,sizeof(gpointer));

	gnome_appbar_set_progress (appbar,0.0);
	gnome_appbar_set_status (appbar,"Analysing symbol information..");

	gtk_clist_freeze (list);
	gtk_clist_clear (list);

	errno = 0;
	abfd = bfd_openr (filename,0);
	if (abfd != NULL)
	{
		if (bfd_check_format (abfd,bfd_object))
		{
			long storage_needed;
			asymbol **symbol_table, *sym;
			asection *section;
			bfd_size_type datasize = 0;
			long number_of_symbols;
			long i;
			char *label[10];
			int row;

			storage_needed = bfd_get_symtab_upper_bound (abfd);
			if (storage_needed <= 0)
			{
				printf("storage_needed exceeds limit! %d\n",storage_needed);
				exit(0);
			}

			printf ("PC=0x%08lx GP=0x%08lx\n",abfd->start_address,bfd_ecoff_get_gp_value(abfd));

		//	pccl = open(pccl_device_name,O_RDWR);
		//	if (pccl != -1)
			if (0)
			{
				exec.pc0 = abfd->start_address;
				exec.gp0 = 0; //abfd->start_address;
				exec.t_size = 0;
				exec.t_addr = 0;
				exec.s_addr = 0x801ffff0;
				exec.s_size = 0;
				exec.task = 4;
				exec.event = 0x10;
				exec.stack = 0x801ffff0;
				exec.mode = 1;

				for (section = abfd->sections; section != NULL; section = section->next)
				{
					datasize = bfd_section_size(abfd,section);
					if (section->flags & SEC_HAS_CONTENTS)
					{
						bfd_byte *data = (bfd_byte *) g_malloc (datasize);
						bfd_get_section_contents (abfd,section,data,0,datasize);

						printf("%s: %d bytes -> 0x%08lx\n",section->name,datasize,data);

						mem.address = section->vma;
						mem.size    = datasize;

						if (strcmp(section->name,".text")==0)
						{
							exec.t_size = mem.size;
							exec.t_addr = mem.address;
						}

						rc = ioctl (pccl,CAEIOC_WRITE_MEMORY,&mem);
						if (rc == -1) perror("ioctl(CAEIOC_WRITE_MEMORY)");

						rc = write (pccl,data,datasize);
						if (rc < 0) perror("write()");

						rc = ioctl (pccl,CAEIOC_SWAP_8,&checksum);
						if (rc == -1) perror("ioctl(CAEIOC_SWAP_8)");

						rc = ioctl (pccl,CAEIOC_DEVICE_INFO,&info);
						if (rc == -1) perror("ioctl(CAEIOC_DEVICE_INFO)");

						if (checksum != info.checksum)
						{
							perror ("invalid checksum");
						}

						g_free (data);
					}
				}
				
				rc = ioctl (pccl,CAEIOC_EXECUTE,&exec);
				if (rc == -1) perror("ioctl(CAEIOC_EXECUTE)");

	//			psxdev_server(pccl,0,1);

				close (pccl);
			}

			symbol_table = (asymbol **) malloc (storage_needed);
			number_of_symbols = bfd_canonicalize_symtab (abfd, symbol_table);
			for (i = 0; i < number_of_symbols; i++)
			{
				symbol_info syminf;
				
				sym = symbol_table[i];

	gnome_appbar_set_progress (appbar,(gfloat)i / (gfloat)number_of_symbols);

			
				bfd_symbol_info (sym,&syminf);
				
				if (
					(bfd_asymbol_value(sym)==0) &&
					(syminf.type == '?') &&
					(sym->flags == 8)
				)
				{
					gint typeid;
					char *s = strchr(sym->name,':');

					if (s != NULL)
					{
						if (s[1] == 't')
						{
							typeid = atoi (s+2);
							*s = 0;
							g_array_set_size (idlist,typeid+1);
							g_array_insert_val (idlist,typeid,sym->name);
						}
					}
				}
				
				if ( (sym->flags == 9) && (syminf.type != 't'))
				{
					char *s = strchr(sym->name,':'), *t;
					int typeid;
					
					if (s != NULL)
					{
						typeid = atoi(s+2);
						*s = 0;
					
						t = g_array_index (idlist,gpointer,typeid);
						
						if (t != NULL)
						{
							label[0] = g_strdup_printf("0x%08lx",bfd_asymbol_value(sym));
							label[1] = t;
							label[2] = sym->name;

							sym->flags = typeid;

							row = gtk_clist_append (list,label);
							gtk_clist_set_row_data (list,row,(gpointer)sym);
							
							g_free (label[0]);
						}
					}
				}
			}
		}
	}
	else printf ("bfd_openr(%s); %s",filename,bfd_errmsg(bfd_get_error()));

	gtk_clist_thaw (list);
	gnome_appbar_set_status (appbar,"Ok.");
	gnome_appbar_set_progress (appbar,0);
}

/***********************************************************/

void set_remote (unsigned long addr, void *ptr, unsigned long size)
{
	// old style

/*
	caetla_memory_t mem;
	caetla_byte_t byte;
	caetla_device_info_t info;
	int pccl, rc;
	u_char checksum;

	mem.address = addr;
	mem.size    = size;

	pccl = open(pccl_device_name,O_RDWR);
	if (pccl == -1) return;

	rc = ioctl (pccl,CAEIOC_DU_WRITE_MEMORY,&mem);
	if (rc == -1) perror ("CAEIOC_DU_WRITE_MEMORY");
	
	rc = write (pccl,ptr,size);
	if (rc < 0) perror("write()");

	rc = ioctl (pccl,CAEIOC_DEVICE_INFO,&info);
	if (rc == -1) perror("ioctl(CAEIOC_DEVICE_INFO)");

	rc = ioctl (pccl,CAEIOC_RESUME);
	if (rc == -1) perror("ioctl(CAEIOC_RESUME)");

	close (pccl);
*/
	// new style

	int file;
	
	file = open ("/proc/pccl/0/fullmem",O_RDWR);
	if (file)
	{
		lseek (file,addr,0);
		write (file,ptr,size);
		close (file);
	}
	else perror(0);

}

void get_remote (unsigned long addr, void *ptr, unsigned long size)
{
	int file;
	
	file = open ("/proc/pccl/0/fullmem",O_RDWR);
	if (file)
	{
		lseek (file,addr,0);
		read (file,ptr,size);
		close (file);
	}
	else perror(0);
/*

	caetla_memory_t mem;
	caetla_device_info_t info;
	u_char checksum;
	u_long x;
	int pccl, rc;

	mem.address = addr;
	mem.size    = size;

	pccl = open(pccl_device_name,O_RDWR);
	if (pccl == -1) return;

	rc = ioctl (pccl,CAEIOC_DU_READ_MEMORY,&mem);
	if (rc == -1) perror ("CAEIOC_DU_READ_MEMORY");
	
	rc = read (pccl,ptr,size);
	if (rc < 0) perror("write()");

	rc = ioctl (pccl,CAEIOC_SWAP_8,&checksum);
	if (rc == -1) perror("ioctl(CAEIOC_SWAP_8)");

	rc = ioctl (pccl,CAEIOC_DEVICE_INFO,&info);
	if (rc == -1) perror("ioctl(CAEIOC_DEVICE_INFO)");

	if (checksum != info.checksum)
	{
		printf ("invalid checksum (%02x/%02x)",checksum,info.checksum);
	}

	x = 0;
	rc = ioctl (pccl,CAEIOC_SWAP_32,&x);
	if (rc == -1) perror("ioctl(CAEIOC_SWAP_32)");
	x = 0;
	rc = ioctl (pccl,CAEIOC_SWAP_32,&x);
	if (rc == -1) perror("ioctl(CAEIOC_SWAP_32)");

	rc = ioctl (pccl,CAEIOC_SWAP_8,&checksum);
	if (rc == -1) perror("ioctl(CAEIOC_SWAP_8)");
	rc = ioctl (pccl,CAEIOC_SWAP_8,&checksum);
	if (rc == -1) perror("ioctl(CAEIOC_SWAP_8)");

	rc = ioctl (pccl,CAEIOC_RESUME);
	if (rc == -1) perror("ioctl(CAEIOC_RESUME)");

	close (pccl);
*/
}

/***********************************************************/

void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	retrieve_symbols (the_filename,clist);
//	retrieve_symbols ("/usr/psxdev/src/db0player/a.out",clist);
}


void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_main_quit();
}


void
on_preferences1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *about = create_about ();
	gtk_widget_show (about);
}

/***********************************************************/

void
on_Q_uchar_changed                     (GtkEditable     *editable,
                                        gpointer         user_data)
{
	u_char value = gtk_spin_button_get_value_as_int (GTK_WIDGET(editable));
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(editable),"symbol");
	u_long address = bfd_asymbol_value(sym);
	set_remote (address,&value,sizeof(u_char));
}


void
on_Q_char_changed                      (GtkEditable     *editable,
                                        gpointer         user_data)
{
	char value = gtk_spin_button_get_value_as_int (GTK_WIDGET(editable));
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(editable),"symbol");
	u_long address = bfd_asymbol_value(sym);
	set_remote (address,&value,sizeof(char));
}


void
on_Q_ushort_changed                    (GtkEditable     *editable,
                                        gpointer         user_data)
{
	u_short value = gtk_spin_button_get_value_as_int (GTK_WIDGET(editable));
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(editable),"symbol");
	u_long address = bfd_asymbol_value(sym);
	set_remote (address,&value,sizeof(u_short));
}


void
on_Q_short_changed                     (GtkEditable     *editable,
                                        gpointer         user_data)
{
	short value = gtk_spin_button_get_value_as_int (GTK_WIDGET(editable));
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(editable),"symbol");
	u_long address = bfd_asymbol_value(sym);
	set_remote (address,&value,sizeof(short));
}


void
on_Q_long_changed                      (GtkEditable     *editable,
                                        gpointer         user_data)
{
	long value = gtk_spin_button_get_value_as_int (GTK_WIDGET(editable));
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(editable),"symbol");
	u_long address = bfd_asymbol_value(sym);
	set_remote (address,&value,sizeof(long));
}


void
on_Q_ulong_changed                     (GtkEditable     *editable,
                                        gpointer         user_data)
{
	u_long value = gtk_spin_button_get_value_as_int (GTK_WIDGET(editable));
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(editable),"symbol");
	u_long address = bfd_asymbol_value(sym);
	set_remote (address,&value,sizeof(u_long));
}


void
on_Q_svector_changed                   (GtkEditable     *editable,
                                        gpointer         user_data)
{
	svector_t value;
	GtkWidget *w;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(editable),"symbol");
	u_long address = bfd_asymbol_value(sym);

	w = lookup_widget(editable,"Q_svector_x");
	value.vx = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_svector_y");
	value.vy = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_svector_z");
	value.vz = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));

	set_remote (address,&value,sizeof(svector_t));
}


void
on_Q_rect_changed                      (GtkEditable     *editable,
                                        gpointer         user_data)
{
	rect_t value;
	GtkWidget *w;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(editable),"symbol");
	u_long address = bfd_asymbol_value(sym);

	w = lookup_widget(editable,"Q_rect_x");
	value.x = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_rect_y");
	value.y = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_rect_w");
	value.w = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_rect_h");
	value.h = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));

	set_remote (address,&value,sizeof(rect_t));
}


void
on_Q_vector_changed                    (GtkEditable     *editable,
                                        gpointer         user_data)
{
	vector_t value;
	GtkWidget *w;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(editable),"symbol");
	u_long address = bfd_asymbol_value(sym);

	w = lookup_widget(editable,"Q_vector_x");
	value.vx = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_vector_y");
	value.vy = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_vector_z");
	value.vz = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));

	set_remote (address,&value,sizeof(vector_t));
}


void
on_Q_matrix_changed                    (GtkEditable     *editable,
                                        gpointer         user_data)
{
	matrix_t value;
	GtkWidget *w;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(editable),"symbol");
	u_long address = bfd_asymbol_value(sym);

	w = lookup_widget(editable,"Q_matrix_a11");
	value.m[0][0] = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_matrix_a12");
	value.m[0][1] = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_matrix_a13");
	value.m[0][2] = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));

	w = lookup_widget(editable,"Q_matrix_a21");
	value.m[1][0] = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_matrix_a22");
	value.m[1][1] = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_matrix_a23");
	value.m[1][2] = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));

	w = lookup_widget(editable,"Q_matrix_a31");
	value.m[2][0] = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_matrix_a32");
	value.m[2][1] = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));
	w = lookup_widget(editable,"Q_matrix_a33");
	value.m[2][2] = gtk_spin_button_get_value_as_int (GTK_EDITABLE(w));

	set_remote (address,&value,sizeof(matrix_t));
}


void
on_Q_colorwheel_color_changed          (GtkColorSelection *colorselection,
                                        gpointer         user_data)
{
	gdouble color[3];
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(colorselection),"symbol");
	u_long address = bfd_asymbol_value(sym);
	cvector_t rgb;

	gtk_color_selection_get_color (colorselection,color);

	rgb.r = color[0] * 255;
	rgb.g = color[1] * 255;
	rgb.b = color[2] * 255;

	set_remote (address,&rgb,sizeof(cvector_t));
}

void
on_Q_toggle_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	int value = gtk_toggle_button_get_active (GTK_WIDGET(togglebutton));
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(togglebutton),"symbol");
	u_long address = bfd_asymbol_value(sym);
	
	
	if (value)
	{
		gtk_label_set_text (GTK_LABEL(GTK_BIN(togglebutton)->child),"ON");
	}
	else
	{
		gtk_label_set_text (GTK_LABEL(GTK_BIN(togglebutton)->child),"OFF");
	}
	
	set_remote (address,&value,sizeof(int));
}

/***********************************************************/

void
on_B_char_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget,*window = create_window_char();
	gchar *title;
	gtk_widget_show (window);
	title = g_strdup_printf ("'char %s' at 0x%08lx",
		bfd_asymbol_name (current_symbol),
		bfd_asymbol_value (current_symbol));
	gtk_window_set_title (GTK_WINDOW(window),title);
	g_free (title);
	widget = lookup_widget(window,"Q_char");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_char_reload");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	
	gtk_signal_emit_by_name (GTK_OBJECT(widget),"clicked");
}


void
on_B_uchar_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget,*window = create_window_uchar();
	gchar *title;
	gtk_widget_show (window);
	title = g_strdup_printf ("'uchar %s' at 0x%08lx",
		bfd_asymbol_name (current_symbol),
		bfd_asymbol_value (current_symbol));
	gtk_window_set_title (GTK_WINDOW(window),title);
	g_free (title);
	widget = lookup_widget(window,"Q_uchar");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_uchar_reload");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);

	gtk_signal_emit_by_name (GTK_OBJECT(widget),"clicked");
}


void
on_B_short_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget,*window = create_window_short();
	gchar *title;
	gtk_widget_show (window);
	title = g_strdup_printf ("'short %s' at 0x%08lx",
		bfd_asymbol_name (current_symbol),
		bfd_asymbol_value (current_symbol));
	gtk_window_set_title (GTK_WINDOW(window),title);
	g_free (title);
	widget = lookup_widget(window,"Q_short");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_short_reload");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	gtk_signal_emit_by_name (GTK_OBJECT(widget),"clicked");
}


void
on_B_rect_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget,*window = create_window_rect();
	gchar *title;
	gtk_widget_show (window);
	title = g_strdup_printf ("'RECT %s' at 0x%08lx",
		bfd_asymbol_name (current_symbol),
		bfd_asymbol_value (current_symbol));
	gtk_window_set_title (GTK_WINDOW(window),title);
	g_free (title);
	widget = lookup_widget(window,"Q_rect_x");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_rect_y");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_rect_w");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_rect_h");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_rect_reload");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	gtk_signal_emit_by_name (GTK_OBJECT(widget),"clicked");
}


void
on_B_ushort_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget,*window = create_window_ushort();
	gchar *title;
	gtk_widget_show (window);
	title = g_strdup_printf ("'ushort %s' at 0x%08lx",
		bfd_asymbol_name (current_symbol),
		bfd_asymbol_value (current_symbol));
	gtk_window_set_title (GTK_WINDOW(window),title);
	g_free (title);
	widget = lookup_widget(window,"Q_ushort");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_ushort_reload");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	gtk_signal_emit_by_name (GTK_OBJECT(widget),"clicked");
}


void
on_B_long_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget,*window = create_window_long();
	gchar *title;
	gtk_widget_show (window);
	title = g_strdup_printf ("'long %s' at 0x%08lx",
		bfd_asymbol_name (current_symbol),
		bfd_asymbol_value (current_symbol));
	gtk_window_set_title (GTK_WINDOW(window),title);
	g_free (title);
	widget = lookup_widget(window,"Q_long");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_long_reload");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	gtk_signal_emit_by_name (GTK_OBJECT(widget),"clicked");
}


void
on_B_ulong_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget,*window = create_window_ulong();
	gchar *title;
	gtk_widget_show (window);
	title = g_strdup_printf ("'ulong %s' at 0x%08lx",
		bfd_asymbol_name (current_symbol),
		bfd_asymbol_value (current_symbol));
	gtk_window_set_title (GTK_WINDOW(window),title);
	g_free (title);
	widget = lookup_widget(window,"Q_ulong");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_ulong_reload");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	gtk_signal_emit_by_name (GTK_OBJECT(widget),"clicked");
}


void
on_B_vector_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget,*window = create_window_vector();
	gchar *title;
	gtk_widget_show (window);
	title = g_strdup_printf ("'VECTOR %s' at 0x%08lx",
		bfd_asymbol_name (current_symbol),
		bfd_asymbol_value (current_symbol));
	gtk_window_set_title (GTK_WINDOW(window),title);
	g_free (title);

	widget = lookup_widget(window,"Q_vector_x");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_vector_y");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_vector_z");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);

	widget = lookup_widget(window,"Q_vector_reload");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	gtk_signal_emit_by_name (GTK_OBJECT(widget),"clicked");
}


void
on_B_toggle_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget,*window = create_window_toggle();
	gchar *title;
	gtk_widget_show (window);
	title = g_strdup_printf ("'int %s' at 0x%08lx",
		bfd_asymbol_name (current_symbol),
		bfd_asymbol_value (current_symbol));
	gtk_window_set_title (GTK_WINDOW(window),title);
	g_free (title);
	widget = lookup_widget(window,"Q_toggle");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_toggle_reload");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	gtk_signal_emit_by_name (GTK_OBJECT(widget),"clicked");
}


void
on_B_svector_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget,*window = create_window_svector();
	gchar *title;
	gtk_widget_show (window);
	title = g_strdup_printf ("'SVECTOR %s' at 0x%08lx",
		bfd_asymbol_name (current_symbol),
		bfd_asymbol_value (current_symbol));
	gtk_window_set_title (GTK_WINDOW(window),title);
	g_free (title);

	widget = lookup_widget(window,"Q_svector_x");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_svector_y");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_svector_z");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);

	widget = lookup_widget(window,"Q_svector_reload");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	gtk_signal_emit_by_name (GTK_OBJECT(widget),"clicked");
}


void
on_B_matrix_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget,*window = create_window_matrix();
	gchar *title;
	gtk_widget_show (window);
	title = g_strdup_printf ("'MATRIX %s' at 0x%08lx",
		bfd_asymbol_name (current_symbol),
		bfd_asymbol_value (current_symbol));
	gtk_window_set_title (GTK_WINDOW(window),title);
	g_free (title);

	widget = lookup_widget(window,"Q_matrix_a11");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_matrix_a12");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_matrix_a13");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);

	widget = lookup_widget(window,"Q_matrix_a21");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_matrix_a22");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_matrix_a23");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);

	widget = lookup_widget(window,"Q_matrix_a31");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_matrix_a32");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_matrix_a33");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);

	widget = lookup_widget(window,"Q_matrix_reload");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	gtk_signal_emit_by_name (GTK_OBJECT(widget),"clicked");
}


void
on_B_cvector_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget,*window = create_window_cvector();
	gchar *title;
	gtk_widget_show_now (window);
	title = g_strdup_printf ("'CVECTOR %s' at 0x%08lx",
		bfd_asymbol_name (current_symbol),
		bfd_asymbol_value (current_symbol));
	gtk_window_set_title (GTK_WINDOW(window),title);
	g_free (title);
	widget = lookup_widget(window,"Q_colorwheel");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	widget = lookup_widget(window,"Q_cvector_reload");
	gtk_object_set_data (GTK_OBJECT(widget),"symbol",current_symbol);
	gtk_signal_emit_by_name (GTK_OBJECT(widget),"clicked");
}


/***********************************************************/

void
on_clist_select_row                    (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	GtkWidget *w;
	current_symbol = gtk_clist_get_row_data (clist, row);

	w = lookup_widget(clist,"B_char"); gtk_widget_set_sensitive(w,TRUE);
	w = lookup_widget(clist,"B_uchar"); gtk_widget_set_sensitive(w,TRUE);
	w = lookup_widget(clist,"B_short"); gtk_widget_set_sensitive(w,TRUE);
	w = lookup_widget(clist,"B_ushort"); gtk_widget_set_sensitive(w,TRUE);
	w = lookup_widget(clist,"B_long"); gtk_widget_set_sensitive(w,TRUE);
	w = lookup_widget(clist,"B_ulong"); gtk_widget_set_sensitive(w,TRUE);
	w = lookup_widget(clist,"B_matrix"); gtk_widget_set_sensitive(w,TRUE);
	w = lookup_widget(clist,"B_svector"); gtk_widget_set_sensitive(w,TRUE);
	w = lookup_widget(clist,"B_vector"); gtk_widget_set_sensitive(w,TRUE);
	w = lookup_widget(clist,"B_cvector"); gtk_widget_set_sensitive(w,TRUE);
	w = lookup_widget(clist,"B_matrix"); gtk_widget_set_sensitive(w,TRUE);
	w = lookup_widget(clist,"B_toggle"); gtk_widget_set_sensitive(w,TRUE);
	w = lookup_widget(clist,"B_rect"); gtk_widget_set_sensitive(w,TRUE);
}

void
on_clist_unselect_row                  (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	GtkWidget *w;
	
	w = lookup_widget(clist,"B_char"); gtk_widget_set_sensitive(w,FALSE);
	w = lookup_widget(clist,"B_uchar"); gtk_widget_set_sensitive(w,FALSE);
	w = lookup_widget(clist,"B_short"); gtk_widget_set_sensitive(w,FALSE);
	w = lookup_widget(clist,"B_ushort"); gtk_widget_set_sensitive(w,FALSE);
	w = lookup_widget(clist,"B_long"); gtk_widget_set_sensitive(w,FALSE);
	w = lookup_widget(clist,"B_ulong"); gtk_widget_set_sensitive(w,FALSE);
	w = lookup_widget(clist,"B_matrix"); gtk_widget_set_sensitive(w,FALSE);
	w = lookup_widget(clist,"B_svector"); gtk_widget_set_sensitive(w,FALSE);
	w = lookup_widget(clist,"B_vector"); gtk_widget_set_sensitive(w,FALSE);
	w = lookup_widget(clist,"B_cvector"); gtk_widget_set_sensitive(w,FALSE);
	w = lookup_widget(clist,"B_matrix"); gtk_widget_set_sensitive(w,FALSE);
	w = lookup_widget(clist,"B_toggle"); gtk_widget_set_sensitive(w,FALSE);
	w = lookup_widget(clist,"B_rect"); gtk_widget_set_sensitive(w,FALSE);
}


void
on_clist_unselect_all                  (GtkCList        *clist,
                                        gpointer         user_data)
{
}

/***********************************************************/

void
on_Q_uchar_reload_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	u_char value;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(button),"symbol");
	u_long address = bfd_asymbol_value(sym);
	get_remote (address,&value,sizeof(u_char));
	gtk_spin_button_set_value (lookup_widget(button,"Q_uchar"),(gfloat)value);
}


void
on_Q_char_reload_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	char value;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(button),"symbol");
	u_long address = bfd_asymbol_value(sym);
	get_remote (address,&value,sizeof(char));
	gtk_spin_button_set_value (lookup_widget(button,"Q_char"),(gfloat)value);
}


void
on_Q_ushort_reload_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	u_short value;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(button),"symbol");
	u_long address = bfd_asymbol_value(sym);
	get_remote (address,&value,sizeof(u_short));
	gtk_spin_button_set_value (lookup_widget(button,"Q_ushort"),(gfloat)value);
}


void
on_Q_short_reload_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	short value;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(button),"symbol");
	u_long address = bfd_asymbol_value(sym);
	get_remote (address,&value,sizeof(short));
	gtk_spin_button_set_value (lookup_widget(button,"Q_short"),(gfloat)value);
}


void
on_Q_long_reload_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	long value;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(button),"symbol");
	u_long address = bfd_asymbol_value(sym);
	get_remote (address,&value,sizeof(long));
	gtk_spin_button_set_value (lookup_widget(button,"Q_long"),(gfloat)value);
}


void
on_Q_ulong_reload_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	u_long value;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(button),"symbol");
	u_long address = bfd_asymbol_value(sym);
	get_remote (address,&value,sizeof(u_long));
	gtk_spin_button_set_value (lookup_widget(button,"Q_ulong"),(gfloat)value);
}


void
on_Q_svector_reload_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	svector_t value;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(button),"symbol");
	u_long address = bfd_asymbol_value(sym);
	get_remote (address,&value,sizeof(svector_t));

	gtk_spin_button_set_value (lookup_widget(button,"Q_svector_x"),(gfloat)value.vx);
	gtk_spin_button_set_value (lookup_widget(button,"Q_svector_y"),(gfloat)value.vy);
	gtk_spin_button_set_value (lookup_widget(button,"Q_svector_z"),(gfloat)value.vz);
}


void
on_Q_rect_reload_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	rect_t value;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(button),"symbol");
	u_long address = bfd_asymbol_value(sym);
	get_remote (address,&value,sizeof(rect_t));

	gtk_spin_button_set_value (lookup_widget(button,"Q_rect_x"),(gfloat)value.x);
	gtk_spin_button_set_value (lookup_widget(button,"Q_rect_y"),(gfloat)value.y);
	gtk_spin_button_set_value (lookup_widget(button,"Q_rect_w"),(gfloat)value.w);
	gtk_spin_button_set_value (lookup_widget(button,"Q_rect_h"),(gfloat)value.h);
}


void
on_Q_vector_reload_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	vector_t value;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(button),"symbol");
	u_long address = bfd_asymbol_value(sym);
	get_remote (address,&value,sizeof(vector_t));

	gtk_spin_button_set_value (lookup_widget(button,"Q_vector_x"),(gfloat)value.vx);
	gtk_spin_button_set_value (lookup_widget(button,"Q_vector_y"),(gfloat)value.vy);
	gtk_spin_button_set_value (lookup_widget(button,"Q_vector_z"),(gfloat)value.vz);
}


void
on_Q_cvector_reload_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	gdouble color[3];
	cvector_t value;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(button),"symbol");
	u_long address = bfd_asymbol_value(sym);
	get_remote (address,&value,sizeof(cvector_t));

	color[0] = value.r / 256.0;
	color[1] = value.g / 256.0;
	color[2] = value.b / 256.0;

	gtk_color_selection_set_color (lookup_widget(button,"Q_colorwheel"),color);
}


void
on_Q_matrix_reload_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	matrix_t value;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(button),"symbol");
	u_long address = bfd_asymbol_value(sym);
	get_remote (address,&value,sizeof(matrix_t));

	gtk_spin_button_set_value (lookup_widget(button,"Q_matrix_a11"),(gfloat)value.m[0][0]);
	gtk_spin_button_set_value (lookup_widget(button,"Q_matrix_a12"),(gfloat)value.m[0][1]);
	gtk_spin_button_set_value (lookup_widget(button,"Q_matrix_a13"),(gfloat)value.m[0][2]);

	gtk_spin_button_set_value (lookup_widget(button,"Q_matrix_a21"),(gfloat)value.m[1][0]);
	gtk_spin_button_set_value (lookup_widget(button,"Q_matrix_a22"),(gfloat)value.m[1][1]);
	gtk_spin_button_set_value (lookup_widget(button,"Q_matrix_a23"),(gfloat)value.m[1][2]);

	gtk_spin_button_set_value (lookup_widget(button,"Q_matrix_a31"),(gfloat)value.m[2][0]);
	gtk_spin_button_set_value (lookup_widget(button,"Q_matrix_a32"),(gfloat)value.m[2][1]);
	gtk_spin_button_set_value (lookup_widget(button,"Q_matrix_a33"),(gfloat)value.m[2][2]);

}


void
on_Q_toggle_reload_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	long value;
	asymbol *sym = gtk_object_get_data (GTK_OBJECT(button),"symbol");
	u_long address = bfd_asymbol_value(sym);
	get_remote (address,&value,sizeof(long));
	gtk_toggle_button_set_active (lookup_widget(button,"Q_toggle"),(gfloat)value);
}


/***********************************************************/

void
on_resume_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
	int pccl, rc;

	pccl = open(pccl_device_name,O_RDWR);
	if (pccl == -1) return;

	rc = ioctl (pccl,CAEIOC_RESUME);
	if (rc == -1) perror("ioctl(CAEIOC_RESUME)");

	close (pccl);
}


void
on_halt_clicked                        (GtkButton       *button,
                                        gpointer         user_data)
{
	int pccl, rc;

	pccl = open(pccl_device_name,O_RDWR);
	if (pccl == -1) return;

	rc = ioctl (pccl,CAEIOC_CONTROL,CAETLA_DU);
	if (rc == -1) perror("ioctl(CAEIOC_CONTROL)");

	close (pccl);
}


void
on_reset_clicked                       (GtkButton       *button,
                                        gpointer         user_data)
{
	int pccl, rc;

	pccl = open(pccl_device_name,O_RDWR);
	if (pccl == -1) return;

	rc = ioctl (pccl,CAEIOC_RESET);
	if (rc == -1) perror("ioctl(CAEIOC_RESET)");

	close (pccl);
}

void
on_open_clicked                        (GtkButton       *button,
                                        gpointer         user_data)
{
	retrieve_symbols (the_filename,clist);
}

void
on_open2_clicked                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	retrieve_symbols (the_filename,clist);
}
