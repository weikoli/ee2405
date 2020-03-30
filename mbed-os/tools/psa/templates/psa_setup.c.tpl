/* Copyright (c) 2017-2019 ARM Limited
 *
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
 */

/*******************************************************************************
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * THIS FILE IS AN AUTO-GENERATED FILE - DO NOT MODIFY IT.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Template Version 1.0
 * Generated by tools/psa/generate_partition_code.py Version {{script_ver}}
 ******************************************************************************/

#include "cmsis.h"
#include "rtx_os.h"

#include "mbed_toolchain.h" /* For using MBED_ALIGN macro */

#include "spm_panic.h"
#include "spm_internal.h"
#include "handles_manager.h"
#include "mbed_spm_partitions.h"

#include "psa_manifest/sid.h"

extern spm_db_t g_spm;

{% macro do_parition(partition) -%}

/* -----------------------------------------------------------------------------
 * {{partition.name|upper}} declarations
 * -------------------------------------------------------------------------- */
MBED_ALIGN(8) static uint8_t {{partition.name|lower}}_thread_stack[{{partition.stack_size}}] = {0};

static osRtxThread_t {{partition.name|lower}}_thread_cb = {0};
static const osThreadAttr_t {{partition.name|lower}}_thread_attr = {
    .name = "{{partition.name|lower}}",
    .attr_bits = 0,
    .cb_mem = &{{partition.name|lower}}_thread_cb,
    .cb_size = sizeof({{partition.name|lower}}_thread_cb),
    .stack_mem = {{partition.name|lower}}_thread_stack,
    .stack_size = {{partition.stack_size}},
    .priority = {{partition.priority_mbed}},
    .tz_module = 0,
    .reserved = 0
};

static osRtxMutex_t {{partition.name|lower}}_mutex = {0};
static const osMutexAttr_t {{partition.name|lower}}_mutex_attr = {
    .name = "{{partition.name|lower}}_mutex",
    .attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust,
    .cb_mem = &{{partition.name|lower}}_mutex,
    .cb_size = sizeof({{partition.name|lower}}_mutex),
};

{% if partition.rot_services|count > 0 %}
spm_rot_service_t {{partition.name|lower}}_rot_services[] = {
{% for rot_srv in partition.rot_services %}
    {
        .sid = {{rot_srv.name|upper}},
        .mask = {{rot_srv.signal|upper}},
        .partition = NULL,
        .min_version = {{rot_srv.minor_version}},
        .min_version_policy = PSA_MINOR_VERSION_POLICY_{{rot_srv.minor_policy|upper}},
{% if rot_srv.nspe_callable %}
        .allow_nspe = true,
{% else %}
        .allow_nspe = false,
{% endif %}
        .queue = {
            .head = NULL,
            .tail = NULL
        }
    },
{% endfor %}
};

{% endif %}
{% if partition.extern_sids|count > 0 %}
/* External SIDs used by {{partition.name}} */
const uint32_t {{partition.name|lower}}_external_sids[{{partition.extern_sids|count}}] = {
{% for sid in partition.extern_sids %}
    {{sid|upper}},
{% endfor %}
};

{% endif %}
{% if partition.irqs|count > 0 %}
// Mapper function from irq signal to interupts number
IRQn_Type spm_{{partition.name|lower}}_signal_to_irq_mapper(uint32_t signal)
{
    SPM_ASSERT({{partition.name|upper}}_WAIT_ANY_IRQ_MSK & signal);
    switch(signal){
    {% for irq in partition.irqs %}
        case {{ irq.signal }}:
            return (IRQn_Type){{irq.line_num}};
            break;
    {% endfor %}
        default:
            break;
    }

    SPM_PANIC("Unknown signal number %lu", signal);
    return 0;
}

{% for irq in partition.irqs %}
// ISR handler for interrupt {irq.line_num}
void spm_irq_{{irq.signal}}_{{partition.name|lower}}(void)
{
    spm_partition_t *partition = NULL;
    for (uint32_t i = 0; i < g_spm.partition_count; ++i) {
        if (g_spm.partitions[i].partition_id == {{partition.name|upper}}_ID) {
            partition = &(g_spm.partitions[i]);
        }
    }
    SPM_ASSERT(partition);

    NVIC_DisableIRQ((IRQn_Type){{irq.line_num}});  // will be enabled by psa_eoi()
    osThreadFlagsSet(partition->thread_id, {{irq.signal|upper}});  // notify partition
}

{% endfor %}
{% endif %}
extern void {{partition.entry_point}}(void *ptr);

void {{partition.name|lower}}_init(spm_partition_t *partition)
{
    if (NULL == partition) {
        SPM_PANIC("partition is NULL!\n");
    }

    partition->mutex = osMutexNew(&{{partition.name|lower}}_mutex_attr);
    if (NULL == partition->mutex) {
        SPM_PANIC("Failed to create mutex for secure partition {{partition.name|lower}}!\n");
    }

    {% if partition.rot_services|count > 0 %}
    for (uint32_t i = 0; i < {{partition.name|upper}}_ROT_SRV_COUNT; ++i) {
        {{partition.name|lower}}_rot_services[i].partition = partition;
    }
    partition->rot_services = {{partition.name|lower}}_rot_services;
    {% else %}
    partition->rot_services = NULL;
    {% endif %}

    partition->thread_id = osThreadNew({{partition.entry_point}}, NULL, &{{partition.name|lower}}_thread_attr);
    if (NULL == partition->thread_id) {
        SPM_PANIC("Failed to create start main thread of partition {{partition.name|lower}}!\n");
    }
}

{%- endmacro %}
{# -------------- macro do_parition(partition) -----------------------------  #}
/****************** Service Partitions ****************************************/

{% for partition in service_partitions %}
{{do_parition(partition)}}

{% endfor %}

/****************** Test Partitions *******************************************/
#ifdef USE_PSA_TEST_PARTITIONS

{% for test_partition in test_partitions %}
#ifdef USE_{{test_partition.name|upper}}
{{ do_parition(test_partition) }}

#endif  // USE_{{test_partition.name|upper}}

{% endfor %}
#endif  // USE_PSA_TEST_PARTITIONS

{# -------------- spm_db_entry(partition) ----------------------------------- #}
{% macro spm_db_entry(partition) -%}

    /* {{partition.name|upper}} */
    {
        .partition_id = {{partition.name|upper}}_ID,
        .thread_id = 0,
        .flags = {{partition.name|upper}}_WAIT_ANY_SID_MSK | {{partition.name|upper}}_WAIT_ANY_IRQ_MSK,
        .rot_services = NULL,
    {% if partition.rot_services|count > 0 %}
        .rot_services_count = {{partition.name|upper}}_ROT_SRV_COUNT,
    {% else %}
        .rot_services_count = 0,
    {% endif %}
    {% if partition.extern_sids|count > 0 %}
        .extern_sids = {{partition.name|lower}}_external_sids,
    {% else %}
        .extern_sids = NULL,
    {% endif %}
        .extern_sids_count = {{partition.name|upper}}_EXT_ROT_SRV_COUNT,
    {% if partition.irqs|count > 0 %}
        .irq_mapper = spm_{{partition.name|lower}}_signal_to_irq_mapper,
    {% else %}
        .irq_mapper = NULL,
    {% endif %}
    },
{%- endmacro %}
{# -------------- spm_db_entry(partition) ----------------------------------- #}
/****************** SPM DB initialization *************************************/
spm_partition_t g_partitions[] = {
{% for partition in service_partitions %}
{{spm_db_entry(partition)}}

{% endfor %}
#ifdef USE_PSA_TEST_PARTITIONS

{% for test_partition in test_partitions %}
#ifdef USE_{{test_partition.name|upper}} {{ spm_db_entry(test_partition) }}
#endif  // USE_{{test_partition.name|upper}}

{% endfor %}
#endif  // USE_PSA_TEST_PARTITIONS

};

/****************** MMIO regions **********************************************/
{% if regions|count > 0 %}
/****************** Sanity checks *********************************************/
/* Check all the defined memory regions for overlapping. */
{% for region_pair in region_pair_list %}
MBED_STATIC_ASSERT(
    ((uintptr_t)({{region_pair[0].base}}) + {{region_pair[0].size}} - 1 < (uintptr_t)({{region_pair[1].base}})) ||
    ((uintptr_t)({{region_pair[1].base}}) + {{region_pair[1].size}} - 1 < (uintptr_t)({{region_pair[0].base}})),
    "The region with base {{region_pair[0].base}} and size {{region_pair[0].size}} overlaps with the region with base {{region_pair[1].base}} and size {{region_pair[1].size}}!");

{% endfor %}
/****************** MMIO regions definition ***********************************/
/* A list of all the memory regions. */
const mem_region_t mem_regions[] = {
{% for region in regions %}
    { (uint32_t)({{region.base}}), {{region.size}}, {{region.permission}}, {{region.partition_id}} },
{% endfor %}
};
{% else %}
const mem_region_t *mem_regions = NULL;
{% endif %}
const uint32_t mem_region_count = {{regions|count}};

/****************** Partitions init function  *********************************/
uint32_t init_partitions(spm_partition_t **partitions)
{
    uint32_t partition_idx = 0;

    if (NULL == partitions) {
        SPM_PANIC("partitions is NULL!\n");
    }

{% for partition in service_partitions %}
    {{partition.name|lower}}_init(&(g_partitions[partition_idx++]));
{% endfor %}

#ifdef USE_PSA_TEST_PARTITIONS

{% for test_partition in test_partitions %}
#ifdef USE_{{test_partition.name|upper}}
    {{test_partition.name|lower}}_init(&(g_partitions[partition_idx++]));
#endif  // USE_{{test_partition.name|upper}}

{% endfor %}
#endif  // USE_PSA_TEST_PARTITIONS

    *partitions = g_partitions;
    return partition_idx;
}

{# End of file #}
