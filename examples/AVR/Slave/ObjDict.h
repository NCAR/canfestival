
/* File generated by gen_cfile.py. Should not be modified. */

#ifndef OBJDICT_H
#define OBJDICT_H

#include "data.h"

/* Prototypes of function provided by object dictionnary */
UNS32 ObjDict_valueRangeTest (UNS8 typeValue, void * value);
const indextable * ObjDict_scanIndexOD (UNS16 wIndex, UNS32 * errorCode, ODCallback_t **callbacks);

/* Master node data struct */
extern CO_Data ObjDict_Data;
extern ODCallback_t Transmit_PDO_1_Parameter_callbacks[];		/* Callbacks of index0x1800 */
extern UNS8 Read_Inputs_8_Bit[1];		/* Mapped at index 0x6000, subindex 0x01 - 0x01 */
extern UNS8 Polarity_Input_8_Bit[1];		/* Mapped at index 0x6002, subindex 0x01 - 0x01 */
extern UNS8 Filter_Constant_Input_8_Bit[1];		/* Mapped at index 0x6003, subindex 0x01 - 0x01 */
extern UNS8 Global_Interrupt_Enable_Digital;		/* Mapped at index 0x6005, subindex 0x00*/
extern UNS8 Interrupt_Mask_Any_Change_8_Bit[1];		/* Mapped at index 0x6006, subindex 0x01 - 0x01 */
extern UNS8 Interrupt_Mask_Low_to_High_8_Bit[1];		/* Mapped at index 0x6007, subindex 0x01 - 0x01 */
extern UNS8 Interrupt_Mask_High_to_Low_8_Bit[1];		/* Mapped at index 0x6008, subindex 0x01 - 0x01 */
extern UNS8 Write_Outputs_8_Bit[1];		/* Mapped at index 0x6200, subindex 0x01 - 0x01 */
extern UNS8 Change_Polarity_Outputs_8_Bit[1];		/* Mapped at index 0x6202, subindex 0x01 - 0x01 */
extern UNS8 Error_Mode_Outputs_8_Bit[1];		/* Mapped at index 0x6206, subindex 0x01 - 0x01 */
extern UNS8 Error_Value_Outputs_8_Bit[1];		/* Mapped at index 0x6207, subindex 0x01 - 0x01 */

#endif // OBJDICT_H
