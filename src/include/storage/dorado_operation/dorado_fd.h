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
 * dorado_fd.h
 *
 *
 *
 * IDENTIFICATION
 *        src/include/storage/dorado_operation/dorado_fd.h
 *
 * ---------------------------------------------------------------------------------------
 */

#ifndef DORADO_FD_H
#define DORADO_FD_H

#include "postgres.h"
#include "knl/knl_variable.h"

const uint32 MEMORY_ALIGNED_SIZE = 8192;
const uint32 DORADO_CTL_WRITE_SIZE = 512;

void InitDoradoStorage(char *filePath, uint64 fileSize);

#endif
