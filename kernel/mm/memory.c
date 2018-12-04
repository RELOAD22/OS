#include "mm.h"
//TODO:Finish memory management functions here refer to mm.h and add any functions you need.
void do_TLB_Refill(int Context){
    int count;
	int k1;
	int vpn2;
	int asid;
	int epfn;
	int coherency;
	int opfn;
	int Dirty;
	int Valid;
	int Global;
	int index_of_some_entry;  
    int i;  
	int page_fault_flag = 0;
    int BadVnum = (Context / 0x10)*2;
	asid = get_C0_ENHI();
	i = asid & 0xff;
    /*寻找进程i页表中对应虚拟地址的一项*/
    for(count = 0; count < 256; ++count){
        if((BadVnum == page[i][count].virtual_pageframe_num) && (page[i][count].valid_flag == 1))
            break;
    }

	//未找到对应的页表项
    if(count >= 256)
    {
		do_page_fault(BadVnum);
		page_fault_flag = 1;
		for(count = 0; count < 256; ++count){
        	if((BadVnum == page[i][count].virtual_pageframe_num) && (page[i][count].valid_flag == 1))
            break;
    	}
    }

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
    for(count = 0; count < 256; ++count){
        if(0 == page[pid_now][count].valid_flag)
            break;
    }
	//写两个页表项（奇偶对）
	page[pid_now][count].virtual_pageframe_num = BadVnum;
	page[pid_now][count].physical_pageframe_num = physical_unused_num;
	page[pid_now][count].valid_flag = 1;

	page[pid_now][count + 1].virtual_pageframe_num = BadVnum + 1;
	page[pid_now][count + 1].physical_pageframe_num = physical_unused_num + 1;
	page[pid_now][count + 1].valid_flag = 1;

	//物理页面未使用的页面位置
	physical_unused_num += 2;
}