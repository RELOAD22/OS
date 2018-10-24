#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write_bootblock(FILE *image, FILE *bbfile, Elf32_Phdr *Phdr);
Elf32_Phdr *read_exec_file(FILE *opfile);
uint8_t count_kernel_sectors(Elf32_Phdr *Phdr);
void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz);

void write_filesegement_image(FILE *image, FILE *file, Elf32_Phdr *phdr)
{
    //按照存储在程序头表中的内容找到对应段存入image
    fseek(file, phdr->p_offset, SEEK_SET);
    void *buffer_ptr = (void *)malloc(phdr->p_memsz);
    fread(buffer_ptr, phdr->p_memsz, 1, file);
    fwrite(buffer_ptr, phdr->p_memsz, 1, image); 	
    free(buffer_ptr);
}

Elf32_Phdr *read_exec_file(FILE *opfile)
{
    FILE *file_elf = opfile;
    void *buffer_ptr;
    int phoffset = 0; //程序头表偏移
 
 	//读入elf头文件
	Elf32_Ehdr *Elf_header = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
	memset(Elf_header,0,sizeof(Elf32_Ehdr));
	fseek(file_elf,0,SEEK_SET);
    fread(Elf_header,sizeof(Elf32_Ehdr),1,file_elf);
 
    phoffset = Elf_header->e_phoff;

	Elf32_Phdr *Pro_header = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr));
    fseek(file_elf,phoffset,SEEK_SET);
    fread((Elf32_Phdr *)Pro_header,sizeof(Elf32_Phdr),1,file_elf);

    return Pro_header;
}

uint8_t count_kernel_sectors(Elf32_Phdr *Phdr)
{
	return  (Phdr->p_memsz - 1)/ 512 + 1;
}
//向image写入bootblock，不足一个扇区的部分用0补足
void write_bootblock(FILE *image, FILE *file, Elf32_Phdr *phdr)
{
	fseek(image, 0, SEEK_SET);
	fseek(file, 0, SEEK_SET);
	write_filesegement_image(image, file, phdr);
	int blanklength = 512 - ftell(image);
	void *blankptr = (void *)malloc(blanklength);
	memset(blankptr,0,blanklength);
    fwrite(blankptr, blanklength,1,image);
    free(blankptr);
}
//将kernel写入image
void write_kernel(FILE *image, FILE *knfile, Elf32_Phdr *Phdr, int kernelsz)
{
	fseek(image, 512, SEEK_SET);
	fseek(knfile, 0, SEEK_SET);
	write_filesegement_image(image, knfile, Phdr);
}
//在image上直接更改第一扇区的最后的值为kernel扇区数
void record_kernel_sectors(FILE *image, uint8_t kernelsz)
{
	int buffer=kernelsz;
	fseek(image,508,SEEK_SET);
	int *buffer_ptr = &buffer;
    fwrite(buffer_ptr, sizeof(int), 1, image);	  
}
//打印扩展信息，--extended
void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz)
{
	printf("bootblock memsz = %d  p_offset = %d\n", Phdr_bb->p_memsz, Phdr_bb->p_offset);
	printf("kernel memsz = %d     p_offset = %d\n", Phdr_k->p_memsz, Phdr_k->p_offset);	
	printf("kernelsz = %d\n", kernelsz);
}

int main(int argc, char *argv[])
{
	Elf32_Phdr *Phdr_bb = NULL;
    Elf32_Phdr *Phdr_k = NULL;
    char *extentstr = "--extended";
	FILE * opfile; FILE * knfile; FILE * image;
	int kernelsz;

	opfile= fopen("bootblock","r");
	knfile= fopen("main","r");
	image= fopen("image","w");

    Phdr_bb = read_exec_file(opfile);
    Phdr_k = read_exec_file(knfile);
	kernelsz = count_kernel_sectors(Phdr_k);
	//将bootblock写入image（占据第一个扇区）
	write_bootblock(image, opfile, Phdr_bb);
	//将kernel写入image
	write_kernel(image, knfile, Phdr_k, kernelsz);

	record_kernel_sectors(image, kernelsz);

	if( argc >= 2 && strcmp(argv[1],extentstr)==0) 
		extent_opt(Phdr_bb,Phdr_k,kernelsz);

	fclose(opfile);fclose(knfile);fclose(image);
}


/*在当前的image指针位置上写入elf文件的段内容，执行后image指针位置在写入内容的最后

该函数对于一般elf文件（程序头表表项可能不止一个）的情况都可以执行

void write_filesegement_image(FILE *image, FILE *file)
{

    FILE *file_elf = file;
    void *buffer_ptr;
    int phoffset = 0; //程序头表偏移
    int phnum = 0;//程序头表数目
 
 	//读入elf头文件
	Elf32_Ehdr *Elf_header = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
	memset(Elf_header,0,sizeof(Elf32_Ehdr));
	fseek(file_elf,0,SEEK_SET);
    fread(Elf_header,sizeof(Elf32_Ehdr),1,file_elf);
 
    phoffset = Elf_header->e_phoff;
    phnum = Elf_header->e_phnum;

    //将bootblock中的程序头表转存进结构数组中
	Elf32_Phdr *Pro_header = (Elf32_Phdr *)malloc(phnum * sizeof(Elf32_Phdr));
    fseek(file_elf,phoffset,SEEK_SET);
    fread((Elf32_Phdr *)Pro_header,sizeof(Elf32_Phdr),phnum,file_elf);

    //按照存储在程序头表中的各项分别找到对应段存入image
    while(phnum -- > 0){		
    	fseek(file_elf, (Pro_header[phnum]).p_offset, SEEK_SET);
    	buffer_ptr = (void *)malloc((Pro_header[phnum]).p_memsz);
    	fread(buffer_ptr, (Pro_header[phnum]).p_memsz, 1, file_elf);
    	fwrite(buffer_ptr, (Pro_header[phnum]).p_memsz, 1, image); 	
    	free(buffer_ptr);
    }
    free(Elf_header);
    free(Pro_header);
}*/