#include "common.h"
#include <bfd.h>

/*
void psxdev_bfd (char *filename)
{
	bfd *abfd;

	char buffer[2048];
	psxexe_hdr_t *psx = (psxexe_hdr_t *) buffer;

	memset (buffer,0,2048);

	bfd_init();

	errno = 0;
	abfd = bfd_openr (filename,0);
	if (abfd != NULL)
	{
		if (bfd_check_format (abfd,bfd_object))
		{
			asection *section;
			bfd_size_type datasize = 0;
			u_long size = 0;
			u_long base = 0xffffffff;
			u_char *mem = NULL;

			printf (_("Object file format is \"%s\"\n"),abfd->xvec->name);

			if (bfd_get_gp_size(abfd) != 0) warnf (_("GP optimized size is %d\n"),bfd_get_gp_size(abfd));
			if (bfd_get_symtab_upper_bound (abfd) != 0) errorf (_("Executable not stripped!\n"));

			for (section = abfd->sections; section != NULL; section = section->next)
			{
				if (section->flags & SEC_DEBUGGING)
				{
					warnf (_("Section \"%8s\" is not stripped\n"),section->name);
				}

				if (section->flags & SEC_HAS_CONTENTS)
				{
					printf (_("Section \"%8s\" will be exported\n"),section->name);
					size += bfd_section_size (abfd,section);
				}

				if (section->vma < base) base = section->vma;
			}

			strncpy (psx->key,"PS-X EXE",8);
			
			switch (area)
			{
				case 'i':
					strncpy(psx->title,"Sony Computer Entertainment Inc. for Japan area",60);
					break;
				case 'a':
					strncpy(psx->title,"Sony Computer Entertainment Inc. for North America area",60);
					break;
				case 'e':
					strncpy(psx->title,"Sony Computer Entertainment Inc. for Europe area",60);
					break;
			}

			if (use_gp) psx->exec.gp0 = bfd_ecoff_get_gp_value (abfd);

			ksize = size;

			if (ksize % 2048)
			{
				ksize = size + (2048 - (size % 2048));
				printf2 (_("Aligning: %d -> %d\n"),(int)size,(int)ksize);
			}

			psx->exec.pc0 = abfd->start_address;
			psx->exec.t_size = ksize;
			psx->exec.t_addr = base;
			psx->exec.s_addr = stack;
			psx->exec.s_size = 0;

			printf2 (_("PC=0x%08lx GP=0x%08lx Base=%08lx Size=0x%08lx\n"),abfd->start_address,bfd_ecoff_get_gp_value(abfd),base,size);
			
			mem = calloc (1,ksize);
			if (mem)
			{
				for (section = abfd->sections; section != NULL; section = section->next)
				{
					datasize = bfd_section_size(abfd,section);
				
					printf2 (_("Section [%8s] Address [%08lx] Length [%06lx]"),section->name,section->vma,datasize);
					if (section->flags & SEC_HAS_CONTENTS)
					{
						char *data = (bfd_byte*) (mem + (section->vma - base));
					
						printf2 (" * ");
					
						bfd_get_section_contents (abfd,section,data,0,datasize);
					}
					printf2 ("\n");
				}

				file = fopen (outname,"wb");
				if (file)
				{
					if (1 != fwrite (buffer,2048,1,file))
						iowarnf ("fwrite()");

					if (1 != fwrite (mem,ksize,1,file))
						iowarnf ("fwrite()");

					fclose (file);
				}
				else iowarnf ("open(%s)",outname);

				free (mem);
			}
		}
		else warnf ("bfd_check_format(); %s",bfd_errmsg(bfd_get_error()));
		bfd_close(abfd);
	}
	else warnf ("bfd_openr(%s); %s",filename,bfd_errmsg(bfd_get_error()));

	return EXIT_SUCCESS;
}
*/
