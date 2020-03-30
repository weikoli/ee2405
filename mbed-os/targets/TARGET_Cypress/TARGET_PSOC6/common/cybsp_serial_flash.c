/***************************************************************************//**
* \file cybsp_serial_flash.c
*
* \brief
* Provides APIs for interacting with an external flash connected to the SPI or
* QSPI interface, uses the configuration generated by the QSPI configurator,
* uses SFDP to auto-discover memory properties if SFDP is enabled in the
* configuration.
*
********************************************************************************
* \copyright
* Copyright 2018-2019 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include <stdbool.h>
#include "cybsp_serial_flash.h"
#include "cy_pdl.h"
#include "cyhal_qspi.h"
#include "cy_utils.h"
#include "cybsp.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(CYBSP_QSPI_SCK)
#include "cycfg_qspi_memslot.h"

/** \cond internal */

#define QSPI_BUS_FREQUENCY_HZ                          (50000000lu)

/** Timeout to apply while polling the memory for its ready status after quad
 * enable command has been sent out. Quad enable is a non-volatile write.
*/
#define CYBSP_SERIAL_FLASH_QUAD_ENABLE_TIMEOUT_US      (5000lu) /* in microseconds */

/* SMIF slot number to which the memory is connected */
#define MEM_SLOT                                        (0u)

/** \endcond */

static cyhal_qspi_t qspi_obj;

cy_rslt_t cybsp_serial_flash_init(void)
{
    cy_en_smif_status_t smifStatus = CY_SMIF_SUCCESS;

    cy_rslt_t result = cyhal_qspi_init(&qspi_obj, CYBSP_QSPI_D0, CYBSP_QSPI_D1, CYBSP_QSPI_D2, CYBSP_QSPI_D3, NC, NC, NC, NC,
                                        CYBSP_QSPI_SCK, CYBSP_QSPI_SS, QSPI_BUS_FREQUENCY_HZ, 0);
    if(CY_RSLT_SUCCESS == result)
    {
        /* Perform SFDP detection and XIP register configuration depending on the
         * memory configuration.
         */
        smifStatus = Cy_SMIF_Memslot_Init(qspi_obj.base, (cy_stc_smif_block_config_t *) &smifBlockConfig, &qspi_obj.context);
        if(CY_SMIF_SUCCESS == smifStatus)
        {
            /* Enable Quad mode (1-1-4 or 1-4-4 modes) to use all the four I/Os during
            * communication.
            */
            if(smifMemConfigs[MEM_SLOT]->deviceCfg->readCmd->dataWidth == CY_SMIF_WIDTH_QUAD
                    || smifMemConfigs[MEM_SLOT]->deviceCfg->programCmd->dataWidth == CY_SMIF_WIDTH_QUAD)
            {
                bool isQuadEnabled = false;
                smifStatus = Cy_SMIF_MemIsQuadEnabled(qspi_obj.base, smifMemConfigs[MEM_SLOT], &isQuadEnabled, &qspi_obj.context);
                if(CY_SMIF_SUCCESS == smifStatus)
                {
                    if(!isQuadEnabled)
                    {
                        smifStatus = Cy_SMIF_MemEnableQuadMode(qspi_obj.base, smifMemConfigs[MEM_SLOT], CYBSP_SERIAL_FLASH_QUAD_ENABLE_TIMEOUT_US, &qspi_obj.context);
                    }
                }
            }
        }
    }

    if((CY_RSLT_SUCCESS == result) && (CY_SMIF_SUCCESS == smifStatus))
    {
        return CY_RSLT_SUCCESS;
    }
    else
    {
        cybsp_serial_flash_deinit();
        return (cy_rslt_t)smifStatus;
    }
}

void cybsp_serial_flash_deinit(void)
{
    cyhal_qspi_free(&qspi_obj);
}

size_t cybsp_serial_flash_get_size(void)
{
    return (size_t)smifMemConfigs[MEM_SLOT]->deviceCfg->memSize;
}

/* address is ignored for the memory with uniform sector size. Currently,
 * QSPI Configurator does not support memories with hybrid sectors.
 */
size_t cybsp_serial_flash_get_erase_size(uint32_t addr)
{
    CY_UNUSED_PARAMETER(addr);
    return (size_t)smifMemConfigs[MEM_SLOT]->deviceCfg->eraseSize;
}

cy_rslt_t cybsp_serial_flash_read(uint32_t addr, size_t length, uint8_t *buf)
{
    /* Cy_SMIF_MemRead() returns error if (addr + length) > total flash size. */
    return (cy_rslt_t)Cy_SMIF_MemRead(qspi_obj.base, smifMemConfigs[MEM_SLOT], addr, buf, length, &qspi_obj.context);
}

cy_rslt_t cybsp_serial_flash_write(uint32_t addr, size_t length, const uint8_t *buf)
{
    /* Cy_SMIF_MemWrite() returns error if (addr + length) > total flash size. */
    return (cy_rslt_t)Cy_SMIF_MemWrite(qspi_obj.base, smifMemConfigs[MEM_SLOT], addr, (uint8_t *)buf, length, &qspi_obj.context);
}

/* Does not support hybrid sectors, sector size must be uniform on the entire
 * chip. Use cybsp_serial_flash_get_erase_size(addr) to implement hybrid sector
 * support when QSPI Configurator and PDL supports memories with hybrid sectors.
 */
cy_rslt_t cybsp_serial_flash_erase(uint32_t addr, size_t length)
{
    cy_en_smif_status_t smifStatus;

    /* If the erase is for the entire chip, use chip erase command */
    if((addr == 0u) && (length == cybsp_serial_flash_get_size()))
    {
        smifStatus = Cy_SMIF_MemEraseChip(qspi_obj.base, smifMemConfigs[MEM_SLOT], &qspi_obj.context);
    }
    else
    {
        /* Cy_SMIF_MemEraseSector() returns error if (addr + length) > total flash size
         * or if addr is not aligned to erase sector size or if (addr + length)
         * is not aligned to erase sector size.
         */
        smifStatus = Cy_SMIF_MemEraseSector(qspi_obj.base, smifMemConfigs[MEM_SLOT], addr, length, &qspi_obj.context);
    }

    return (cy_rslt_t)smifStatus;
}

// This function enables or disables XIP on the MCU, does not send any command
// to the serial flash. XIP register configuration is already done as part of
// cybsp_serial_flash_init() if MMIO mode is enabled in the QSPI
// Configurator.
cy_rslt_t cybsp_serial_flash_enable_xip(bool enable)
{
    if(enable)
    {
        Cy_SMIF_SetMode(qspi_obj.base, CY_SMIF_MEMORY);
    }
    else
    {
        Cy_SMIF_SetMode(qspi_obj.base, CY_SMIF_NORMAL);
    }

    return CY_RSLT_SUCCESS;
}

#endif /* defined(CYBSP_QSPI_SCK) */

#if defined(__cplusplus)
}
#endif
