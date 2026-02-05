
# cpp-projects
C++ Libraries

---

## License

> *This section is meant to be helpful, not legal advice. It reflects how this project understands and applies the Apache-2.0 license. If you need actual legal advice, a real lawyer will do a much better job than a README.*

This project is licensed under the **Apache License, Version 2.0**.

>**TL;DR:** *You can use this library pretty much anywhere (including commercial and closed-source projects). You're welcome to give your changes back but not required. Everyone gets clear rights and protections. Just keep the license and attribution intact.*

### Why Apache 2.0?

These libraries are intended to be used, embedded, and redistributed as part of larger systems. The Apache-2.0 license was chosen to make that easy and predictable, while clearly spelling out everyone’s rights and responsibilities.

In practical terms, Apache-2.0:

- **Allows broad use**, including commercial and proprietary software
- **Provides an explicit patent license**, so we aren’t all relying on implied assumptions
- **Includes patent retaliation protections**, reducing downstream risk
- **Works well for libraries**, where reuse is expected
- **Scales well** from a solo project to one with many contributors
- **Avoids ambiguity** by clearly specifying what is permitted and required

The goal is to make the legal surface area of this project boring, predictable, and easy to reason about—so users and contributors can focus on the code and not on licensing edge cases. Clear rules up front mean fewer surprises later.

### Contributions

Contributions are very welcome.

By contributing to this project, you agree that your contributions are licensed under the Apache License, Version 2.0, consistent with the rest of the project. This ensures that all users of the project receive the same rights and protections.

No Contributor License Agreement (CLA) is required; the Apache-2.0 license itself provides the necessary clarity.

### Philosophy

Open source works best when everyone is on the same page. Think of a license as the copyright *interface* for a project. Apache-2.0 acts like a well-specified API: it defines clear preconditions, postconditions, and failure modes. That clarity is intentional and preferred over relying on subtle implications or informal norms. It's like preferring smart pointers and template parameters over raw void pointers in modern software design.

If you have questions about licensing or usage, feel free to open an issue for discussion.

### License FAQ

- **Q: Do I need to add a `NOTICE` file when using this library?**

  No. A `NOTICE` file is only required if a project *includes one*. This repository does not currently include a `NOTICE` file, so there is nothing for downstream users to preserve. Most projects do not need a `NOTICE` file unless they are redistributing Apache-licensed code that already has one or need to provide explicit attribution beyond the license text.

- **Q: I’m using this library as a dependency. Do I need to copy its license into my project?**

  If you are redistributing this library (for example, vendoring the source or shipping it as part of a binary distribution), you must include the Apache-2.0 license text. If you are simply depending on it (e.g. via a package manager or system install), no additional action is required.

- **Q: I modified a file. Do I need to update the `NOTICE` file or list myself somewhere?**

  No. Apache-2.0 does *not* use the `NOTICE` file to track modifications. If you redistribute a modified version of a file, the license requires only that the modified file carries a brief, prominent notice indicating that it was changed. There is no required format and no need to describe the change in detail. 

  Feel free to use the Doxygen `@author`/`@authors` tag to credit yourself if you like, but this is optional and purely informational.

  In practice, version control history usually provides sufficient context, and contributors to this repository are not expected to add manual modification notices.

- **Q: Do contributors need to sign a CLA?**

  No. Contributions are accepted under the Apache License, Version 2.0. The license already provides clear terms for copyright and patent rights, so no separate Contributor License Agreement is required.

- **Q: This project depends on other libraries. Do their licenses affect me?**

  Only if you redistribute those libraries yourself. Merely depending on or linking against third-party libraries does not transfer their license obligations to you. Each dependency’s license applies to copies of that dependency, not to code that merely uses it.

### How It’s Applied in This Repository
1. **`LICENSE` file**
   - See the `LICENSE` file at the repository root for the full text of the License.

2. **SPDX headers**  
   - Every source file starts with a machine-readable SPDX identifier:
     ```cpp
     // SPDX-License-Identifier: Apache-2.0
     ```

   - This allows automated tools to detect the license reliably.

3. **Per-file Doxygen license block**  
   - Each file header comment contains a Doxygen-style paragraph block with the full Apache-2.0 boilerplate notice.

   - Example:
     ```cpp
     /**
      * @par License: @parblock
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
      * limitations under the License. @endparblock
      */
     ```

   - This ensures the license is **readable in generated documentation** and present in each file for legal clarity.

---
