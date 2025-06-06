/*
 * Copyright (c) 2020 Huawei Technologies Co.,Ltd.
 *
 * openGauss is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * ---------------------------------------------------------------------------------------
 *
 * nvm.h
 *
 *
 * IDENTIFICATION
 *        src/include/storage/nvm/nvm.h
 *
 * ---------------------------------------------------------------------------------------
 */
#ifndef NVM_H
#define NVM_H

#include "storage/smgr/smgr.h"

void nvm_init(void);

BufferDesc *NvmBufferAlloc(const RelFileNode& rel_file_node, char relpersistence, ForkNumber fork_num,
    BlockNumber block_num, BufferAccessStrategy strategy, bool *found, const XLogPhyBlock *pblk);

#endif
