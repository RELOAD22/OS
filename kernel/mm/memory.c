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
    int BadVnum = (Context / 0x10)*2;
	asid = get_C0_ENHI();
	i = asid & 0xff;
    /*寻找进程i页表中对应虚拟地址的一项*/
    for(count = 0; count < 256; ++count){
        if(BadVnum == page[i][count].virtual_pageframe_num)
            break;
    }
    if(count == 256)
    {
        vt100_move_cursor(1, 7);
	    printk("ERROR ADDR! NO INPUT VADDR IN PAGE TABLE.");

    }

	vpn2 = page[i][count].virtual_pageframe_num >> 1;
	asid = get_C0_ENHI();
	k1 = (vpn2<<13)|(asid & 0xff);
	set_C0_ENHI(k1);

    find_tlbp(k1);	//查找C0_ENHI在tlb中

	coherency = 2; Dirty = 1; Valid = 1; Global = 1;
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
	
    if(get_index() < 0){    //未查找到对应的值，随机写入tlb
        set_tlbwr();
	    vt100_move_cursor(1, 7);
	    printk("asid:%x BadVaddr:%x vnum: %x pnum: %x                 ", (asid & 0xff), Context,BadVnum,epfn);
    }
    else{   //查找到了对应的虚拟地址，此时index设置到相应位置
        set_tlb();  
	    vt100_move_cursor(1, 7);
	    printk("(invalid)asid:%x BadVaddr:%x vnum: %x pnum: %x        ", (asid & 0xff), Context,BadVnum,epfn);
    }
}