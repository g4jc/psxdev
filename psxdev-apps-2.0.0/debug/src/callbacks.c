/*
	TODO:
	
	- JUMP address
	- file server
	- add breakpoint
	- upload executable
	- save projects
	
	psx-nm a.out | grep " T " | awk '{print "0x" $0}' > ~/.psxdev/debug_bookmarks

	BUGS:
	
	- breakpoint refresh and crash

*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

/******************************/

#include <gtkmemory.h>
#include <gtkbreakpoints.h>
#include <gtkr3kdisasm.h>
#include <gtkhexentry.h>

#include <pccl.h>
#include <psxdev.h>

#include <pthread.h>

#include <r3k_disasm.h>

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

GtkMemory *memory;
GtkBreakpoints *breakpoints;
GtkR3kDisasm *disasm;
GtkHexEntry *hexentry;

GtkHexEntry *edit_addr;
GtkHexEntry *edit_value;
GtkHexEntry *findhex;

GtkHexEntry *registers[36];

GtkStyle *style;
gchar *configdirname = NULL;

gboolean server_active = FALSE;
u_long *mem = NULL;
	int polling = 1;


extern GtkWidget *app;

static const char * const reg_names[] = {
	"AT",	"V0",	"V1",	"A0",	"A1",	"A2",
	"A3",	"T0",	"T1",	"T2",	"T3",	"T4",
	"T5",	"T6",	"T7",	"S0",	"S1",	"S2",
	"S3",	"S4",	"S5",	"S6",	"S7",	"T8",
	"T9",	"K0",	"K1",	"GP",	"SP",	"S8",
	"RA",	"EPC",	"HI",	"LO",	"SR",	"CR"
};

/******************************/

GtkWidget *log = NULL;
GdkFont *font, *listingfont;
GdkColor fore, back;

void tty_print_mode (gboolean ok)
{
	if (ok)
	{
		gdk_color_parse ("green",&fore);
		gdk_color_parse ("black",&back);
	}
	else
	{
		gdk_color_parse ("black",&fore);
		gdk_color_parse ("white",&back);
	}
}

void db_g_print_func (const gchar *string)
{
	if (log==NULL)
	{
		log = lookup_widget(app,"log");
		font = gdk_font_load ("-b&h-lucidatypewriter-medium-r-normal-*-10-*-*-*-m-*-iso8859-1");
		gdk_color_parse ("black",&fore);
		gdk_color_parse ("white",&back);
	}
	gtk_text_insert (log,font,&fore,&back,string,strlen(string));
}

void
on_bookmark_load_clicked               (GtkButton       *button,
                                        gpointer         user_data);

/******************************/

static void
pccl_read_memory (u_long address, u_char *buf, u_long len)
{
	int file;

	if (server_active) return;

	file = open(gtk_entry_get_text(lookup_widget(app,"cfg_fullmem")),O_RDWR);
	if (file)
	{
		lseek (file,address,0);
		read (file,buf,len);
		close (file);
	}
}

static void
pccl_write_memory (u_long address, u_char *buf, u_long len)
{
	int file;
	
	if (server_active) return;

	file = open(gtk_entry_get_text(lookup_widget(app,"cfg_fullmem")),O_RDWR);
	if (file)
	{
		lseek (file,address,0);
		write (file,buf,len);
		close (file);
	}
}

static void
on_read_sync (GtkMemory *memory, gulong address, guchar *buffer, gulong size, gpointer data)
{
	gtk_hex_entry_set_value (lookup_widget(app,"pc"),memory->address);
	pccl_read_memory (memory->address, memory->buffer, memory->size);
}

static void
on_write_sync (GtkMemory *memory, gulong address, guchar *buffer, gulong size, gpointer data)
{
	pccl_write_memory (memory->address, memory->buffer, memory->size);
}

void on_register_activate (GtkHexEntry *hex_entry, gpointer reg)
{
	gulong val = gtk_hex_entry_get_value (hex_entry);
	
	g_print ("a-a-activate %d!\n",GPOINTER_TO_INT(reg));
}

void on_hex_activate (GtkHexEntry *hex_entry, GtkR3kDisasm *widget)
{
	gtk_r3k_disasm_set_address (disasm,gtk_hex_entry_get_value (hex_entry));
}

void on_edit_value (GtkHexEntry *hex_entry, GtkR3kDisasm *widget)
{
	u_long addr;
	u_long value;

	addr = gtk_hex_entry_get_value (edit_addr);
	value = gtk_hex_entry_get_value (edit_value);
	
	pccl_write_memory (addr,&value,4);
	
	g_print ("wrote 0x%08lx to address 0x%08lx.\n",value,addr);
}

void sync_bkpt_list (GtkBreakpoints *breakpoints)
{
	GtkCList *clist = lookup_widget (app,"bkpt_clist");
	GList *ptr = breakpoints->list;
	gchar *labels[7];
	gchar buf_addr[16];
	gchar buf_opc[256];
	gchar buf_1[16];
	gchar buf_2[16];
	gchar buf_3[16];
	gchar buf_4[16];
	gint row = -1;

	gtk_clist_freeze (clist);

	labels[0] = buf_addr;
	labels[2] = buf_opc;
	labels[3] = buf_1;
	labels[4] = buf_2;
	labels[5] = buf_3;
	labels[6] = buf_4;
	
	if (clist->selection)
	{
		row = GPOINTER_TO_INT(clist->selection->data);
	}
	gtk_clist_clear (clist);
	
	for (;ptr;ptr=ptr->next)
	{
		GtkBreakpoint *bkpt = (GtkBreakpoint*) ptr->data;

		g_snprintf (buf_addr,16,"0x%08lx",bkpt->address);
		labels[1] =  (bkpt->enabled) ? "ON" : "OFF";
		r3k_decode (buf_opc,256,bkpt->address,(insn_t)bkpt->safe);

		if (bkpt->hardware)
		{
			g_snprintf (buf_1,16,"0x%08lx",bkpt->pc_addr);
			g_snprintf (buf_2,16,"0x%08lx",bkpt->pc_mask);
			g_snprintf (buf_3,16,"0x%08lx",bkpt->data_addr);
			g_snprintf (buf_4,16,"0x%08lx",bkpt->data_mask);
		}
		else
		{
			strcpy (buf_1,"-");
			strcpy (buf_2,"-");
			strcpy (buf_3,"-");
			strcpy (buf_4,"-");
		}

		gtk_clist_append (clist,labels);
	}
	
	if (row!=-1) gtk_clist_select_row (clist,row,0);

	gtk_clist_thaw (clist);
}

static void
on_insert (GtkBreakpoints *breakpoints, GtkBreakpoint *breakpoint, gpointer data)
{
	g_print ("Breakpoint inserted at ");
	gdk_color_parse ("red",&fore);
	g_print ("0x%08lx\n",breakpoint->address);
	gdk_color_parse ("black",&fore);

	pccl_read_memory (breakpoint->address,(u_char*)&breakpoint->safe,4);
	pccl_write_memory (breakpoint->address,(u_char*)&breakpoint->insn,4);

	sync_bkpt_list (breakpoints);
}

static void
on_remove (GtkBreakpoints *breakpoints, GtkBreakpoint *breakpoint, gpointer data)
{
	g_print ("Breakpoint removed from ");
	gdk_color_parse ("red",&fore);
	g_print ("0x%08lx\n",breakpoint->address);
	gdk_color_parse ("black",&fore);

	pccl_write_memory (breakpoint->address,(u_char*)&breakpoint->safe,4);

	sync_bkpt_list (breakpoints);
}


static void
on_find_hex (GtkHexEntry *hex_entry, gpointer user_data)
{
	gulong begin;
	gulong end;
	gulong find;
	gulong mask;
	gchar buf[1000];
	addr_t addr;
	insn_t insn;
	opcode_t *opcode;
	gint i,hits=0,row;
	GtkCList *clist;
	
	if (mem==NULL) return;
	
	clist = lookup_widget(app,"search_clist");
	
	begin = gtk_hex_entry_get_value (lookup_widget(app,"search_from"));
	end = gtk_hex_entry_get_value (lookup_widget(app,"search_to"));
	find = gtk_hex_entry_get_value (hex_entry);
	mask = gtk_hex_entry_get_value (lookup_widget(app,"mask"));

	if (begin>=end) return;

	gtk_clist_freeze (clist);
	gtk_clist_clear (clist);

	g_print ("searching from 0x%08lx to 0x%08lx for 0x%08lx (mask=0x%08lx)...\n",begin,end,find,mask);

	{
		gchar buf2[16];
		gchar buf3[16];
		gchar *label[3];
	
		label[0] = buf2;
		label[1] = buf3;
		label[2] = buf;
	
		for (i=0; i<((end-begin)>>4); i++)
		{
			if ((mem[i]&mask) == (find&mask))
			{
				addr = begin + (i*4);
				insn = (insn_t) mem[i];
				opcode = r3k_decode (buf,1000,addr,insn);
				hits++;
				g_snprintf(buf2,16,"0x%08lx",addr);
				g_snprintf(buf3,16,"0x%08lx",mem[i]);
				row = gtk_clist_append (clist,label);
				
				gtk_clist_set_cell_style (clist,row,0,style);
				gtk_clist_set_cell_style (clist,row,1,style);
				gtk_clist_set_cell_style (clist,row,2,style);
			}
		}

	}

	gdk_color_parse ("blue",&fore);
	g_print ("%d hits found!\n\n",hits);
	gdk_color_parse ("black",&fore);

	gtk_clist_thaw (clist);
}

void on_cursor_changed (GtkR3kDisasm *r3k_disasm, gpointer data)
{
	GtkWidget *notebook = lookup_widget(app,"notebook");
	FILE *process;
	gchar *cmd;
	gchar *dbgexe = gtk_entry_get_text (lookup_widget(app,"debug_exe_entry"));
	int line = -1, size;
	struct stat st;
	char buf[1024];
		
//	if (1 != gtk_notebook_get_current_page(notebook)) return;
	if (strlen(dbgexe)==0) return;
		
	cmd = g_strdup_printf (gtk_entry_get_text(lookup_widget(app,"cfg_addr2line")),dbgexe,
		gtk_r3k_disasm_get_cursor(r3k_disasm));

	process = popen (cmd,"r");
	if (process)
	{
		
		while (fgets(buf,1024,process))
		{
			char *s = strchr(buf,':');
			
			if (s)
			{
				*s = 0;
				line = strtoul(s+1,NULL,0);
				break;
			}
		}
	
		pclose (process);
	}
	else perror(0);

	g_free (cmd);

	if (stat(buf,&st)!=-1)
	{
		cmd = g_strdup_printf (gtk_entry_get_text(lookup_widget(app,"cfg_open_editor")),line,buf);
		system (cmd);
		g_free (cmd);
	}
	else perror(0);
}

void on_pointer_changed (GtkR3kDisasm *r3k_disasm, gpointer data)
{
}

/******************************/

void init (void)
{
	configdirname = g_strdup_printf ("%s/.psxdev",g_get_home_dir());

	mkdir (configdirname,0755);

	psxdev_init ();

	gtk_signal_connect (GTK_OBJECT (lookup_widget(app,"pc")), "activate",
		GTK_SIGNAL_FUNC (on_hex_activate), memory);

	gtk_signal_connect (GTK_OBJECT (edit_value), "activate",
		GTK_SIGNAL_FUNC (on_edit_value), NULL);

	gtk_signal_connect (GTK_OBJECT (findhex), "activate",
		GTK_SIGNAL_FUNC (on_find_hex), NULL);

	g_set_print_handler (db_g_print_func);
	
	style = gtk_style_copy (lookup_widget(app,"search_clist")->style);
	style->font = gdk_font_load ("9x15");
	
	listingfont = gdk_font_load ("9x15");
	
	on_bookmark_load_clicked (0,0);
}

int psxioctl (int cmd, int param)
{
	int pccl;
	int rc = -1;
	
	pccl = open(gtk_entry_get_text(lookup_widget(app,"cfg_device")),O_RDWR);
	
	if (pccl!=-1)
	{
		rc = ioctl(pccl,cmd, param);
		
		if (rc == -1) perror(0);
		
		close (pccl);
	}
	else perror(0);

	return rc;
}

/******************************/

gboolean
on_app_delete_event                    (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gtk_main_quit();
  return FALSE;
}


void
on_button_exit_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_main_quit();
}


void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_main_quit();
}


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_widget_show (create_about());
}


void
on_button_reset_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	psxioctl(CAEIOC_RESET,0);

//  gtk_widget_show (create_messagebox1());
//  gtk_widget_show (create_messagebox2());
}


void
on_button_resume_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	caetla_device_mode_t mode;

	mode.mode = PCCL_MODE_AUTO_RESUME;
	mode.value = 1;
	psxioctl (CAEIOC_DEVICE_SET_MODE,&mode);

	psxioctl (CAEIOC_RESUME,0);

	gnome_appbar_set_status (lookup_widget(app,"appbar"),"PlayStation is in control...");
	g_print ("Continuing execution on remote target...\n");
}


void
on_button_halt_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	caetla_registers_t regs;
	pspar_status_t st;
	gint i,j;
	gulong a;
	int rc;
	caetla_device_mode_t mode;

	if (polling==1)
	{
		polling = 0;
		return;
	}

	psxioctl (CAEIOC_GET_STATUS_PSPAR,&st);
	
	if (st==PSPAR_IN_MENU)
	{
		gtk_widget_show (create_messagebox1());
		return;
	}

	mode.mode = PCCL_MODE_AUTO_RESUME;
	mode.value = 0;
	psxioctl (CAEIOC_DEVICE_SET_MODE,&mode);

	psxioctl (CAEIOC_CONTROL,CAETLA_DU);

	psxioctl (CAEIOC_DU_GET_REGISTERS,&regs);

	for (i=0; i<36; i++)
	{
		gulong old = gtk_hex_entry_get_value (registers[i]);
		
		gtk_hex_entry_set_value (registers[i],regs.data[i]);
		
		if (old != regs.data[i]) gtk_hex_entry_value_changed(registers[i]);
	}

	a = (disasm->rows/2) * 4;

	gtk_r3k_disasm_set_address (disasm, regs.data[MIPSREG_EPC]-a);

	gtk_r3k_disasm_set_pointer (disasm, regs.data[MIPSREG_EPC]);

	disasm->cursor = regs.data[MIPSREG_EPC];

	g_print ("Execution on remote target halted at PC=");
	gdk_color_parse ("red",&fore);
	g_print ("0x%08lx\n\n",regs.data[MIPSREG_EPC]);
	gdk_color_parse ("black",&fore);

	gnome_appbar_set_status (lookup_widget(app,"appbar"),"PC is in control...");
}


void
on_button_step_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	caetla_registers_t regs;
	caetla_register_t reg;
	gint i,j;
	gulong a;
	gulong current;
	gulong next;
	int dev,rc;
	caetla_status_t st;
	GtkBreakpoint step_bkpt;
	caetla_word_t safe, safe1, safe2;
	caetla_word_t bkpt, bkpt1, bkpt2;
	cr_t *cr;
	gboolean delay_slot;
	GtkBreakpoint *tbkpt;

	dev = open (gtk_entry_get_text(lookup_widget(app,"cfg_device")),O_RDWR);
	if (dev == -1) return;

	rc = ioctl (dev,CAEIOC_CONTROL,CAETLA_DU);
	if (rc==-1) g_error("%s = %s","CAEIOC_CONTROL",g_strerror(rc),close(dev));

	rc = ioctl (dev,CAEIOC_CONSOLE,0);
	if (rc==-1) g_error("%s = %s","CAEIOC_CONSOLE",g_strerror(rc),close(dev));

	rc = ioctl (dev,CAEIOC_DU_GET_REGISTERS,&regs);
	if (rc==-1) g_error("%s = %s","CAEIOC_DU_GET_REGISTERS",g_strerror(rc),close(dev));

	rc = ioctl (dev,CAEIOC_DU_FLUSH_ICACHE,0);
	if (rc==-1) g_error("%s = %s","CAEIOC_DU_FLUSH_ICACHE",g_strerror(rc),close(dev));

	cr = (cr_t*) &(regs.data[MIPSREG_CR]);
	delay_slot = (cr->bd == 1);
	
	if (delay_slot)
	{
		opcode_t *opcode;
	
		g_print ("epc in branch delay slot, forking..\n");

		safe1.address = regs.data[MIPSREG_EPC];
		safe1.data    = 0;
		rc = ioctl (dev,CAEIOC_DU_READ_WORD,&safe1);
		if (rc==-1) g_error("%s = %s","CAEIOC_DU_READ_WORD",g_strerror(rc),close(dev));

		// decode
		opcode = r3k_find_opcode (safe1.address,(insn_t)safe1.data);

		safe1.address = regs.data[MIPSREG_EPC]+8;
		safe1.data    = 0;
		rc = ioctl (dev,CAEIOC_DU_READ_WORD,&safe1);
		if (rc==-1) g_error("%s = %s","CAEIOC_DU_READ_WORD",g_strerror(rc),close(dev));

		bkpt1.address = regs.data[MIPSREG_EPC]+8;
		bkpt1.data    = 0x10d;
		rc = ioctl (dev,CAEIOC_DU_WRITE_WORD,&bkpt1);
		if (rc==-1) g_error("%s = %s","CAEIOC_DU_WRITE_WORD",g_strerror(rc),close(dev));

		safe2.address = 0;
		if (opcode)
		{
			if (opcode->has_branch_target)
			{
				safe2.address = opcode->branch_target;
				g_print ("0x%08lx\n",safe2.address);
			}
			if (opcode->use_branch_register)
			{
				if (opcode->branch_register>0)
				safe2.address = regs.data[opcode->branch_register-1];
				g_print ("0x%08lx (branch register)\n",safe2.address);
			}
		}
		safe2.data    = 0;
		rc = ioctl (dev,CAEIOC_DU_READ_WORD,&safe2);
		if (rc==-1) g_error("%s = %s","CAEIOC_DU_READ_WORD",g_strerror(rc),close(dev));

		bkpt2.address = safe2.address;
		bkpt2.data    = 0x20d;
		rc = ioctl (dev,CAEIOC_DU_WRITE_WORD,&bkpt2);
		if (rc==-1) g_error("%s = %s","CAEIOC_DU_WRITE_WORD",g_strerror(rc),close(dev));
	}
	else
	{
		safe.address = regs.data[MIPSREG_EPC]+4;
		safe.data    = 0;
		rc = ioctl (dev,CAEIOC_DU_READ_WORD,&safe);
		if (rc==-1) g_error("%s = %s","CAEIOC_DU_READ_WORD",g_strerror(rc),close(dev));

		bkpt.address = regs.data[MIPSREG_EPC]+4;
		bkpt.data    = 0xb0d;
		rc = ioctl (dev,CAEIOC_DU_WRITE_WORD,&bkpt);
		if (rc==-1) g_error("%s = %s","CAEIOC_DU_WRITE_WORD",g_strerror(rc),close(dev));
	}

	// check if insn at current epc is a breakpoint. if yes, NOP it temporary

	tbkpt = gtk_breakpoints_find(breakpoints,regs.data[MIPSREG_EPC]);
	if (tbkpt != NULL)
	{
		g_print ("stepping into breakpoint!\n");
	}

	{
		caetla_device_mode_t mode;
		int oldmode;

		mode.mode = PCCL_MODE_AUTO_RESUME;

		rc = ioctl (dev,CAEIOC_DEVICE_GET_MODE,&mode);
		if (rc==-1) g_error("%s = %s","CAEIOC_DEVICE_GET_MODE",g_strerror(rc),close(dev));
		oldmode = mode.value;

		mode.value = 1;
		rc = ioctl (dev,CAEIOC_DEVICE_SET_MODE,&mode);
		if (rc==-1) g_error("%s = %s","CAEIOC_DEVICE_SET_MODE",g_strerror(rc),close(dev));

		rc = ioctl (dev,CAEIOC_RESUME,0);
		if (rc==-1) g_error("%s = %s","CAEIOC_RESUME",g_strerror(rc),close(dev));

		mode.value = oldmode;
		rc = ioctl (dev,CAEIOC_DEVICE_SET_MODE,&mode);
		if (rc==-1) g_error("%s = %s","CAEIOC_DEVICE_SET_MODE",g_strerror(rc),close(dev));
	}


	while (1)
	{
		rc = ioctl (dev,CAEIOC_GET_STATUS_CAETLA,&st);
		if (rc==-1) g_error("%s = %s","CAEIOC_GET_STATUS_CAETLA",g_strerror(rc),close(dev));

		g_print ("%02x\n",st);
		if (st & CAESTF_BRK_STOP) break;
	}

	rc = ioctl (dev,CAEIOC_CONTROL,CAETLA_DU);
	if (rc==-1) g_error("%s = %s","CAEIOC_CONTROL",g_strerror(rc),close(dev));

	rc = ioctl (dev,CAEIOC_DU_GET_REGISTERS,&regs);
	if (rc==-1) g_error("%s = %s","CAEIOC_DU_GET_REGISTERS",g_strerror(rc),close(dev));

	if (delay_slot)
	{
		rc = ioctl (dev,CAEIOC_DU_WRITE_WORD,&safe1);
		if (rc==-1) g_error("%s = %s","CAEIOC_DU_WRITE_WORD",g_strerror(rc),close(dev));

		rc = ioctl (dev,CAEIOC_DU_WRITE_WORD,&safe2);
		if (rc==-1) g_error("%s = %s","CAEIOC_DU_WRITE_WORD",g_strerror(rc),close(dev));
	}
	else
	{
		rc = ioctl (dev,CAEIOC_DU_WRITE_WORD,&safe);
		if (rc==-1) g_error("%s = %s","CAEIOC_DU_WRITE_WORD",g_strerror(rc),close(dev));
	}

	rc = ioctl (dev,CAEIOC_CONSOLE,1);
	if (rc==-1) g_error("%s = %s","CAEIOC_CONSOLE",g_strerror(rc),close(dev));

	{
		cr_t *cr = (cr_t*) &(regs.data[MIPSREG_CR]);
		g_print ("pc=0x%08lx sr=0x%08lx ",regs.data[MIPSREG_EPC],regs.data[MIPSREG_SR]);
		g_print ("ExceptionCode=%d BD=%d CE=%d IP=%d\n",
			cr->exccode,cr->bd,cr->ce,cr->ip);
	}

	if ((regs.data[MIPSREG_CR] & 0x80000000) == 0x80000000)
	{
		regs.data[MIPSREG_EPC] += 4;

		g_print ("BD bit set (indicating a branch delay)\n");
	
//		reg.number = MIPSREG_EPC;
//		reg.data = regs.data[MIPSREG_EPC];
//		rc = ioctl (dev,CAEIOC_DU_SET_REGISTER,&reg);
//		if (rc==-1) g_error("%s = %s","CAEIOC_DU_SET_REGISTER",g_strerror(rc),close(dev));
	}

	close(dev);

	for (i=0; i<36; i++)
	{
		gulong old = gtk_hex_entry_get_value (registers[i]);
		
		gtk_hex_entry_set_value (registers[i],regs.data[i]);
		
		if (old != regs.data[i]) gtk_hex_entry_value_changed(registers[i]);
	}

	a = (disasm->rows/2) * 4;
	gtk_r3k_disasm_set_address (disasm, regs.data[MIPSREG_EPC]-a);
	gtk_r3k_disasm_set_pointer (disasm, regs.data[MIPSREG_EPC]);
}


GtkWidget*
create_pc (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	GtkWidget *widget;
	
	widget = gtk_hex_entry_new (0,0);
	gtk_widget_show (widget);

	return widget;
}


GtkWidget*
create_registers (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	GtkWidget *table, *frame, *widget;
	int i,j,n;
	
	table = gtk_table_new (9, 8, FALSE);
	gtk_widget_show (table);

	n = 0;
	for (j=0;j<9;j++)
	{
		for (i=0;i<8;i++)
		{
			if (i&1 == 1)
			{
				frame = gtk_frame_new (NULL);
				gtk_frame_set_shadow_type (frame,GTK_SHADOW_IN);
				gtk_widget_show (frame);

				widget = gtk_hex_entry_new (8,0x0);
				gtk_widget_show (widget);
				gtk_container_add (GTK_CONTAINER (frame), widget);

				registers[n] = widget;

				gtk_signal_connect (GTK_OBJECT (widget), "activate",
					GTK_SIGNAL_FUNC (on_register_activate), GINT_TO_POINTER(n));

				n++;
			}
			else
			{
				frame = gtk_label_new (reg_names[n]);
				gtk_widget_show (frame);
			}

			gtk_table_attach (GTK_TABLE (table), frame, i, i+1, j, j+1,
				(GtkAttachOptions) GTK_EXPAND | GTK_FILL,
				(GtkAttachOptions) GTK_EXPAND | GTK_FILL, 2, 2);
		}
	}

	return table;
}


GtkWidget*
create_disassembly (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	memory = GTK_MEMORY(gtk_memory_new (0x80010000, 0));

	gtk_signal_connect (GTK_OBJECT (memory), "read_sync",
		GTK_SIGNAL_FUNC (on_read_sync), NULL);

	gtk_signal_connect (GTK_OBJECT (memory), "write_sync",
		GTK_SIGNAL_FUNC (on_write_sync), NULL);

	/**************************/

	breakpoints = GTK_BREAKPOINTS(gtk_breakpoints_new ());

	gtk_signal_connect (GTK_OBJECT (breakpoints), "insert",
		GTK_SIGNAL_FUNC (on_insert), NULL);

	gtk_signal_connect (GTK_OBJECT (breakpoints), "remove",
		GTK_SIGNAL_FUNC (on_remove), NULL);

	/**************************/

	disasm = gtk_r3k_disasm_new (memory, breakpoints);

	gtk_signal_connect (GTK_OBJECT (disasm), "pointer_changed",
		GTK_SIGNAL_FUNC (on_pointer_changed), NULL);

	gtk_signal_connect (GTK_OBJECT (disasm), "cursor_changed",
		GTK_SIGNAL_FUNC (on_cursor_changed), NULL);

	gtk_widget_show (disasm);

	return disasm;
}


GtkWidget*
create_search_from (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	GtkWidget *widget;
	widget = gtk_hex_entry_new (0,0);
	gtk_widget_show (widget);
	return widget;
}


GtkWidget*
create_search_to (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	GtkWidget *widget;
	widget = gtk_hex_entry_new (0,0);
	gtk_widget_show (widget);
	return widget;
}


void
on_log_clear_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *log = lookup_widget (app,"log");

	gtk_text_set_point (log,0);
	gtk_text_forward_delete (log,gtk_text_get_length(log));
}


void
on_log_status_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	int file;
	char buf[BUFSIZ];

	g_print ("Status of remote target:\n");

	gdk_color_parse ("blue",&fore);
	file = open(gtk_entry_get_text(lookup_widget(app,"cfg_status")),O_RDONLY);
	if (file)
	{
		int len;
		len = read (file,buf,BUFSIZ);
		buf[len]=0;
		g_print (buf);
		close (file);
	}
	else perror (0);
	gdk_color_parse ("black",&fore);
}


void
on_log_versions_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	int file;
	char buf[BUFSIZ];

	g_print ("Versions of caetla subsystems:\n");

	gdk_color_parse ("red",&fore);
	file = open(gtk_entry_get_text(lookup_widget(app,"cfg_version")),O_RDONLY);
	if (file)
	{
		int len;
		len = read (file,buf,BUFSIZ);
		buf[len]=0;
		g_print (buf);
		close (file);
	}
	else perror (0);
	gdk_color_parse ("black",&fore);
	g_print ("\n");
}


void
on_search_clist_select_row             (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gchar *label = NULL;
	u_long addr;

	gtk_clist_get_text (clist,row,0,&label);

	addr = strtoul (label,NULL,0);
	gtk_r3k_disasm_set_address (disasm,addr);
}


void
on_search_entry_activate               (GtkEditable     *editable,
                                        gpointer         user_data)
{
	u_long begin;
	u_long end;
	gchar *pattern;
	gchar buf[1000];
	addr_t addr;
	insn_t insn;
	opcode_t *opcode;
	int i,hits=0,row;
	GtkCList *clist;
	
	if (mem==NULL) return;
	
	clist = lookup_widget(app,"search_clist");
	
	begin = gtk_hex_entry_get_value (lookup_widget(app,"search_from"));
	end = gtk_hex_entry_get_value (lookup_widget(app,"search_to"));

	if (begin>=end) return;

	pattern = gtk_entry_get_text (lookup_widget(app,"search_entry"));

	if (pattern==NULL) return;

	gtk_clist_freeze (clist);
	gtk_clist_clear (clist);

	g_print ("searching from 0x%08lx to 0x%08lx for \"%s\"...\n",begin,end,pattern);

	{
		char buf2[16], buf3[16];
		gchar *label[3];
	
		label[0] = buf2;
		label[1] = buf3;
		label[2] = buf;
	
		for (i=0; i<((end-begin)>>4); i++)
		{
			addr = begin + (i*4);
			insn = (insn_t) mem[i];

			opcode = r3k_decode (buf,1000,addr,insn);
			
			if (strstr(buf,pattern))
			{
				hits++;
				g_snprintf(buf2,16,"0x%08lx",addr);
				g_snprintf(buf3,16,"0x%08lx",mem[i]);
				row = gtk_clist_append (clist,label);
				
				gtk_clist_set_cell_style (clist,row,0,style);
				gtk_clist_set_cell_style (clist,row,1,style);
				gtk_clist_set_cell_style (clist,row,2,style);
			}
		}

	}

	gdk_color_parse ("blue",&fore);
	g_print ("%d hits found!\n\n",hits);
	gdk_color_parse ("black",&fore);

	gtk_clist_thaw (clist);
}

GtkWidget*
create_edit_addr (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	edit_addr = gtk_hex_entry_new (0,0);
	gtk_widget_show (edit_addr);
	return edit_addr;
}


GtkWidget*
create_edit_value (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	edit_value = gtk_hex_entry_new (0,0);
	gtk_widget_show (edit_value);
	return edit_value;
}


void
on_load_memory_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	u_long begin;
	u_long end;
	
	begin = gtk_hex_entry_get_value (lookup_widget(app,"search_from"));
	end = gtk_hex_entry_get_value (lookup_widget(app,"search_to"));

	if (begin>=end) return;

	g_free (mem);
	mem = g_malloc (end-begin);
	if (mem)
	{	
		pccl_read_memory (begin,mem,end-begin);
	}

	gtk_widget_set_sensitive (lookup_widget(app,"search_entry"),TRUE);
	gtk_widget_set_sensitive (lookup_widget(app,"findhex"),TRUE);
	gtk_widget_set_sensitive (lookup_widget(app,"mask"),TRUE);
	
	gtk_hex_entry_set_value (lookup_widget(app,"mask"),0xFFFFFFFF);
}


GtkWidget*
create_findhex (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	findhex = gtk_hex_entry_new (0,0);
	gtk_widget_show (findhex);
	return findhex;
}



void server (void)
{
	int dev;
	
	dev = open (gtk_entry_get_text(lookup_widget(app,"cfg_device")),O_RDWR);
	if (dev)
	{
		psxdev_server (dev,1,0);
		close (dev);
	}
}

void server_mode (gboolean server_active)
{
	gtk_widget_set_sensitive (lookup_widget(app,"frame_results"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"frame_memory"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"frame_goto"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"frame_search"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"frame_registers"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"log_status"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"log_versions"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"button_reset"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"button_resume"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"button_step"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"button_exit"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"button_resume_with_server"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"frame_disasm"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"frame_breakpoints"),server_active);
	gtk_widget_set_sensitive (lookup_widget(app,"frame_jump"),server_active);
}

GtkWidget*
create_hexentry (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	GtkWidget *widget = gtk_hex_entry_new (int2,int1);
	gtk_widget_show (widget);
	return widget;
}

static GtkBreakpoint *bkpt = NULL;

void
on_bkpt_clist_select_row               (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	bkpt = (GtkBreakpoint *) g_list_nth (breakpoints->list,row)->data;
	
	if (bkpt)
	{
		gtk_hex_entry_set_value (lookup_widget(app,"pc_addr"),bkpt->pc_addr);
		gtk_hex_entry_set_value (lookup_widget(app,"pc_mask"),bkpt->pc_mask);
		gtk_hex_entry_set_value (lookup_widget(app,"data_addr"),bkpt->data_addr);
		gtk_hex_entry_set_value (lookup_widget(app,"data_mask"),bkpt->data_mask);

		gtk_widget_set_sensitive (lookup_widget(app,"frame_masks"),bkpt->hardware);
		gtk_widget_set_sensitive (lookup_widget(app,"bkpt_remove"),TRUE);
		gtk_widget_set_sensitive (lookup_widget(app,"bkpt_enable"),TRUE);
		gtk_widget_set_sensitive (lookup_widget(app,"bkpt_hardware"),TRUE);

		gtk_toggle_button_set_active (lookup_widget(app,"bkpt_enable"),bkpt->enabled);
		gtk_toggle_button_set_active (lookup_widget(app,"bkpt_hardware"),bkpt->hardware);
	}
}


void
on_bkpt_clist_unselect_row             (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	bkpt = NULL;
	gtk_widget_set_sensitive (lookup_widget(app,"frame_masks"),FALSE);
	gtk_widget_set_sensitive (lookup_widget(app,"bkpt_remove"),FALSE);
	gtk_widget_set_sensitive (lookup_widget(app,"bkpt_enable"),FALSE);
	gtk_widget_set_sensitive (lookup_widget(app,"bkpt_hardware"),FALSE);
}


void
on_bkpt_add_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_bkpt_remove_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
//	pccl_write_memory (breakpoint->address,(u_char*)&breakpoint->safe,4);
	if (bkpt)
	{
		gtk_breakpoints_remove (breakpoints,bkpt);
	}
}


void
on_bkpt_enable_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if (bkpt)
	{
		bkpt->enabled = gtk_toggle_button_get_active(togglebutton);
		sync_bkpt_list (breakpoints);
	}
}


void
on_bkpt_hardware_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if (bkpt)
	{
		bkpt->hardware = gtk_toggle_button_get_active(togglebutton);

		gtk_widget_set_sensitive (lookup_widget(app,"frame_masks"),bkpt->hardware);

		sync_bkpt_list (breakpoints);
	}
}

void
on_bookmark_clist_select_row           (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gchar *comment;
	gulong addr;

	gtk_clist_get_text (clist,row,1,&comment);
	gtk_entry_set_text (lookup_widget(app,"entry_comment"),comment);

	gtk_widget_set_sensitive (lookup_widget(app,"entry_comment"),TRUE);
	gtk_widget_set_sensitive (lookup_widget(app,"bookmark_remove"),TRUE);

	gtk_clist_get_text (clist,row,0,&comment);
	
	addr = strtoul (comment,0,0);
	gtk_r3k_disasm_set_address (disasm,addr);
}


void
on_bookmark_clist_unselect_row         (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gtk_widget_set_sensitive (lookup_widget(app,"entry_comment"),TRUE);
	gtk_widget_set_sensitive (lookup_widget(app,"bookmark_remove"),TRUE);
}


void
on_entry_comment_changed               (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gint row;
	GtkCList *clist = lookup_widget (app,"bookmark_clist");
	
	if (clist->selection)
	{
		gchar *label;
	
		row = GPOINTER_TO_INT(clist->selection->data);

		label = gtk_entry_get_text (lookup_widget(app,"entry_comment"));
		
		gtk_clist_set_text (clist,row,1,label);
	}
}


void
on_bookmark_add_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkCList *clist = lookup_widget (app,"bookmark_clist");
	gulong address;
	gchar *labels[2];
	gchar buf1[32];
	
	address = gtk_hex_entry_get_value (lookup_widget(app,"pc"));
	
	labels[0] = buf1;
	labels[1] = gtk_entry_get_text (lookup_widget(app,"entry_comment"));
	
	g_snprintf (buf1,32,"0x%08lx",address);
	
	gtk_clist_append (clist,labels);
}


void
on_bookmark_remove_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	gint row;
	GtkCList *clist = lookup_widget (app,"bookmark_clist");
	
	if (clist->selection)
	{
		row = GPOINTER_TO_INT(clist->selection->data);
		gtk_clist_remove (clist,row);
	}
}


void
on_bookmark_save_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	gchar *filename = g_strdup_printf ("%s/debug_bookmarks",configdirname);
	FILE *file;
	
	file = fopen (filename,"w");
	if (file)
	{
		GtkCList *clist = lookup_widget(app,"bookmark_clist");
		gint i;
		
		for (i=0; i<clist->rows; i++)
		{
			gchar *a1, *a2;
		
			gtk_clist_get_text (clist,i,0,&a1);
			gtk_clist_get_text (clist,i,1,&a2);
			
			fprintf (file,"%s %s\n",a1,a2);
		}
		fclose (file);
	}
	
	g_free (filename);
}



void
on_bookmark_clear_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkCList *clist = lookup_widget(app,"bookmark_clist");
	gtk_clist_freeze (clist);
	gtk_clist_clear (clist);
	gtk_clist_thaw (clist);
}


void
on_bookmark_load_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	gchar *filename = g_strdup_printf ("%s/debug_bookmarks",configdirname);
	FILE *file;
	
	file = fopen (filename,"r");
	if (file)
	{
		GtkCList *clist = lookup_widget(app,"bookmark_clist");
		gchar buf[1000];
		gchar *labels[2];

		gtk_clist_freeze (clist);
		gtk_clist_clear (clist);
		
		while (fgets (buf,1000,file))
		{
			buf[strlen(buf)-1]=0;
			
			buf[10]=0;
			labels[0] = buf;
			labels[1] = buf+11;
			gtk_clist_append (clist,labels);
		}

		gtk_clist_thaw (clist);

		fclose (file);
	}
	
	g_free (filename);
}

void
on_exe_run_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	gchar *filename = gtk_entry_get_text (lookup_widget(app,"exe_entry"));
	gchar *device = gtk_entry_get_text (lookup_widget(app,"device_entry"));
	gchar *command = gtk_entry_get_text (lookup_widget(app,"command_entry"));
	gchar *directory = gtk_entry_get_text (lookup_widget(app,"directory_entry"));
	gulong stack = gtk_hex_entry_get_value (lookup_widget(app,"exe_stack"));
	gulong task = gtk_hex_entry_get_value (lookup_widget(app,"exe_task"));
	gulong event = gtk_hex_entry_get_value (lookup_widget(app,"exe_event"));
	gboolean reset = gtk_toggle_button_get_active (lookup_widget(app,"toggle_reset"));
	gboolean tbreak = gtk_toggle_button_get_active (lookup_widget(app,"toggle_break"));
	gboolean usedir = gtk_toggle_button_get_active (lookup_widget(app,"toggle_usedir"));
	gboolean usestack = gtk_toggle_button_get_active (lookup_widget(app,"toggle_usestack"));
	GtkWidget *nb = lookup_widget(app,"notebook");
	gchar *stackstring = NULL;

	gchar *cmd;
	
	if (strlen(filename))
	{
		if (usedir) directory = g_dirname (filename);
		if (usestack==FALSE)
		{
			stackstring = g_strdup_printf ("--stack=%08lx",stack);
		}
	
	
		cmd = g_strdup_printf (
		"( cd %s && %s %s%s -v %s %s --task=0x%08lx --event=0x%08lx %s %s %s )",
			directory,
			command,
			strlen(device)?"--device=":"",
			strlen(device)?device:"",
			tbreak?"--server":"--console",
			stackstring?stackstring:"",
			task,
			event,
			reset?"--reset":"",
			tbreak?"--break":"",
			filename
			);

		gtk_notebook_set_page (nb,0);
		g_print ("Executing %s...\n",filename);
		gtk_main_iteration();

		tty_print_mode(TRUE);
		{
			FILE *process;
			
			process = popen(cmd,"r");
			if (process)
			{
				char buf[1024];
				
				while (fgets(buf,1024,process))
				{
					g_print (buf);
					gtk_main_iteration();
				}
				
				pclose (process);
			}
		}
		tty_print_mode(FALSE);

		polling = 0;
		if (tbreak)
			gtk_button_clicked (lookup_widget(app,"button_halt"));
		else
			gtk_button_clicked (lookup_widget(app,"button_resume_with_server"));

		if (usedir) g_free (directory);
		g_free (stackstring);
		g_free (cmd);
	}
	else
	{
		gtk_notebook_set_page (nb,2);
		gtk_widget_show (create_box_filename_needed());
		gtk_widget_grab_focus (lookup_widget(app,"exe_entry"));
	}
}


void
on_exe_info_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_exe_save_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	gchar *filename = gtk_entry_get_text (lookup_widget(app,"exe_entry"));
	gchar *device = gtk_entry_get_text (lookup_widget(app,"device_entry"));
	gchar *command = gtk_entry_get_text (lookup_widget(app,"command_entry"));
	gchar *directory = gtk_entry_get_text (lookup_widget(app,"directory_entry"));
	gulong stack = gtk_hex_entry_get_value (lookup_widget(app,"exe_stack"));
	gulong task = gtk_hex_entry_get_value (lookup_widget(app,"exe_task"));
	gulong event = gtk_hex_entry_get_value (lookup_widget(app,"exe_event"));
	gboolean reset = gtk_toggle_button_get_active (lookup_widget(app,"toggle_reset"));
	gboolean tbreak = gtk_toggle_button_get_active (lookup_widget(app,"toggle_break"));
	gboolean usedir = gtk_toggle_button_get_active (lookup_widget(app,"toggle_usedir"));
	gboolean usestack = gtk_toggle_button_get_active (lookup_widget(app,"toggle_usestack"));
}


void
on_exe_load_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{

}

void input_function (gpointer data, gint source, GdkInputCondition condition)
{
	int i,n,m;
	gchar *buffer;
		
	i = ioctl (source,FIONREAD,&n);
	if (i != -1)
	{
		if (n > 0)
		{
			buffer = g_malloc (n+1);
			m = read(source,buffer,n);
			if (m == n)
			{
				buffer[n] = 0;
				g_print (buffer);
			}
			g_free (buffer);
		}
	}
}

typedef struct {
	int dev;
	int fd;
} server_param_t;

pthread_t thread;

void server_thread (server_param_t *param)
{
	caetla_status_t cst;
	pspar_status_t pst;
	int rc;

	sleep (1);

	do {
		rc = psxdev_server(param->dev,0,param->fd);
		if (rc == -1) { perror("psxdev_server"); break; }

		cst = (caetla_status_t) rc;

		if (
			(cst & CAESTF_HBP_STOP) ||
			(cst & CAESTF_BRK_STOP) ||
			(cst & CAESTF_IN_CONTROL) )
		{
			if (cst & CAESTF_HBP_STOP) break;
			if (cst & CAESTF_BRK_STOP) break;
			if (cst & CAESTF_IN_CONTROL) break;
		}
		else
		{
			rc = ioctl (param->dev,CAEIOC_GET_STATUS_PSPAR,&pst);
			if (rc == -1) { perror ("ioctl(CAEIOC_GET_STATUS_PSPAR)"); break; }

			if (pst == PSPAR_IN_MENU) break;
		}

	} while (polling);
	
	polling = 0;
	
	pthread_exit (0);
}

void
on_button_resume_with_server_clicked   (GtkButton       *button,
                                        gpointer         user_data)
{
	server_param_t param;
	caetla_status_t cst;
	int dev, rc;
	gchar *directory = gtk_entry_get_text (lookup_widget(app,"directory_entry"));
	gchar *filename = gtk_entry_get_text (lookup_widget(app,"exe_entry"));
	gboolean usedir = gtk_toggle_button_get_active (lookup_widget(app,"toggle_usedir"));
	char buf[1024];

	if (usedir) directory = g_dirname (filename);

	server_mode (FALSE);

	polling = 1;

	dev = open (gtk_entry_get_text(lookup_widget(app,"cfg_device")),O_RDWR);
	if (dev)
	{
		int olddir = chdir (directory);
		if (olddir==-1) perror (directory);
		else
		{
			int pipes[2];
			guint handler;
			caetla_device_mode_t mode;
			int oldmode;
			pid_t pid;
			gboolean leave;

			mode.mode = PCCL_MODE_AUTO_RESUME;

			rc = ioctl (dev,CAEIOC_DEVICE_GET_MODE,&mode);
			oldmode = mode.value;
			mode.value = 1;
			rc = ioctl (dev,CAEIOC_DEVICE_SET_MODE,&mode);
			rc = ioctl (dev,CAEIOC_CONSOLE,1);
			rc = ioctl (dev,CAEIOC_RESUME);
			
			pipe (pipes);

			g_print ("Attaching standard I/O and file server...\n");
			tty_print_mode(TRUE);

			polling = 1;
			param.dev = dev;
			param.fd = pipes[1];

			pthread_create (&thread,NULL,server_thread,&param);

			handler = gtk_input_add_full (pipes[0],GDK_INPUT_READ,input_function,
				NULL,NULL,NULL);

			while (polling==1)
			{
				gtk_main_iteration();
			}
			pthread_join (thread,NULL);
					
			gtk_input_remove (handler);
			close (pipes[0]);
			close (pipes[1]);

			tty_print_mode(FALSE);
			g_print ("Detaching standard I/O and file server...\n");
			
			rc = ioctl (dev,CAEIOC_CONSOLE,0);
			rc = ioctl (dev,CAEIOC_DEVICE_SET_MODE,&oldmode);
			close (dev);
			fchdir (olddir);
		}
	}

	server_mode (TRUE);

	gtk_button_clicked (lookup_widget(app,"button_halt"));

	if (usedir) g_free (directory);
}


void
on_generate_bookmarks1_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	FILE *process;
	gchar *cmd;
	gchar *dbgexe = gtk_entry_get_text (lookup_widget(app,"debug_exe_entry"));
	int line = -1, size;
		
	if (strlen(dbgexe)==0)
	{
		gtk_widget_show (create_box_debugexe_needed());
		return;
	}
		
	cmd = g_strdup_printf (gtk_entry_get_text(lookup_widget(app,"cfg_getsymbols")),dbgexe);

	process = popen (cmd,"r");
	if (process)
	{
		GtkCList *clist = lookup_widget(app,"bookmark_clist");
		gchar buf[1000];
		gchar *labels[2];

		gtk_clist_freeze (clist);
		gtk_clist_clear (clist);

		while (fgets (buf,1000,process))
		{
			buf[strlen(buf)-1]=0;

			buf[10]=0;
			labels[0] = buf;
			labels[1] = buf+11;
			gtk_clist_append (clist,labels);
		}

		gtk_clist_thaw (clist);
		pclose (process);
	}
	else perror(0);

	g_free (cmd);
}
