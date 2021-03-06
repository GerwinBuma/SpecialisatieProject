/*******************************************************************************
* File Name: USB_1_Dp.h  
* Version 2.20
*
* Description:
*  This file contains the Alias definitions for Per-Pin APIs in cypins.h. 
*  Information on using these APIs can be found in the System Reference Guide.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_USB_1_Dp_ALIASES_H) /* Pins USB_1_Dp_ALIASES_H */
#define CY_PINS_USB_1_Dp_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"


/***************************************
*              Constants        
***************************************/
#define USB_1_Dp_0			(USB_1_Dp__0__PC)
#define USB_1_Dp_0_INTR	((uint16)((uint16)0x0001u << USB_1_Dp__0__SHIFT))

#define USB_1_Dp_INTR_ALL	 ((uint16)(USB_1_Dp_0_INTR))

#endif /* End Pins USB_1_Dp_ALIASES_H */


/* [] END OF FILE */
