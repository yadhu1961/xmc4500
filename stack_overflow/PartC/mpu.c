#include<stdint.h>
#include <xmc_common.h>
enum MPUeasyPermissions
{
    MPUeasy_None_None = 0, MPUeasy_RW_None = 1, \
    
    MPUeasy_RW_R = 2, MPUeasy_RW_RW = 3, \
    
    MPUeasy_R_None = 5, MPUeasy_R_R = 6};

#define MPUeasyENABLEREGION  (0x1 << 7)

typedef  struct
{
    
    void *baseAddress;
    /* will be  aligned  according  to size */
    
    int permissions;
    /* bit 7 enables  region ,other  options  auto  set  according  to  manual
     bits 2-0  according  to  table  2-18:
     
     value   privileged   unprivileged
     
     0     none     none
     
     1     rw       none
     
     2     rw       r
     
     3     rw       rw
     
     5     r      none
     
     6     r      r                */
    
    uint8_t  size;
    /* as  power of 2, so e.g. 10 = 1 KiB , 20 = 1 MiB ,
     
     * NOTE: Differs  from  SIZE  field  in RASR by 1 */
    
    uint8_t  priority;
    /* exclusive , i.e. only  one  region  per  priority  */
    
    uint8_t Enable_XN;
    
} MPUconfig_t;



void enableMPU(int enableBackgroundRegion)
{
    
    __DSB ();
    
    __ISB ();
    
    PPB ->MPU_CTRL  |= (enableBackgroundRegion ? 0x4 : 0x0) | 0x1;
    
    __DSB ();
    
    __ISB ();
}


void disableMPU(void)
{
    __DSB ();
    
    __ISB ();
    
    PPB ->MPU_CTRL = 0;
    
    __DSB ();
    
    __ISB ();
}

void configMPU(MPUconfig_t  config) {
    
    /* to  align  baseAddress , we  shift  right  and  then  left  again ,
     * with at least  the  position  of the  ADDR  bitfield
     * so other  bitfields  don’t get  changed  */
    
    uint8_t  addrShift = config.size > PPB_MPU_RBAR_ADDR_Pos ? \
    config.size : PPB_MPU_RBAR_ADDR_Pos;
    
    /* TEX , S, C, and B field  will be set  according  to  recommendation
     
     * in table  2-17 of  manual  */
    
    uint8_t  autoSet = config.baseAddress  < (void *)0x10000000 ? 0x2 : \
    config.baseAddress  < (void *)0x40000000 ? 0x6 : \
    config.baseAddress  < (void *)0x60000000 ? 0x5 : \
    0x7;
    
    /*  switch  to  correct  priority  slot */
    
    PPB ->MPU_RNR = config.priority & 0x7;
    
    /*  disable  region  before  changing  parameters  to  avoid  glitches  */
    
    PPB ->MPU_RASR  &= ~PPB_MPU_RASR_ENABLE_Msk;
    
    PPB ->MPU_RBAR = (( uint32_t) config.baseAddress  >> addrShift) \
    << addrShift;
    
    PPB ->MPU_RASR = (config.permissions  << PPB_MPU_RASR_AP_Pos \
                      & PPB_MPU_RASR_AP_Msk) | \
    
    (autoSet               << PPB_MPU_RASR_B_Pos) | \
    
    (( config.size > 0 ? config.size - 1 : config.size) \
     
					<< PPB_MPU_RASR_SIZE_Pos \
					& PPB_MPU_RASR_SIZE_Msk) | \
    
    (config.permissions  >> 7 \
     & PPB_MPU_RASR_ENABLE_Msk);
    
    if(config.Enable_XN)
    { 
        PPB ->MPU_RASR  |= 0x01<<28;
    }
}


void setupMPU(void)
{


MPUconfig_t PSRAM_both_RW = {.baseAddress = (void *) 0x10000000, .size = 16, .priority = 1, .permissions = MPUeasyENABLEREGION | MPUeasy_RW_RW,.Enable_XN = 0};
MPUconfig_t FLASH_both_RW = {.baseAddress = (void *) 0x80000000, .size = 26, .priority = 0, .permissions = MPUeasyENABLEREGION | MPUeasy_RW_RW,.Enable_XN = 0};
MPUconfig_t DSRAM_both_RW = {.baseAddress = (void *) 0x20000000, .size = 29, .priority = 2, .permissions = MPUeasyENABLEREGION | MPUeasy_RW_RW,.Enable_XN = 0};
MPUconfig_t STACK_both_RW = {.baseAddress = (void *) 0x10000000, .size = 11, .priority = 3, .permissions = MPUeasyENABLEREGION | MPUeasy_RW_RW,.Enable_XN = 1};
MPUconfig_t Peripherals_both_RW = {.baseAddress = (void *) 0x40000000, .size = 16, .priority = 4, .permissions = MPUeasyENABLEREGION | MPUeasy_RW_RW,.Enable_XN =0};
configMPU(PSRAM_both_RW); 
configMPU(FLASH_both_RW);
configMPU(DSRAM_both_RW);
configMPU(Peripherals_both_RW); 
configMPU(STACK_both_RW); 
enableMPU(1);
}
