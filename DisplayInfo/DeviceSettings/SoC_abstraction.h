/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>

/** @brief returns the total GPU memory available in
 *         the platform in bytes
 *  @param void
 *  @return memory in bytes.
 */
uint64_t SoC_GetTotalGpuRam();

/** @brief returns the current free GPU memory
 *  @param void
 *  @return memory in bytes.
 */
uint64_t SoC_GetFreeGpuRam();

/** @brief returns the width of the graphics plane in pixels
 *  @param void
 *  @return pixel width of graphics plane.
 */
uint32_t SoC_GetGraphicsWidth();

/** @brief returns the height of the graphics plane in pixels
 *  @param void
 *  @return pixel height of graphics plane.
 */
uint32_t SoC_GetGraphicsHeight();