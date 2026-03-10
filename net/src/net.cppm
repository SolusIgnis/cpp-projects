// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Jeremy Murphy and any Contributors
/**
 * @file net.cppm
 * @date March 11, 2026
 *
 * @copyright © 2026 Jeremy Murphy and any Contributors
 * @par License: @parblock
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
 * limitations under the License. @endparblock
 *
 * @brief Primary module interface for the `net` metamodule.
 * @details Exports partitions for:
 *   - `net.asio_concepts` = Asio `concept`s module.
 *   - `net.telnet`        = Telnet module.
 */

//Primary module interface unit
export module net;

//Export all sub-modules
export import net.asio_concepts; ///< @see "net.asio_concepts.cppm"
export import net.telnet;        ///< @see "net.telnet.cppm"
