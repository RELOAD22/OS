#include "mac.h"
#include "irq.h"
#include "type.h"
#include "screen.h"
#include "syscall.h"
#include "sched.h"
#include "test5.h"
#define  SEND_DESC_BASE 0xa0f10000
#define  RECV_DESC_BASE 0xa0f80000
#define  SEND_DESC_BASE_PHY 0x0f10000
#define  RECV_DESC_BASE_PHY 0x0f80000
#define  DESC_SIZE 16

#define  SEND_BUFFER_BASE 0xa1f00000
#define  RECV_BUFFER_BASE 0xa1f80000
#define  SEND_BUFFER_BASE_PHY 0x1f00000
#define  RECV_BUFFER_BASE_PHY 0x1f80000
#define  BUFFER_SIZE 1024

queue_t recv_block_queue;
desc_t *send_desc;
desc_t *receive_desc;
uint32_t cnt = 1; //record the time of iqr_mac
//uint32_t buffer[PSIZE] = {0x00040045, 0x00000100, 0x5d911120, 0x0101a8c0, 0xfb0000e0, 0xe914e914, 0x00000801,0x45000400, 0x00010000, 0x2011915d, 0xc0a80101, 0xe00000fb, 0x14e914e9, 0x01080000};
uint32_t buffer[PSIZE] = {0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000, 0x005e0001, 0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};

/**
 * Clears all the pending interrupts.
 * If the Dma status register is read then all the interrupts gets cleared
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void clear_interrupt()
{
    uint32_t data;
    data = reg_read_32(0xbfe11000 + DmaStatus);
    reg_write_32(0xbfe11000 + DmaStatus, data);
}

static void send_desc_init(mac_t *mac)
{
    int count;
    desc_t *desc_ptr;
    uint32_t DESC_LOC_TEMP = SEND_DESC_BASE;
    uint32_t DESC_LOC_TEMP_PHY = SEND_DESC_BASE_PHY;
    uint32_t BUFFER_LOC_TEMP = SEND_BUFFER_BASE;
    uint32_t BUFFER_LOC_TEMP_PHY = SEND_BUFFER_BASE_PHY;
    int list_len = 256;
    uint32_t *w_ptr; int count_p;
    for(count = 0; count < list_len - 1; count ++){

        desc_ptr = DESC_LOC_TEMP;

        desc_ptr->tdes0 = 0x80000000;
        desc_ptr->tdes1 = 0x81000200;
        desc_ptr->tdes2 = BUFFER_LOC_TEMP_PHY;
        desc_ptr->tdes3 = DESC_LOC_TEMP_PHY + DESC_SIZE;

        w_ptr = BUFFER_LOC_TEMP;
        for(count_p = 0; count_p < PSIZE; ++count_p){
            *w_ptr = buffer[count_p];
            ++w_ptr;
        }

        DESC_LOC_TEMP += DESC_SIZE;
        DESC_LOC_TEMP_PHY += DESC_SIZE;
        BUFFER_LOC_TEMP += BUFFER_SIZE;
        BUFFER_LOC_TEMP_PHY += BUFFER_SIZE;
    }
    desc_ptr = DESC_LOC_TEMP;
    desc_ptr->tdes0 = 0x80000000;
    desc_ptr->tdes1 = 0x83000200;
    desc_ptr->tdes2 = BUFFER_LOC_TEMP_PHY;
    desc_ptr->tdes3 = SEND_DESC_BASE_PHY;

    w_ptr = BUFFER_LOC_TEMP;
    for(count_p = 0; count_p < PSIZE; ++count_p){
        *w_ptr = buffer[count_p];
        ++w_ptr;
    }

    mac->saddr = SEND_BUFFER_BASE_PHY + 0xa0000000;
    mac->saddr_phy = SEND_BUFFER_BASE_PHY;

    mac->td = SEND_DESC_BASE;
    mac->td_phy = SEND_DESC_BASE_PHY;
}

static void recv_desc_init(mac_t *mac)
{
    int count;
    desc_t *desc_ptr;
    uint32_t DESC_LOC_TEMP = RECV_DESC_BASE;
    uint32_t DESC_LOC_TEMP_PHY = RECV_DESC_BASE_PHY;
    uint32_t BUFFER_LOC_TEMP = RECV_BUFFER_BASE;
    uint32_t BUFFER_LOC_TEMP_PHY = RECV_BUFFER_BASE_PHY;
    int list_len = 256;
    uint32_t *w_ptr; int count_p;
    for(count = 0; count < list_len - 1; count ++){

        desc_ptr = DESC_LOC_TEMP;

        desc_ptr->tdes0 = 0x80000000;
        desc_ptr->tdes1 = 0x81000200;
        desc_ptr->tdes2 = BUFFER_LOC_TEMP_PHY;
        desc_ptr->tdes3 = DESC_LOC_TEMP_PHY + DESC_SIZE;

        w_ptr = BUFFER_LOC_TEMP;
        for(count_p = 0; count_p < PSIZE; ++count_p){
            *w_ptr = 0;
            ++w_ptr;
        }

        DESC_LOC_TEMP += DESC_SIZE;
        DESC_LOC_TEMP_PHY += DESC_SIZE;
        BUFFER_LOC_TEMP += BUFFER_SIZE;
        BUFFER_LOC_TEMP_PHY += BUFFER_SIZE;
    }
    desc_ptr = DESC_LOC_TEMP;
    desc_ptr->tdes0 = 0x80000000;
    desc_ptr->tdes1 = 0x83000200;
    desc_ptr->tdes2 = BUFFER_LOC_TEMP_PHY;
    desc_ptr->tdes3 = RECV_DESC_BASE_PHY;

    w_ptr = BUFFER_LOC_TEMP;
    for(count_p = 0; count_p < PSIZE; ++count_p){
        *w_ptr = 0;
        ++w_ptr;
    }

    mac->daddr = RECV_BUFFER_BASE_PHY + 0xa0000000;
    mac->daddr_phy = RECV_BUFFER_BASE_PHY;

    mac->rd = RECV_DESC_BASE;
    mac->rd_phy = RECV_DESC_BASE_PHY; 

}



static void mii_dul_force(mac_t *mac)
{
    reg_write_32(mac->dma_addr, 0x80); //?s
                                       //   reg_write_32(mac->dma_addr, 0x400);
    uint32_t conf = 0xc800;            //0x0080cc00;

    // loopback, 100M
    reg_write_32(mac->mac_addr, reg_read_32(mac->mac_addr) | (conf) | (1 << 8));
    //enable recieve all
    reg_write_32(mac->mac_addr + 0x4, reg_read_32(mac->mac_addr + 0x4) | 0x80000001);
}

static void start_tran(mac_t *mac)
{

}

void dma_control_init(mac_t *mac, uint32_t init_value)
{
    reg_write_32(mac->dma_addr + DmaControl, init_value);
    return;
}

void phy_regs_task1()
{

    mac_t test_mac;
    uint32_t i;
    uint32_t print_location = 2;

    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 64bytes
    test_mac.pnum = PNUM;       // pnum

    send_desc_init(&test_mac);

    dma_control_init(&test_mac, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);
    clear_interrupt(&test_mac);

    mii_dul_force(&test_mac);

    // register_irq_handler(LS1C_MAC_IRQ, irq_mac);

    // irq_enable(LS1C_MAC_IRQ);
    sys_move_cursor(1, print_location);
    printf("> [SEND TASK] start send package.               \n");

    uint32_t cnt = 0;
    i = 4;
    while (i > 0)
    {
        sys_net_send(test_mac.td, test_mac.td_phy);
        cnt += PNUM;
        sys_move_cursor(1, print_location);
        printf("> [SEND TASK] totally send package %d !        \n", cnt);
        i--;
    }
    sys_exit();
}

void phy_regs_task2()
{

    mac_t test_mac;
    uint32_t i;
    uint32_t ret;
    uint32_t print_location = 1;

    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 64bytes
    test_mac.pnum = PNUM;       // pnum
    recv_desc_init(&test_mac);

    dma_control_init(&test_mac, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);
    clear_interrupt(&test_mac);

    mii_dul_force(&test_mac);

    queue_init(&recv_block_queue);
    sys_move_cursor(1, print_location);
    printf("[RECV TASK] start recv:                    ");
    ret = sys_net_recv(test_mac.rd, test_mac.rd_phy, test_mac.daddr);
    if (ret == 0)
    {
        sys_move_cursor(1, print_location);
        printf("[RECV TASK]     net recv is ok!                          ");
    }
    else
    {
        sys_move_cursor(1, print_location);
        printf("[RECV TASK]     net recv is fault!                       ");
    }

    //ch_flag = 0;
    /*
    for (i = 0; i < PNUM; i++)
    {
        recv_flag[i] = 0;
    }*/

    uint32_t cnt = 0;
    uint32_t *Recv_desc;
    Recv_desc = (uint32_t *)(test_mac.rd + (PNUM - 1) * 16);
    //printf("(test_mac.rd 0x%x ,Recv_desc=0x%x,REDS0 0X%x\n", test_mac.rd, Recv_desc, *(Recv_desc));
    if (((*Recv_desc) & 0x80000000) == 0x80000000)
    {
        sys_move_cursor(1, print_location);
        printf("> [RECV TASK] waiting receive package.\n");
        sys_wait_recv_package();
    }
    check_recv(&test_mac);

    sys_exit();
}

void phy_regs_task3()
{
    uint32_t print_location = 1;
    sys_move_cursor(1, print_location);
    printf("> [INIT] Waiting for MAC initialization .\n");
    sys_init_mac();
    sys_move_cursor(1, print_location);
    printf("> [INIT] MAC initialization succeeded.           \n");
    sys_exit();
}
