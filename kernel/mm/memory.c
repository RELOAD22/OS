#include "mm.h"
//TODO:Finish memory management functions here refer to mm.h and add any functions you need.
void do_TLB_Refill(int Context){
    int count;
	int k1;
	int vpn2, asid;
	int epfn, opfn;
	int coherency, Dirty, Valid, Global;
    int i;  
	int page_fault_flag = 0;
    int BadVnum = (Context / 0x10)*2;
	asid = get_C0_ENHI();
	i = asid & 0xff;
    /*寻找进程i页表中对应虚拟地址的一项*/
    for(count = 0; count < PTE_NUM; ++count){
        if((BadVnum == page[i][count].virtual_pageframe_num) && (page[i][count].valid_flag == 1))
            break;
    }

	//未找到对应的页表项
    if(count >= PTE_NUM)
		page_fault_flag = 1;

	//page fault
	if(page_fault_flag == 1){
		do_page_fault(BadVnum);
		//page_fault_flag = 1;
		return;		
	}

	//是否在磁盘上
	if(page[i][count].on_disk_flag == 1){
		//可以找到页表项，但映射在磁盘上
		do_page_replace(BadVnum);
		return;
	}

	//可以找到对应页表项，并且在物理内存上

	vpn2 = page[i][count].virtual_pageframe_num >> 1;
	asid = get_C0_ENHI();
	k1 = (vpn2<<13)|(asid & 0xff);
	set_C0_ENHI(k1);

    find_tlbp(k1);	//查找C0_ENHI在tlb中

	coherency = 2; Dirty = 1; Valid = 1; Global = 0;
	epfn = page[i][count].physical_pageframe_num;
	k1 = (epfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
	set_C0_ENLO0(k1);    

    if(count%2 == 0){
	    opfn = page[i][count + 1].physical_pageframe_num;
	    k1 = (opfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
	    set_C0_ENLO1(k1);
    }else{
	    opfn = page[i][count - 1].physical_pageframe_num;
	    k1 = (opfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
	    set_C0_ENLO1(k1);
    }

	k1 = 0; 
	set_C0_PAGEMASK(k1);


    if(get_index() < 0){    //未查找到对应的值，按序写入tlb
		k1 = tlb_unused_index;
		set_C0_INDEX(k1);	
        set_tlb();

		tlb_unused_index++;

	    vt100_move_cursor(1, 7);
		if(page_fault_flag == 0)	
	    	printk("(PTE)asid:%x BadVaddr:%x vnum: %x pnum: %x                    ", (asid & 0xff), Context,BadVnum,epfn);
		else
	    	printk("(pfault)asid:%x BadVaddr:%x vnum: %x pnum: %x                 ", (asid & 0xff), Context,BadVnum,epfn);
    }
    else{   //查找到了对应的虚拟地址，此时index设置到相应位置
        set_tlb();  
	    vt100_move_cursor(1, 7);
	    printk("(invalid)asid:%x BadVaddr:%x vnum: %x pnum: %x        ", (asid & 0xff), Context,BadVnum,epfn);
    }
	vt100_move_cursor(1, 8);
	printk("tlb_unused_index:%x physical_unused_num:%x         ", tlb_unused_index, physical_unused_num);
}

void do_page_fault(int BadVnum){
	int count;
	int asid = get_C0_ENHI();
	int pid_now = asid & 0xff;

	//选择无效页表内容（两个一组）
    for(count = 0; count < PTE_NUM; ++count){
        if(0 == page[pid_now][count].valid_flag)
            break;
    }
	//
	if(pyhsical_page_full_flag == 1){
		//映射建立在disk上
		page[pid_now][count].virtual_pageframe_num = BadVnum;
		page[pid_now][count].disk_pageframe_num = disk_unused_num;
		page[pid_now][count].valid_flag = 1;
		page[pid_now][count].on_disk_flag = 1;

		page[pid_now][count + 1].virtual_pageframe_num = BadVnum + 1;
		page[pid_now][count + 1].disk_pageframe_num = disk_unused_num + 1;
		page[pid_now][count + 1].valid_flag = 1;
		page[pid_now][count + 1].on_disk_flag = 1;

		//disk页面未使用的页面位置
		disk_unused_num += 2;

	}else{
		//映射建立在物理内存上
		//写两个页表项（奇偶对）
		page[pid_now][count].virtual_pageframe_num = BadVnum;
		page[pid_now][count].physical_pageframe_num = physical_unused_num;
		page[pid_now][count].valid_flag = 1;
		page[pid_now][count].on_disk_flag = 0;

		page[pid_now][count + 1].virtual_pageframe_num = BadVnum + 1;
		page[pid_now][count + 1].physical_pageframe_num = physical_unused_num + 1;
		page[pid_now][count + 1].valid_flag = 1;
		page[pid_now][count + 1].on_disk_flag = 0;

		//物理页面未使用的页面位置
		physical_unused_num += 2;
		if(physical_unused_num == PYHSICAL_PAGE_NUMBER)
			pyhsical_page_full_flag = 1;
	}
}

void do_page_replace(int BadVnum){
	int Acount;
	int Bcount;
	int B_pid;
	int asid = get_C0_ENHI();
	int pid_now = asid & 0xff;
	int new_free_physical_pagenum;
	int old_occupy_disk_pagenum;
    /*寻找进程i页表中对应虚拟地址的一项*/
    for(Acount = 0; Acount < PTE_NUM; ++Acount){
        if((BadVnum == page[pid_now][Acount].virtual_pageframe_num) && (page[pid_now][Acount].valid_flag == 1))
            break;
    }

    /*寻找用户进程页表中对应有效虚拟地址-物理地址的一项*/
	for(B_pid = 3; B_pid < NUM_MAX_TASK; ++B_pid){
    	for(Bcount = 0; Bcount < PTE_NUM; ++Bcount){
        	if((page[B_pid][Bcount].on_disk_flag == 0) && (page[B_pid][Bcount].valid_flag == 1))
           		break;
    	}
	}

	//装填B对应的页表项到tlb中，这样之后就能拷贝这部分内存的内容
	int vpn2 = page[B_pid][Bcount].virtual_pageframe_num >> 1;
	asid = B_pid;
	int k1 = (vpn2<<13)|(asid & 0xff);
	set_C0_ENHI(k1);

	find_tlbp(k1);	//查找C0_ENHI在tlb中

	//设成当前进程的pid
	asid = pid_now;
	k1 = (vpn2<<13)|(asid & 0xff);
	set_C0_ENHI(k1);

	int coherency = 2; int Dirty = 1; int Valid = 1; int Global = 0;
	int epfn = page[B_pid][Bcount].physical_pageframe_num;
	k1 = (epfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
	set_C0_ENLO0(k1);    
	int opfn;
    if(Acount%2 == 0){
	    opfn = page[B_pid][Bcount + 1].physical_pageframe_num;
	    k1 = (opfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
	    set_C0_ENLO1(k1);
    }else{
	    opfn = page[B_pid][Bcount - 1].physical_pageframe_num;
	    k1 = (opfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
	    set_C0_ENLO1(k1);
    }

	k1 = 0; 
	set_C0_PAGEMASK(k1);

    if(get_index() < 0){    //未查找到对应的值
		k1 = 31;
		set_C0_INDEX(k1);	
        set_tlb();
	}else{	//查找到了对应的值
        set_tlb();		
	}



	//B原来的物理页面
	new_free_physical_pagenum = page[B_pid][Bcount].physical_pageframe_num;
	//A原来的disk页面
	old_occupy_disk_pagenum = page[pid_now][Acount].disk_pageframe_num;

	//将B映射的物理页面写到新的空闲disk页面上
	int physical_address = new_free_physical_pagenum << 0x1000;
	int disk_address = disk_unused_num << 0x1000;

	//sdwrite(physical_address, disk_address, 0x2000);

	//B映射重新建立在disk上
	page[B_pid][Bcount].virtual_pageframe_num = BadVnum;
	page[B_pid][Bcount].disk_pageframe_num = disk_unused_num;
	page[B_pid][Bcount].valid_flag = 1;
	page[B_pid][Bcount].on_disk_flag = 1;

	page[B_pid][Bcount + 1].virtual_pageframe_num = BadVnum + 1;
	page[B_pid][Bcount + 1].disk_pageframe_num = disk_unused_num + 1;
	page[B_pid][Bcount + 1].valid_flag = 1;
	page[B_pid][Bcount + 1].on_disk_flag = 1;
	//disk页面未使用的页面位置
	disk_unused_num += 2;


	//A映射建立在B的物理页面上
	page[pid_now][Acount].virtual_pageframe_num = BadVnum;
	page[pid_now][Acount].physical_pageframe_num = new_free_physical_pagenum;
	page[pid_now][Acount].valid_flag = 1;
	page[pid_now][Acount].on_disk_flag = 0;

	page[pid_now][Acount + 1].virtual_pageframe_num = BadVnum + 1;
	page[pid_now][Acount + 1].physical_pageframe_num = new_free_physical_pagenum + 1;
	page[pid_now][Acount + 1].valid_flag = 1;
	page[pid_now][Acount + 1].on_disk_flag = 0;	

	//将A映射的disk页面写到新的空闲物理页面上
	physical_address = new_free_physical_pagenum << 0x1000;
	disk_address = old_occupy_disk_pagenum << 0x1000;

	//sdread(physical_address, disk_address, 0x2000);


	//覆盖B的内容
	vpn2 = page[B_pid][Bcount].virtual_pageframe_num >> 1;
	asid = pid_now;
	k1 = (vpn2<<13)|(asid & 0xff);
	set_C0_ENHI(k1);

    find_tlbp(k1);	//查找C0_ENHI在tlb中

	vpn2 = page[pid_now][Acount].virtual_pageframe_num >> 1;
	asid = pid_now;
	k1 = (vpn2<<13)|(asid & 0xff);
	set_C0_ENHI(k1);

	coherency = 2; Dirty = 1; Valid = 1; Global = 0;
	epfn = page[pid_now][Acount].physical_pageframe_num;
	k1 = (epfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
	set_C0_ENLO0(k1);    
	opfn;
    if(Acount%2 == 0){
	    opfn = page[pid_now][Acount + 1].physical_pageframe_num;
	    k1 = (opfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
	    set_C0_ENLO1(k1);
    }else{
	    opfn = page[pid_now][Acount - 1].physical_pageframe_num;
	    k1 = (opfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
	    set_C0_ENLO1(k1);
    }

	k1 = 0; 
	set_C0_PAGEMASK(k1);

    set_tlb();		
}