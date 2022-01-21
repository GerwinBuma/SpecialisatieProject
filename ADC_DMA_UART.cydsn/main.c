/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"

#define BUFFERSIZE 1024
#define SAMPLESIZE  2
 /* Defines for DMA_1 */
#define DMA_1_BYTES_PER_BURST SAMPLESIZE
#define DMA_1_REQUEST_PER_BURST 1
#define DMA_1_SRC_BASE (CYDEV_PERIPH_BASE)
#define DMA_1_DST_BASE (CYDEV_SRAM_BASE)

//#define USEUART


volatile uint8 adc_dma_array_full = 0;
volatile uint8 adc_dma_array_bussy = 1;
volatile uint8 adc_dma_array_1_full = 0;
volatile uint8 adc_dma_array_1_bussy = 0;
volatile uint8 adc_dma_array_2_full = 0;
volatile uint8 adc_dma_array_2_bussy = 0;

uint16 result;
uint16 adc_dma_array[BUFFERSIZE];
uint16 adc_dma_array_1[BUFFERSIZE];
uint16 adc_dma_array_2[BUFFERSIZE];

volatile uint8 fault = 0;

volatile uint16 counter_flip = 0;

uint8 DMA_1_Chan;
uint8 DMA_1_TD[3];

CY_ISR(DmaDone)
{
    if((adc_dma_array_full == 1) & (adc_dma_array_1_full == 1) & (adc_dma_array_2_full == 1)) {
        fault = 1;
    } else if(adc_dma_array_bussy == 1) {
        adc_dma_array_full = 1;       
        adc_dma_array_bussy = 0;
        adc_dma_array_1_bussy = 1;
    } else if(adc_dma_array_1_bussy == 1) {
        adc_dma_array_1_full = 1;
        adc_dma_array_1_bussy = 0;
        adc_dma_array_2_bussy = 1;
    } else if(adc_dma_array_2_bussy == 1) {
        adc_dma_array_2_full = 1;
        adc_dma_array_2_bussy = 0;
        adc_dma_array_bussy = 1;
    }
}

CY_ISR(Timer_ISR)
{
    counter_flip++;
}

uint8 timeout(uint8 ms) {
    uint8 timeout = 0;
    while(USBFS_1_GetEPState(1) != USBFS_1_IN_BUFFER_EMPTY) { // Wait until our IN EP is empty
                CyDelayUs(10); // wait 0.01 ms
                timeout++;
                if(timeout > ms*100) // Max wait ms before timeout
                    return 1;
            }
    return 0;
}

void sendData(uint16 *data) {
    #ifdef USEUART
        uint8 hibyte;
        uint8 lowbyte;
        uint16 i = 0;

        // Calculate amount of samples that are being send
        uint16 bytesToSend = BUFFERSIZE;
        // Send sync bytes
        UART_1_WriteTxData(0xFF);
        UART_1_WriteTxData(0xFF);
        // Send number of bytes to send
        hibyte = (bytesToSend>>8) & 0x0F;
        lowbyte = bytesToSend;
        UART_1_WriteTxData(hibyte);
        UART_1_WriteTxData(lowbyte);
        
        for(; i < BUFFERSIZE; i++) {
            hibyte = (data[i]>>8) & 0x0F;
            lowbyte = data[i];
            
            UART_1_PutChar(hibyte);
            UART_1_PutChar(lowbyte);
            
            //UART_1_WriteTxData(hibyte);
            //UART_1_WriteTxData(lowbyte);
        }
    #else
        if(timeout(1) == 1) // If in buffer not empty, skip
            return;
        // TODO in python script always ignore first packet?
        for(uint16 i=0; i<BUFFERSIZE; i+=32) {
            if(timeout(2) == 1)
                return;
            if((i+32) > BUFFERSIZE) {
                uint16 l = BUFFERSIZE-i;
                USBFS_1_LoadInEP(1, data+i, l*2);
            }
            else
                USBFS_1_LoadInEP(1, data+i, 64);
        }
    #endif
}

int main(void)
{
    Timer_1_Init();
    isr_timer_StartEx(Timer_ISR);
    
    UART_1_Start();
    
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    #ifndef USEUART
        USBFS_1_Start(0, USBFS_1_5V_OPERATION); // Start the USB peripheral
        while(!USBFS_1_GetConfiguration()); // Wait until USB is configured
    #endif
    
    
    /* DMA Configuration for DMA_1 */
    DMA_1_Chan = DMA_1_DmaInitialize(DMA_1_BYTES_PER_BURST, DMA_1_REQUEST_PER_BURST, 
        HI16(DMA_1_SRC_BASE), HI16(DMA_1_DST_BASE));
    DMA_1_TD[0] = CyDmaTdAllocate();
    DMA_1_TD[1] = CyDmaTdAllocate();
    DMA_1_TD[2] = CyDmaTdAllocate(); 
    CyDmaTdSetConfiguration(DMA_1_TD[0], BUFFERSIZE * DMA_1_BYTES_PER_BURST, DMA_1_TD[1], DMA_1__TD_TERMOUT_EN | CY_DMA_TD_INC_DST_ADR);
    CyDmaTdSetConfiguration(DMA_1_TD[1], BUFFERSIZE * DMA_1_BYTES_PER_BURST, DMA_1_TD[2], DMA_1__TD_TERMOUT_EN | CY_DMA_TD_INC_DST_ADR);
    CyDmaTdSetConfiguration(DMA_1_TD[2], BUFFERSIZE * DMA_1_BYTES_PER_BURST, DMA_1_TD[0], DMA_1__TD_TERMOUT_EN | CY_DMA_TD_INC_DST_ADR);
    CyDmaTdSetAddress(DMA_1_TD[0], LO16((uint32)ADC_SAR_1_SAR_WRK0_PTR), LO16((uint32)adc_dma_array));
    CyDmaTdSetAddress(DMA_1_TD[1], LO16((uint32)ADC_SAR_1_SAR_WRK0_PTR), LO16((uint32)adc_dma_array_1));
    CyDmaTdSetAddress(DMA_1_TD[2], LO16((uint32)ADC_SAR_1_SAR_WRK0_PTR), LO16((uint32)adc_dma_array_2));
    CyDmaChSetInitialTd(DMA_1_Chan, DMA_1_TD[0]);
    CyDmaChEnable(DMA_1_Chan, 1);

    /* Setup the Interrupt connected to the nrq terminal. */
    isr_dma_StartEx(DmaDone);
    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    ADC_SAR_1_Start(); // Function calls init function of not done yet and enable the ADC
    CyDelay(10); // Fort ADC
    ADC_SAR_1_StartConvert();
    //PWM_1_Start();
    while(1)
    {   
        if(fault == 1) {
            while(1){
                UART_1_PutString("All full\r\n");
                CyDelay(1000);
            }
        }
        if(adc_dma_array_full) {
            sendData(adc_dma_array);
            adc_dma_array_full = 0;
        }
        if(adc_dma_array_1_full) {
            sendData(adc_dma_array_1);
            adc_dma_array_1_full = 0;
        }
        if(adc_dma_array_2_full) {
            sendData(adc_dma_array_2);
            adc_dma_array_2_full = 0;
        }

        /* Place your application code here. */
        
    }
}
/* [] END OF FILE */
